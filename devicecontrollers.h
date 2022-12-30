#ifndef DEVICECONTROLLERS_H
#define DEVICECONTROLLERS_H

#include "AlcInputDevice.h"
#include "AlcOutputDevice.h"
#include "WaveHeader.hpp"
#include "SoundDataBase.hpp"
#include "SoundDeviceManager.hpp"
#include "loadingdialog.h"

#include <QObject>
#include <QWidget>
#include <QFileDialog>
#include <thread>

enum class OutState { Playing, Stopped };
enum class InState { Recording, Stopped };
enum class InputType { None, File, Device };
enum class OutputType { None, Device };

class DeviceControllers : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QList<QString> inputDeviceListModel READ getInputDeviceListModel WRITE setInputDeviceListModel NOTIFY inputDeviceListModelChanged)
    Q_PROPERTY(QList<QString> outputDeviceListModel READ getOutputDeviceListModel WRITE setOutputDeviceListModel NOTIFY outputDeviceListModelChanged)
    Q_PROPERTY(QString inputSampleRate READ getInputSampleRate WRITE setInputSampleRate NOTIFY inputSampleRateChanged)
    Q_PROPERTY(QString outputSampleRate READ getOutputSampleRate WRITE setOutputSampleRate NOTIFY outputSampleRateChanged)
    Q_PROPERTY(QList<QString> batchSizeListModel READ getBatchSizeListModel WRITE setBatchSizeListModel NOTIFY batchSizeListModelChanged)
    Q_PROPERTY(QString sampleChannel READ getSampleChannel WRITE setSampleChannel NOTIFY sampleChannelChanged)
    Q_PROPERTY(QString sampleBits READ getSampleBits WRITE setSampleBits NOTIFY sampleBitsChanged)

public:
    explicit DeviceControllers(QWidget *parent = nullptr);
    ~DeviceControllers();

    SoundDataBase* pSoundDataBase = nullptr;
//    FilterBoxDataBase* pFilterBoxDataBase = nullptr;
    SoundDeviceManager* pSoundDeviceManager = nullptr;
    uint32_t dataSampleRate;

    QList<QString> getInputDeviceListModel() const;
    void setInputDeviceListModel(const QList<QString>& qstringDeviceNames);
    void setInputDeviceListModel(const std::vector<std::string>& vectorDeviceNames);
    QList<QString> getOutputDeviceListModel() const;
    void setOutputDeviceListModel(const QList<QString>& qstringDeviceNames);
    void setOutputDeviceListModel(const std::vector<std::string>& vectorDeviceNames);

    QString getInputSampleRate() const;
    void setInputSampleRate(const QString& sampleRate);
    QString getOutputSampleRate() const;
    void setOutputSampleRate(const QString& sampleRate);

    QList<QString> getBatchSizeListModel() const;
    void setBatchSizeListModel(const QList<QString>& batchSizeList);

    QString getSampleChannel() const;
    void setSampleChannel(const QString& sampleChannel);
    QString getSampleBits() const;
    void setSampleBits(const QString& sampleBits);

    void connectSoundDataBase(SoundDataBase* pSoundDataBase); // done before load devices
    void connectSoundDeviceManager(SoundDeviceManager* pSoundDeviceManager); // done before load devices
    void connectLoadingDialog(LoadingDialog* pLoadingDialog); // done before load devices

    Q_INVOKABLE void searchDevices();
    Q_INVOKABLE void searchFile();
    Q_INVOKABLE void loadDevices();
    Q_INVOKABLE void unloadDevices();
    Q_INVOKABLE void selectOutputDevice(int idx);
    Q_INVOKABLE void selectInputDevice(int idx);
    Q_INVOKABLE void selectBatchSize(int idx);

private:
    QString inputFileName;
    OutState oState;
    InState iState;
    AlcInputDevice* pInputDevice = nullptr;
    AlcOutputDevice* pOutputDevice = nullptr;
    std::string inputDeviceName;
    std::string outputDeviceName;
    std::vector<std::string> inputDeviceList;
    std::vector<std::string> outputDeviceList;

    QList<QString> inputDeviceListModel;
    QList<QString> outputDeviceListModel;
    QString inputSampleRate;
    QString outputSampleRate;
    QList<QString> batchSizeListModel;

    InputType inputTypeCombo;
    OutputType outputTypeCombo;
    InputType inputTypeLoad;
    OutputType outputTypeLoad;
    ALenum alFormat;
    size_t batchSize;
    WAV_HEADER waveHeader;
    QString sampleChannel;
    QString sampleBits;

    LoadingDialog* pLoadingDialog = nullptr;

    bool loadWaveFile(std::string fileName);
    void loadOutputDevice();
    void loadInputDevice();

    void printInputDeviceCombo();
    void printOutputDeviceCombo();
    void printInputDeviceLoad();
    void printOutputDeviceLoad();

signals:
    void inputDeviceListModelChanged();
    void outputDeviceListModelChanged();
    void inputSampleRateChanged();
    void outputSampleRateChanged();
    void batchSizeListModelChanged();
    void sampleChannelChanged();
    void sampleBitsChanged();

public slots:
    void updateLoadingDialog();

};

#endif // DEVICECONTROLLERS_H
