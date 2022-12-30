#ifndef SOUNDDATABASE_HPP
#define SOUNDDATABASE_HPP

#define _USE_MATH_DEFINES // for M_PI in <cmath>
#include <cmath>
#include <QObject>

#include "myCudaForQt.h"

class SoundDataBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(size_t maxSliderIdx READ getMaxSliderIdx WRITE setMaxSliderIdx NOTIFY maxSliderIdxChanged);
    Q_PROPERTY(size_t batchReadCurrentCount READ getBatchReadCurrentCount WRITE setBatchReadCurrentCount NOTIFY batchReadCurrentCountChanged);

public:
    enum SoundFormat : int { NONE = 0, FORMAT_MONO8 = 0x1100, FORMAT_MONO16 = 0x1101, FORMAT_STEREO8 = 0x1102, FORMAT_STEREO16 = 0x1103 };

public:
    int format;
    size_t sampleRate;
    size_t channelCount;
    size_t sampleByteSize_singleChannel;
    size_t batchSampleCount_singleChannel;
    size_t batchSampleCount_total;
    size_t batchByteSize_total;
    size_t maxSliderIdx = 0; // q_property copy version of Sound::Record::RecordedBatchCount
    int batchReadTotalCount = 0;
    int batchReadCurrentCount = 0;

    std::vector<float> hannFilter;
    std::vector<float> freqFilter;

    std::vector<void*> inputPCMBatches;
    std::vector<void*> inputFreqBatches;
    std::vector<void*> outputFreqBatches;
    std::vector<void*> outputPCMBatches;

    explicit SoundDataBase(QObject *parent = nullptr)
        : QObject{parent}, format(NONE), sampleRate(0) //, recordedBatchCount(0)
    {
    }
    ~SoundDataBase()
    {
        resetFormat();
        resetData();
    }

    size_t getMaxSliderIdx()
    {
        return this->maxSliderIdx;
    }
    void setMaxSliderIdx(const size_t& maxSliderIdx)
    {
        if (this->maxSliderIdx == maxSliderIdx) return;
        this->maxSliderIdx = maxSliderIdx;
        emit maxSliderIdxChanged();
    }
    int getBatchReadCurrentCount()
    {
        return this->batchReadCurrentCount;
    }
    void setBatchReadCurrentCount(const int& newBatchReadCurrentCount)
    {
        if (this->batchReadCurrentCount == newBatchReadCurrentCount) return;
        this->batchReadCurrentCount = newBatchReadCurrentCount;
        emit batchReadCurrentCountChanged();
    }

    void resetFormat()
    {
        this->format = NONE;
        this->sampleRate = 0;
        this->channelCount = 0;
        this->sampleByteSize_singleChannel = 0;
        this->batchSampleCount_singleChannel = 0;
        this->batchSampleCount_total = 0;
        this->batchByteSize_total = 0;

        hannFilter = std::vector<float> {};
        freqFilter = std::vector<float> {};
    }
    void resetData()
    {
        for (void*& pBatch : inputPCMBatches) if (pBatch != nullptr) { free(pBatch); pBatch = nullptr; }
        for (void*& pBatch : inputFreqBatches) if (pBatch != nullptr) { myCUDA::freePtr(pBatch); pBatch = nullptr; }
        for (void*& pBatch : outputFreqBatches) if (pBatch != nullptr) { myCUDA::freePtr(pBatch); pBatch = nullptr; }
        for (void*& pBatch : outputPCMBatches) if (pBatch != nullptr) { free(pBatch); pBatch = nullptr; }
        inputPCMBatches = std::vector<void*> {};
        inputFreqBatches = std::vector<void*> {};
        outputFreqBatches = std::vector<void*> {};
        outputPCMBatches = std::vector<void*> {};
        inputPCMBatches.reserve(100);
        inputFreqBatches.reserve(100);
        outputFreqBatches.reserve(100);
        outputPCMBatches.reserve(100);

        this->setMaxSliderIdx(0);

        emit afterResetOccured();
    }
    void setFormat(int format, size_t batchSampleCount_singleChannel)
    {
        resetFormat();
        resetData();
        this->format = format;
        switch(format)
        {
            case FORMAT_MONO16:
                channelCount = 1;
                sampleByteSize_singleChannel = 2;
                break;
            case FORMAT_STEREO8:
                channelCount = 2;
                sampleByteSize_singleChannel = 1;
                break;
            case FORMAT_STEREO16:
                channelCount = 2;
                sampleByteSize_singleChannel = 2;
                break;
            case FORMAT_MONO8:
            default:
                channelCount = 1;
                sampleByteSize_singleChannel = 1;
                break;
        }
        this->batchSampleCount_singleChannel = batchSampleCount_singleChannel;
        this->batchSampleCount_total = channelCount * batchSampleCount_singleChannel;
        this->batchByteSize_total = batchSampleCount_total * sampleByteSize_singleChannel;
        
        this->hannFilter.resize(batchSampleCount_singleChannel*2);
        double w = M_PI/batchSampleCount_singleChannel/2;
        for (size_t idx = 0; idx < hannFilter.size(); ++idx) hannFilter[idx] = (float)(sin(w*idx)*sin(w*idx));

        this->freqFilter = std::vector<float>(batchSampleCount_singleChannel+1, 1.0f);

        pushZeroPCM();
    }

    void pushPCM(void* src)
    {
        void* pBatch = malloc(this->batchByteSize_total * sizeof(uint8_t));
        std::memcpy(pBatch, src, this->batchByteSize_total);
        this->inputPCMBatches.push_back(pBatch);
        afterPushPCM();
    }
    void pushPCM(void* src, size_t batchByteSize_partial)
    {
        void* pBatch = calloc(this->batchByteSize_total, sizeof(uint8_t));
        std::memcpy(pBatch, src, batchByteSize_partial);
        this->inputPCMBatches.push_back(pBatch);
        afterPushPCM();
    }
    void pushPCMs(void* srcs, size_t byteSize_total)
    {
        resetData();
        pushZeroPCM();
        size_t batchCount = byteSize_total / this->batchByteSize_total + 3;
        inputPCMBatches.reserve(batchCount);
        inputFreqBatches.reserve(batchCount);
        outputFreqBatches.reserve(batchCount);
        outputPCMBatches.reserve(batchCount);

        uint8_t* src_offset = (uint8_t*)srcs;
        size_t byteSize_remain = byteSize_total;
        this->batchReadTotalCount = (byteSize_remain+this->batchByteSize_total-1)/this->batchByteSize_total;
        this->setBatchReadCurrentCount(0);
        while (byteSize_remain >= this->batchByteSize_total)
        {
            pushPCM(src_offset);

            src_offset += this->batchByteSize_total;
            byteSize_remain -= this->batchByteSize_total;
            this->setBatchReadCurrentCount(batchReadCurrentCount + 1);
        }
        if (byteSize_remain > 0) 
        {
            pushPCM(src_offset, byteSize_remain);
            this->setBatchReadCurrentCount(batchReadCurrentCount + 1);
        }
        endPushPCM();
    }
    void endPushPCM()
    {
        pushZeroPCMi();
        afterPushPCM();
    }

    void* getOutputPCM(size_t idx) // if you pushed N batches : [0, N]
    {
        if (idx >= outputFreqBatches.size()) return nullptr;
        processFreqBatch(idx);
        return outputPCMBatches[idx];
    }

    Q_INVOKABLE QString getTimeRepresentation(uint64_t timeIdx)
    {
        float batchSize = this->batchSampleCount_singleChannel;
        if (batchSize == 0) return "--:--:---";
        float samplerate = this->sampleRate;
        if (samplerate == 0) return "--:--:---";

        double timeSecondsDouble = (double)timeIdx*batchSize/samplerate;
        int64_t timeSecondsTotal = (int64_t)timeSecondsDouble;
        int64_t timeMiliSeconds = (int64_t)(timeSecondsDouble*1000) - timeSecondsTotal*1000;
        int64_t timeSeconds = timeSecondsTotal%60;
        int64_t timeMinuteTotal = timeSecondsTotal/60;

        QString miliseconds = QString("%1").arg(timeMiliSeconds, 3, 10, QChar('0'));
        QString seconds = QString("%1").arg(timeSeconds, 2, 10, QChar('0'));
        QString minutes = QString("%1").arg(timeMinuteTotal, 2, 10, QChar('0'));

        return minutes + ':' + seconds + ':' + miliseconds;
    }

private:
    void pushZeroPCMi()
    {
        void* pBatch_i = calloc(this->batchByteSize_total, sizeof(uint8_t));
        this->inputPCMBatches.push_back(pBatch_i);
    }
    void pushZeroPCMo()
    {
        void* pBatch_o = calloc(this->batchByteSize_total, sizeof(uint8_t));
        this->outputPCMBatches.push_back(pBatch_o);
    }
    void pushZeroPCM()
    {
        pushZeroPCMi();
        pushZeroPCMo();
    }

    void afterPushPCM()
    {
        size_t curIdx = inputPCMBatches.size()-1; //  1 ~ N+1
        if (sampleByteSize_singleChannel == 1) 
        {
            void* pInputFreqBatch = nullptr;
            myCUDA::afterRecord_8(
                (int8_t*)(this->inputPCMBatches[curIdx-1]), 
                (int8_t*)(this->inputPCMBatches[curIdx]),
                hannFilter.data(),
                pInputFreqBatch,
                batchSampleCount_singleChannel,
                channelCount);
            this->inputFreqBatches.push_back(pInputFreqBatch);
        } 
        else if (sampleByteSize_singleChannel == 2) 
        {
            void* pInputFreqBatch = nullptr;
            myCUDA::afterRecord_16(
                (int16_t*)(this->inputPCMBatches[curIdx-1]), 
                (int16_t*)(this->inputPCMBatches[curIdx]),
                hannFilter.data(),
                pInputFreqBatch,
                batchSampleCount_singleChannel,
                channelCount);
            this->inputFreqBatches.push_back(pInputFreqBatch);
        }
        outputFreqBatches.push_back(nullptr);
        outputPCMBatches.push_back(nullptr);
        this->setMaxSliderIdx(inputFreqBatches.size());

        emit afterPCMPushed();
    }

    void processFreqBatch(size_t idx)
    {
        if (idx >= this->outputFreqBatches.size()) return;
        if (sampleByteSize_singleChannel == 1)
        {
            void* pOutputPCMBatch1 = this->outputPCMBatches[idx];
            void* pOutputPCMBatch2 = this->outputPCMBatches[idx+1];
            if (pOutputPCMBatch1 == nullptr) pOutputPCMBatch1 = calloc(this->batchSampleCount_singleChannel*channelCount, sizeof(uint8_t));
            if (pOutputPCMBatch2 == nullptr) pOutputPCMBatch2 = malloc(sizeof(uint8_t)*this->batchSampleCount_singleChannel*channelCount);
            myCUDA::beforePlay_8(
                (cufftComplex *)this->inputFreqBatches[idx],
                this->outputFreqBatches[idx],
                pOutputPCMBatch1,
                pOutputPCMBatch2,
                this->freqFilter.data(),
                this->batchSampleCount_singleChannel,
                this->channelCount);
            this->outputPCMBatches[idx] = pOutputPCMBatch1;
            this->outputPCMBatches[idx+1] = pOutputPCMBatch2;
        }
        else if (sampleByteSize_singleChannel == 2)
        {
            void* pOutputPCMBatch1 = this->outputPCMBatches[idx];
            void* pOutputPCMBatch2 = this->outputPCMBatches[idx+1];
            if (pOutputPCMBatch1 == nullptr) pOutputPCMBatch1 = calloc(this->batchSampleCount_singleChannel*channelCount, sizeof(int16_t));
            if (pOutputPCMBatch2 == nullptr) pOutputPCMBatch2 = malloc(sizeof(int16_t)*this->batchSampleCount_singleChannel*channelCount);
            myCUDA::beforePlay_16(
                (cufftComplex *)this->inputFreqBatches[idx],
                this->outputFreqBatches[idx],
                pOutputPCMBatch1,
                pOutputPCMBatch2,
                this->freqFilter.data(),
                this->batchSampleCount_singleChannel,
                this->channelCount);
            this->outputPCMBatches[idx] = pOutputPCMBatch1;
            this->outputPCMBatches[idx+1] = pOutputPCMBatch2;
        }
    }

signals:
    void afterResetOccured();
    void afterPCMPushed();
    void maxSliderIdxChanged();
    void batchReadCurrentCountChanged();

/*
case : batch stack when 1 batch pushed and endPushPCM() is called
    input pcm   1 1 1
    input freq   1 1
    output freq  1 1
    output pcm  1 1 1
*/
};


#endif // SOUNDDATABASE_HPP
