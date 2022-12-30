#include "filterboxdatabase.h"

FilterBoxDataBase::FilterBoxDataBase(QObject *parent)
    : QObject{parent}
{}

void FilterBoxDataBase::connectSoundDataBase(SoundDataBase *pSoundDataBase)
{
    this->pSoundDataBase = pSoundDataBase;
}

int64_t FilterBoxDataBase::getActiveFilterIdx() const
{
    return this->activeFilterIdx;
}
void FilterBoxDataBase::setActiveFilterIdx(const uint64_t &newActiveFilterIdx)
{
//    if (this->activeFilterIdx == newActiveFilterIdx) return;
    this->activeFilterIdx = newActiveFilterIdx;
    emit activeFilterIdxChanged();
}

QList<QString> FilterBoxDataBase::getFilterNamesModel()
{
    return this->filterNamesModel;
}
void FilterBoxDataBase::setFilterNamesModel(const QList<QString> &newFilterNamesModel)
{
//    if (this->filterNamesModel == newFilterNamesModel) return;
    this->filterNamesModel = newFilterNamesModel;
    emit filterNamesModelChanged();
}

void FilterBoxDataBase::setFilterData(size_t timeIdx)
{
    if (this->pSoundDataBase == nullptr) return;
    float* pFreqFilter = this->pSoundDataBase->freqFilter.data();
    myCUDA::initFreqFilter(pFreqFilter, 1.0f, this->pSoundDataBase->freqFilter.size());
    for (const auto& filter : this->filters)
    {
        if (timeIdx < filter.timeIdx[0] || filter.timeIdx[1] <= timeIdx) continue;
        if (filter.timeIdx[0] >= filter.timeIdx[1]) continue;

        if (filter.freqIdx[0] <= this->pSoundDataBase->batchSampleCount_singleChannel && 0 <= filter.freqIdx[1]) {
            uint64_t freqIdxSt = filter.freqIdx[0]+1;
            uint64_t freqIdxEn = filter.freqIdx[1];
            if (filter.freqIdx[1] > this->pSoundDataBase->batchSampleCount_singleChannel) freqIdxEn = this->pSoundDataBase->batchSampleCount_singleChannel;
            myCUDA::setFreqFilter(pFreqFilter, filter.filterWeight, freqIdxSt, freqIdxEn);
        }
    }
}

QString FilterBoxDataBase::getFilterName(uint64_t filterIdx)
{
    if (filterIdx >= this->filters.size()) return "-";
    return this->filterNamesModel[filterIdx];
}
float FilterBoxDataBase::getFilterWeight(uint64_t filterIdx, uint64_t dummyIdx)
{
    if (filterIdx >= this->filters.size()) return 0.0f;
    return this->filters[filterIdx].filterWeight;
}
uint64_t FilterBoxDataBase::getFilterTimeIdxSt(uint64_t filterIdx)
{
    if (filterIdx >= this->filters.size()) return 0;
    return this->filters[filterIdx].timeIdx[0];
}
uint64_t FilterBoxDataBase::getFilterTimeIdxEn(uint64_t filterIdx)
{
    if (filterIdx >= this->filters.size()) return 0;
    return this->filters[filterIdx].timeIdx[1];
}
uint64_t FilterBoxDataBase::getFilterFreqIdxSt(uint64_t filterIdx)
{
    if (filterIdx >= this->filters.size()) return 0;
    return this->filters[filterIdx].freqIdx[0];
}
uint64_t FilterBoxDataBase::getFilterFreqIdxEn(uint64_t filterIdx)
{
    if (filterIdx >= this->filters.size()) return 0;
    return this->filters[filterIdx].freqIdx[1];
}

void FilterBoxDataBase::addFilter(QString filterName, float filterWeight, uint64_t timeIdxSt, uint64_t timeIdxEn, uint64_t freqIdxSt, uint64_t freqIdxEn)
{
    this->filters.push_back(Filter{ filterName, filterWeight, timeIdxSt, timeIdxEn, freqIdxSt, freqIdxEn });
    auto newFilterNamesModel = this->filterNamesModel;
    newFilterNamesModel.push_back(filterName);
    this->setFilterNamesModel(newFilterNamesModel);
    size_t idx = this->filters.size()-1;
    this->calcFilterVertices(idx);
    this->setActiveFilterIdx(idx);
}
void FilterBoxDataBase::applyChange(uint64_t filterIdx, QString filterName, QString filterWeightStr, QString timeIdxStStr, QString timeIdxEnStr, QString freqIdxStStr, QString freqIdxEnStr)
{
    if (filterIdx >= this->filters.size()) return;
    float filterWeight = filterWeightStr.toFloat();
    uint64_t timeIdxSt = timeIdxStStr.toULongLong();
    uint64_t timeIdxEn = timeIdxEnStr.toULongLong();
    uint64_t freqIdxSt = freqIdxStStr.toULongLong();
    uint64_t freqIdxEn = freqIdxEnStr.toULongLong();
    if (filterIdx >= this->filters.size()) return;
    Filter& filter = this->filters[filterIdx];
    filter.filterName = filterName;
    filter.filterWeight = filterWeight;
    filter.timeIdx[0] = timeIdxSt;
    filter.timeIdx[1] = timeIdxEn;
    filter.freqIdx[0] = freqIdxSt;
    filter.freqIdx[1] = freqIdxEn;
    this->calcFilterVertices(filterIdx);
    auto newFilterNamesModel = this->filterNamesModel;
    newFilterNamesModel[filterIdx] = filterName;
    this->setFilterNamesModel(newFilterNamesModel);
    this->setActiveFilterIdx(this->activeFilterIdx);
}
void FilterBoxDataBase::deleteFilter(uint64_t filterIdx)
{
    if (filterIdx >= this->filters.size()) return;
    this->filters.erase(this->filters.begin() + filterIdx, this->filters.begin() + filterIdx + 1);
//    auto newFilterNamesModel = this->filterNamesModel;
//    newFilterNamesModel.erase(newFilterNamesModel.cbegin() + filterIdx);
//    this->setFilterNamesModel(newFilterNamesModel);
    this->filterNamesModel.erase(this->filterNamesModel.cbegin() + filterIdx);
    emit this->filterNamesModelChanged();
    this->setActiveFilterIdx(filterIdx-1);
}
void FilterBoxDataBase::resetFilter()
{
    this->filters = std::vector<Filter>{};
    auto newFilterNamesModel = QList<QString>{};
    this->setFilterNamesModel(newFilterNamesModel);
}

QString FilterBoxDataBase::getTimeRepresentation(QString timeIdxStr)
{
    if (this->pSoundDataBase == nullptr) return "--:--:---";
//    if (this->pDeviceControllers == nullptr) return "--:--:---";

    int64_t timeIdx = timeIdxStr.toLongLong();

    float batchSize = this->pSoundDataBase->batchSampleCount_singleChannel;
    if (batchSize == 0) return "--:--:---";
    float samplerate = this->pSoundDataBase->sampleRate;
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
QString FilterBoxDataBase::getFreqRepresentation(QString freqIdxStr, bool isSt)
{
    if (this->pSoundDataBase == nullptr) return "-";
//    if (this->pDeviceControllers == nullptr) return "-";

    int64_t freqIdx = freqIdxStr.toLongLong();

    float to_h = this->pSoundDataBase->batchSampleCount_singleChannel;
    if (to_h == 0) return "-";
    float samplerate = this->pSoundDataBase->sampleRate;
    if (samplerate == 0) return "-";

    double freq;
    if (isSt) { freq = (double)samplerate/2/to_h*(freqIdx+1); }
    else { freq = (double)samplerate/2/to_h*(freqIdx); }

    return QString::number(freq, 'g', 6);
}

void FilterBoxDataBase::calcFilterVerticesAll()
{
    for (size_t idx = 0; idx < this->filters.size(); ++idx) this->calcFilterVertices(idx);
}
void FilterBoxDataBase::calcFilterVertices(uint64_t filterIdx)
{
    if (filterIdx >= this->filters.size()) return;
    if (this->pSoundDataBase == nullptr) return;
    Filter& filter = this->filters[filterIdx];
    float to_h = this->pSoundDataBase->batchSampleCount_singleChannel;
    if (to_h == 0) to_h = 1024;
    float to_v = this->pSoundDataBase->inputFreqBatches.size();
    if (to_v == 0) to_v = 16;
    float up = 1 - (float)filter.timeIdx[0]/to_v*2;
    float down = 1 - (float)filter.timeIdx[1]/to_v*2;
    float left = (float)filter.freqIdx[0]/to_h*2 - 1;
    float right = (float)filter.freqIdx[1]/to_h*2 - 1;
    filter.filterVertices = {
        { left, down, 0 },
        { right, down, 0 },
        { right, up, 0 },
        { left, up, 0 }
    };
//    std::cout << "left : " << left << ", right : " << right << ", up : " << up << ", down : " << down << std::endl;

    emit filterVerticesChanged();
}
