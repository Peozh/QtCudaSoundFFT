#ifndef SOUNDDEVICEMANAGER_HPP
#define SOUNDDEVICEMANAGER_HPP

#include "SoundDataBase.hpp"
#include "filterboxdatabase.h"
#include "AlcInputDevice.h"
#include "AlcOutputDevice.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <condition_variable>

#include <QObject>

namespace Sound {
    namespace Record {
        static std::mutex recordMtx; // isRecording, pInputDevice
    };
    namespace Play {
        static std::mutex playMtx; // idx, isPlaying, pOutputDevice
        static std::size_t willPlayIdx = 0; // already played stack count
        static std::size_t willPushIdx = 0; // next pushable stack index
	};
};

class SoundDeviceManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isRecording READ getIsRecording WRITE setIsRecording NOTIFY isRecordingChanged)
    Q_PROPERTY(bool isPlaying READ getIsPlaying WRITE setIsPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(size_t currentSliderIdx READ getCurrentSliderIdx WRITE setCurrentSliderIdx NOTIFY currentSliderIdxChanged)

public:
    std::mutex cvPlayMtx;
    std::condition_variable cvPlayThreadIdleWait;
    std::mutex cvRecordMtx;
    std::condition_variable cvRecordThreadIdleWait;
    bool isRecording = false;
    bool isPlayingFiltered = true;
    bool isPlaying = false;
    size_t currentSliderIdx = 0; // q_property copy version of Sound::Play::willPlayIdx

    bool getIsRecording() const { return this->isRecording; }
    void setIsRecording(const bool& isRecording)
    {
        if (this->isRecording == isRecording) return;
        {
            std::lock_guard<std::mutex> recordLockGuard(Sound::Record::recordMtx);
            this->isRecording = isRecording;
        }
        emit isRecordingChanged();
    }

    bool getIsPlaying() const { return this->isPlaying; }
    void setIsPlaying(const bool& isPlaying)
    {
        if (this->isPlaying == isPlaying) return;
        {
            std::lock_guard<std::mutex> playLockGuard(Sound::Play::playMtx);
            this->isPlaying = isPlaying;
        }
        emit isPlayingChanged();
    }

    size_t getCurrentSliderIdx()
    {
        return this->currentSliderIdx;
    }
    void setCurrentSliderIdx(const size_t& currentSliderIdx)
    {
        if (this->currentSliderIdx == currentSliderIdx) return;
        this->currentSliderIdx = currentSliderIdx;
        emit currentSliderIdxChanged();
    }

    SoundDataBase* pSoundDataBase = nullptr;
    FilterBoxDataBase* pFilterBoxDataBase = nullptr;
    AlcOutputDevice* pOutputDevice = nullptr;
    AlcInputDevice* pInputDevice = nullptr;
    size_t frequency = 44100;

    explicit SoundDeviceManager(QObject *parent = nullptr)
        : QObject{parent}
    {
    }
    ~SoundDeviceManager() = default;

    void connectSoundDataBase(SoundDataBase* pSoundDataBase)
    {
        std::lock_guard<std::mutex> playLockGuard(Sound::Play::playMtx);
        std::lock_guard<std::mutex> recordLockGuard(Sound::Record::recordMtx);
        this->pSoundDataBase = pSoundDataBase;
    }
    void connectFilterBoxDataBase(FilterBoxDataBase* pFilterBoxDataBase)
    {
        std::lock_guard<std::mutex> playLockGuard(Sound::Play::playMtx);
        this->pFilterBoxDataBase = pFilterBoxDataBase;
    }
    void connectOutputDevice(AlcOutputDevice* pOutputDevice)
    {
        std::lock_guard<std::mutex> playLockGuard(Sound::Play::playMtx);
        this->pOutputDevice = pOutputDevice;
    }
    void connectInputDevice(AlcInputDevice* pInputDevice)
    {
        std::lock_guard<std::mutex> recordLockGuard(Sound::Record::recordMtx);
        this->pInputDevice = pInputDevice;
    }

    void startRecord()
    {
        if (this->pInputDevice == nullptr) return;
        std::cout << "[Start Record]" << std::endl;
        // Start capture inputDevice
        pInputDevice->startCapture();
//        pSoundDataBase->resetData(); // todo : mute this line if possible

        // Capture into baseHolder
        while (pInputDevice->isCapturing()) {
            std::lock_guard<std::mutex> recordLockGuard(Sound::Record::recordMtx);
            // Poll for captured audio
            if (pInputDevice->getCapturedSampleCount() > pSoundDataBase->batchSampleCount_singleChannel) {
                // Grab the sound
                pInputDevice->attachCapturedSamplesToData(pSoundDataBase->batchSampleCount_singleChannel);
                pSoundDataBase->pushPCM(pInputDevice->data.data());
            }
        }
        pSoundDataBase->endPushPCM();
    }
    void stopRecord()
    {
        if (this->pInputDevice == nullptr) return;
        this->setIsRecording(false);
        pInputDevice->stopCapture();
    }
    void startPlay()
    {
        if (this->pOutputDevice == nullptr) return;
        std::cout << "[Start Play]" << std::endl;
        // clear attached buffer
        pOutputDevice->detachBufferFromSourceQueue(pOutputDevice->getPlayedBufferCount());

        Sound::Play::willPushIdx = Sound::Play::willPlayIdx; // todo : apply mutex?
        while (Sound::Play::willPlayIdx < pSoundDataBase->outputPCMBatches.size()-1) // 재생 가능한 데이터의 끝에 도달하지 않은 경우 지속
        {
            std::lock_guard<std::mutex> playLockGuard(Sound::Play::playMtx); // use mutex for { pDataBases, pDevices, Idx, isPlaying }
            if (this->pOutputDevice == nullptr) return;

            // detach buffers from source, put it back into buffer pool
            ALint playedBufferCount = pOutputDevice->getPlayedBufferCount();
            if (this->isPlaying) { Sound::Play::willPlayIdx += playedBufferCount; }
            else { Sound::Play::willPushIdx = Sound::Play::willPlayIdx; }
            this->setCurrentSliderIdx(Sound::Play::willPlayIdx);
            if (playedBufferCount > 0) {
                pOutputDevice->detachBufferFromSourceQueue(playedBufferCount);
                std::cout << "willPushIdx = " << Sound::Play::willPushIdx << ", willPlayIdx = " << Sound::Play::willPlayIdx << ", pSoundDataBase->outputPCMBatches.size() = " << pSoundDataBase->outputPCMBatches.size() << std::endl;
            }
            if ((this->isPlaying == false) && (Sound::Play::willPushIdx == Sound::Play::willPlayIdx)) break; // 정지되어야 하는 상황에서 일단 큐드된 버퍼를 모두 재생한 경우 종료조건

            // push buffer to Source
            if (!pOutputDevice->queueableBufferIds.empty() && (this->isPlaying == true) && Sound::Play::willPushIdx < pSoundDataBase->outputPCMBatches.size()-1) // buffer pool 에 가용 버퍼가 있는 경우
            {
                ALuint freshBufferId = pOutputDevice->queueableBufferIds.front();
                pOutputDevice->queueableBufferIds.pop();
                {
                    const void* pData;
                    if (this->isPlayingFiltered == true)
                    {
                        this->pFilterBoxDataBase->setFilterData(Sound::Play::willPushIdx);
                        pData = pSoundDataBase->getOutputPCM(Sound::Play::willPushIdx);
                    }
                    else if (this->isPlayingFiltered == false) pData = pSoundDataBase->inputPCMBatches[Sound::Play::willPushIdx];
                    // Copy data to Buffer
                    pOutputDevice->setBufferData(freshBufferId,
                        pData,
                        pSoundDataBase->batchSampleCount_singleChannel,
                        this->frequency);
                    ++Sound::Play::willPushIdx;
                }

                // Attach Buffer to Source (Queued way)
                pOutputDevice->attachBufferToSourceQueue(&freshBufferId);

                // Restart the source if needed
                // (if we take too long and the queue dries up,
                //  the source stops playing).
                ALint sourceState = pOutputDevice->getSourceState();
                {
                    if (this->isPlaying == true && (sourceState != AL_PLAYING) && (pOutputDevice->queueableBufferIds.size() <= pOutputDevice->bufferCount - 1)) { // 재생해야 하는데 멈춘상태고, 1개 이상이 큐드된 상태면 재생
                        pOutputDevice->playSource();
                    }
                }
            }
        }
        this->setIsPlaying(false);
        if (Sound::Play::willPushIdx == pSoundDataBase->outputPCMBatches.size()-1)
        {
            std::cout << "willPushIdx = " << Sound::Play::willPushIdx << ", willPlayIdx = " << Sound::Play::willPlayIdx << ", pSoundDataBase->outputPCMBatches.size() = " << pSoundDataBase->outputPCMBatches.size() << std::endl;
            this->setPlayIndex(0);
        }
    }
    void stopPlay()
    {
        if (this->pOutputDevice == nullptr) return;
        std::cout << "[Stop Play]" << std::endl;
        this->setIsPlaying(false);
        pOutputDevice->stopSource();
//        std::cout << "willPushIdx = " << Sound::Play::willPushIdx << ", willPlayIdx = " << Sound::Play::willPlayIdx << std::endl;
//        this->setPlayIndex(Sound::Play::willPlayIdx);
    }
    Q_INVOKABLE void setPlayIndex(size_t idx)
    {
        std::lock_guard<std::mutex> playLockGuard(Sound::Play::playMtx);
        if (this->pSoundDataBase == nullptr) return;
        if (idx >= this->pSoundDataBase->outputPCMBatches.size()-1) return;
//        if (this->pOutputDevice != nullptr) {
//            this->pOutputDevice->stopSource();
////            this->stopPlay();
//        }
        std::cout << "Set Play Index : " << idx << std::endl;
        if (Sound::Play::willPlayIdx != idx)
        {
            Sound::Play::willPlayIdx = idx;
            Sound::Play::willPushIdx = idx;
            this->setCurrentSliderIdx(Sound::Play::willPlayIdx);

            if (this->pOutputDevice == nullptr) return;
            this->pOutputDevice->stopSource();
            ALint playedBufferCount = pOutputDevice->getPlayedBufferCount();
            pOutputDevice->detachBufferFromSourceQueue(playedBufferCount);
            if (this->isPlaying) this->pOutputDevice->playSource();
        }
        // condition variable 사용 필요할듯. wait, notify_one 으로 동일 쓰레드에서 정지되었다가 this->startPlay() 재시작이 보장되어야 함
//        if (this->pOutputDevice != nullptr) {
//            this->pOutputDevice->stopSource();
////            this->pOutputDevice->playSource();
////            this->stopPlay();
//        }
//        if (this->pOutputDevice != nullptr) {
//            if (this->isPlaying == true) this->startPlay();
//        }
    }
    Q_INVOKABLE void resetPlayer()
    {
        this->stopPlay();
        this->stopRecord();
        setPlayIndex(0);
//        pSoundDataBase->resetData();

    }
    Q_INVOKABLE void recordStartStop()
    {
        if (this->isRecording) { // stop
            if (this->pInputDevice == nullptr) return;
            stopRecord();
        }
        else { // record
            if (this->pInputDevice == nullptr) return;
            this->setIsRecording(true);
            this->cvRecordThreadIdleWait.notify_one();
            std::cout << "notify one - record" << std::endl;
        }
    }
    Q_INVOKABLE void playStartStop()
    {
        if (this->isPlaying) { // stop
            if (this->pOutputDevice == nullptr) return;
            this->stopPlay();
        }
        else { // play
            if (this->pOutputDevice == nullptr) return;
            this->setIsPlaying(true);
            this->cvPlayThreadIdleWait.notify_one();
            std::cout << "notify one - play" << std::endl;
        }
    }

    void makePlayThread()
    {
        if (this->pOutputDevice == nullptr) return;
        std::cout << "child play thread making..." << std::endl;

        std::thread threadPlaying([this]() { // make this thread when load devices
            std::cout << "play thread (" << std::this_thread::get_id() << ") generated" << std::endl;
            while(this->pOutputDevice != nullptr && this->pSoundDataBase != nullptr)
            {
                std::unique_lock<std::mutex> cvUniqueLock(this->cvPlayMtx);
                std::cout << "play thread waiting..." << std::endl;
                this->cvPlayThreadIdleWait.wait(cvUniqueLock , [this]{ return this->isPlaying || this->pOutputDevice == nullptr; }); // 재생해야 하거나, 스레드 종료해야 하는 경우, 깨어남을 허용

                std::cout << "play thread wake up" << std::endl;
                this->startPlay(); // device unload 방지, unload 시 stopdevice 먼저하고 락걸고 디바이스 nullptr 화 하기

                std::cout << "willPushIdx = " << Sound::Play::willPushIdx << ", willPlayIdx = " << Sound::Play::willPlayIdx << std::endl;
                this->setPlayIndex(Sound::Play::willPlayIdx);
            }
            std::cout << "closed play thread (" << std::this_thread::get_id() << ")" << std::endl;
        });
        threadPlaying.detach();
        std::cout << "child play thread detached" << std::endl;
    }
    void makeRecordThread()
    {
        if (this->pInputDevice == nullptr) return;
        std::cout << "child record thread making..." << std::endl;

        std::thread threadRecording([this]() { // make this thread when load devices
            std::cout << "record thread (" << std::this_thread::get_id() << ") generated" << std::endl;
            while(this->pInputDevice != nullptr && this->pSoundDataBase != nullptr)
            {
                std::unique_lock<std::mutex> cvUniqueLock(this->cvRecordMtx);
                std::cout << "record thread waiting..." << std::endl;
                this->cvRecordThreadIdleWait.wait(cvUniqueLock , [this]{ return this->isRecording || this->pInputDevice == nullptr; }); // 재생해야 하거나, 스레드 종료해야 하는 경우, 깨어남을 허용
                std::cout << "record thread wake up" << std::endl;
                this->startRecord(); // device unload 방지, unload 시 stopdevice 먼저하고 락걸고 디바이스 nullptr 화 하기
            }
            std::cout << "closed record thread (" << std::this_thread::get_id() << ")" << std::endl;
        });
        threadRecording.detach();
        std::cout << "child play thread detached" << std::endl;
    }

    Q_INVOKABLE QString getTimeRepresentation(uint64_t timeIdx)
    {
        if (this->pSoundDataBase == nullptr) return "--:--:---";

        float batchSize = this->pSoundDataBase->batchSampleCount_singleChannel;
        if (batchSize == 0) return "--:--:---";
        float samplerate = this->frequency;
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

signals:
    void isRecordingChanged();
    void isPlayingChanged();
    void currentSliderIdxChanged();
};

/*
목표 : 
 - 녹음되면 매 batch 크기 읽어서 자동으로 soundDataBase 에 저장되기. 1차 초벌 fft 계산도 자동으로 진행됨
 - 재생은 녹음과 세트가 되어 바로 재생해주거나, 부하분산 목적이면 녹음과 별도로 재생하도록 할 수 있음
 - 재생 시에 2차 fft 계산을 진행하여 재생 device 에 자동으로 전달하기.

*/

#endif // SOUNDDEVICEMANAGER_HPP
