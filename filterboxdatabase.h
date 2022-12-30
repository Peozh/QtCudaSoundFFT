#ifndef FILTERBOXDATABASE_H
#define FILTERBOXDATABASE_H

#include "SoundDataBase.hpp"

#include <QObject>

struct FilterVertices
{
    float LD[3] = {0, 0, 0};
    float RD[3] = {0, 0, 0};
    float RU[3] = {0, 0, 0};
    float LU[3] = {0, 0, 0};
};

struct Filter
{
    Filter(QString filterName = "new filter", float filterWeight = 1.0f, uint64_t timeIdxSt = 0, uint64_t timeIdxEn = 0, uint64_t freqIdxSt = 0, uint64_t freqIdxEn = 0)
        : filterName{filterName}, filterWeight{filterWeight}, timeIdx{timeIdxSt, timeIdxEn}, freqIdx{freqIdxSt, freqIdxEn}
    {
    }

    QString filterName;
    float filterWeight;
    uint64_t timeIdx[2] = {0, 0};
    uint64_t freqIdx[2] = {0, 0};
    FilterVertices filterVertices;
};

class FilterBoxDataBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint64_t activeFilterIdx READ getActiveFilterIdx WRITE setActiveFilterIdx NOTIFY activeFilterIdxChanged)
    Q_PROPERTY(QList<QString> filterNamesModel READ getFilterNamesModel WRITE setFilterNamesModel NOTIFY filterNamesModelChanged)

public:
    explicit FilterBoxDataBase(QObject *parent = nullptr);

    std::vector<Filter> filters;
    uint64_t activeFilterIdx = -1;
    QList<QString> filterNamesModel;

    void connectSoundDataBase(SoundDataBase* pSoundDataBase);
//    void connectDeviceControllers(DeviceControllers* pDeviceControllers);

    int64_t getActiveFilterIdx() const;
    void setActiveFilterIdx(const uint64_t& newActiveFilterIdx);

    QList<QString> getFilterNamesModel();
    void setFilterNamesModel(const QList<QString>& newFilterNamesModel);

    void setFilterData(size_t timeIdx);

    Q_INVOKABLE QString getFilterName(uint64_t filterIdx);
    Q_INVOKABLE float getFilterWeight(uint64_t filterIdx, uint64_t dummyIdx = 0);
    Q_INVOKABLE uint64_t getFilterTimeIdxSt(uint64_t filterIdx);
    Q_INVOKABLE uint64_t getFilterTimeIdxEn(uint64_t filterIdx);
    Q_INVOKABLE uint64_t getFilterFreqIdxSt(uint64_t filterIdx);
    Q_INVOKABLE uint64_t getFilterFreqIdxEn(uint64_t filterIdx);
    Q_INVOKABLE void addFilter(QString filterName, float filterWeight = 1.0, uint64_t timeIdxSt = 0, uint64_t timeIdxEn = 0, uint64_t freqIdxSt = 0, uint64_t freqIdxEn = 0);
    Q_INVOKABLE void applyChange(uint64_t filterIdx, QString filterName, QString filterWeight, QString timeIdxSt, QString timeIdxEn, QString freqIdxSt, QString freqIdxEn);
    Q_INVOKABLE void deleteFilter(uint64_t filterIdx);
    Q_INVOKABLE void resetFilter();

    Q_INVOKABLE QString getTimeRepresentation(QString timeIdxStr);
    Q_INVOKABLE QString getFreqRepresentation(QString freqIdxStr, bool isSt);

    Q_INVOKABLE void calcFilterVerticesAll();

signals:
    void activeFilterIdxChanged();
    void filterNamesModelChanged();
    void filterVerticesChanged();

private:
    SoundDataBase* pSoundDataBase = nullptr;
//    DeviceControllers* pDeviceControllers = nullptr;

    void calcFilterVertices(uint64_t filterIdx);

};

#endif // FILTERBOXDATABASE_H


// q invokable
// addFilter
// applyChange
// deleteFilter
// resetFilter



// q property
// ListModel
// active filter index
