#include "devicecontrollers.h"
#include <fstream>

DeviceControllers::DeviceControllers(QWidget *parent)
    : QWidget{parent}
{
    pOutputDevice = nullptr;
    pInputDevice = nullptr;
    this->oState = OutState::Stopped;
    this->iState = InState::Stopped;
    this->inputTypeCombo = InputType::None;
    this->outputTypeCombo = OutputType::None;
    this->inputTypeLoad = InputType::None;
    this->outputTypeLoad = OutputType::None;
    this->inputSampleRate = "-";
    this->outputSampleRate = "-";
    this->batchSize = 1024;
    this->batchSizeListModel = { QString::number(1024), QString::number(2048), QString::number(16384) };
    this->dataSampleRate = 0;
    this->sampleChannel = "-";
    this->sampleBits = "-";

    //    loadingDialog->setModal(true);
//    this->pLoadingDialog->setWindowFlag(Qt::FramelessWindowHint, false);
}

DeviceControllers::~DeviceControllers()
{
    if (pOutputDevice != nullptr) { delete pOutputDevice; pOutputDevice = nullptr; }
    if (pInputDevice != nullptr) { delete pInputDevice; pInputDevice = nullptr; }
    unloadDevices();
}

QList<QString> DeviceControllers::getInputDeviceListModel() const
{
    return this->inputDeviceListModel;
}
void DeviceControllers::setInputDeviceListModel(const QList<QString>& qstringDeviceNames)
{
    if (this->inputDeviceListModel == qstringDeviceNames) return;
    this->inputDeviceListModel = qstringDeviceNames;
    emit inputDeviceListModelChanged();
}
void DeviceControllers::setInputDeviceListModel(const std::vector<std::string>& vectorDeviceNames)
{
    QList<QString> newModel;
    newModel.reserve(vectorDeviceNames.size());
    for (const auto& entry : vectorDeviceNames)
    {
        newModel.push_back(QString::fromStdString(entry));
    }
    setInputDeviceListModel(newModel);
}

QList<QString> DeviceControllers::getOutputDeviceListModel() const
{
    return this->outputDeviceListModel;
}
void DeviceControllers::setOutputDeviceListModel(const QList<QString>& qstringDeviceNames)
{
    if (this->outputDeviceListModel == qstringDeviceNames) return;
    this->outputDeviceListModel = qstringDeviceNames;
    emit outputDeviceListModelChanged();
}
void DeviceControllers::setOutputDeviceListModel(const std::vector<std::string>& vectorDeviceNames)
{
    QList<QString> newModel;
    newModel.reserve(vectorDeviceNames.size());
    for (const auto& entry : vectorDeviceNames)
    {
        newModel.push_back(QString::fromStdString(entry));
    }
    setOutputDeviceListModel(newModel);
}

QString DeviceControllers::getInputSampleRate() const
{
    return this->inputSampleRate;
}
void DeviceControllers::setInputSampleRate(const QString &sampleRate)
{
    if (this->inputSampleRate == sampleRate) return;
    this->inputSampleRate = sampleRate;
    emit inputSampleRateChanged();
}

QString DeviceControllers::getOutputSampleRate() const
{
    return this->outputSampleRate;
}
void DeviceControllers::setOutputSampleRate(const QString &sampleRate)
{
    if (this->outputSampleRate == sampleRate) return;
    this->outputSampleRate = sampleRate;
    emit outputSampleRateChanged();
}

QList<QString> DeviceControllers::getBatchSizeListModel() const
{
    return this->batchSizeListModel;
}
void DeviceControllers::setBatchSizeListModel(const QList<QString> &batchSizeList)
{
    if (this->batchSizeListModel == batchSizeList) return;
    this->batchSizeListModel = batchSizeList;
    emit batchSizeListModelChanged();
}

QString DeviceControllers::getSampleChannel() const
{
    return this->sampleChannel;
}
void DeviceControllers::setSampleChannel(const QString &sampleChannel)
{
    if (this->sampleChannel == sampleChannel) return;
    this->sampleChannel = sampleChannel;
    emit sampleChannelChanged();
}

QString DeviceControllers::getSampleBits() const
{
    return this->sampleBits;
}
void DeviceControllers::setSampleBits(const QString &sampleBits)
{
    if (this->sampleBits == sampleBits) return;
    this->sampleBits = sampleBits;
    emit sampleBitsChanged();
}

// done before load devices
void DeviceControllers::connectSoundDataBase(SoundDataBase *pSoundDataBase)
{
    this->pSoundDataBase = pSoundDataBase;
}
// done before load devices
void DeviceControllers::connectSoundDeviceManager(SoundDeviceManager *pSoundDeviceManager)
{
    this->pSoundDeviceManager = pSoundDeviceManager;
}
// done before load devices
void DeviceControllers::connectLoadingDialog(LoadingDialog* pLoadingDialog)
{
    this->pLoadingDialog = pLoadingDialog;
}

void DeviceControllers::searchDevices()
{
    this->inputDeviceList = AlcInputDevice::GetDeviceList();
    this->outputDeviceList = AlcOutputDevice::GetDeviceList();
    setInputDeviceListModel(inputDeviceList);
    setOutputDeviceListModel(outputDeviceList);
    if (!inputDeviceList.empty()) this->inputTypeCombo = InputType::Device;
    if (!outputDeviceList.empty()) this->outputTypeCombo = OutputType::Device;
    printInputDeviceCombo();
    printOutputDeviceCombo();
}
void DeviceControllers::searchFile()
{
    this->inputFileName = QFileDialog::getOpenFileName(this, "Search Folder", QDir::homePath(), "WAVE File (*.wav)");
    if (!this->inputFileName.isEmpty() && !this->inputFileName.isNull())
    {
        setInputDeviceListModel(QList<QString>{ this->inputFileName });
        this->inputTypeCombo = InputType::File;
        printInputDeviceCombo();
    }
}
void DeviceControllers::loadDevices()
{
    if (this->pOutputDevice != nullptr) { this->pOutputDevice->stopSource(); delete pOutputDevice; pOutputDevice = nullptr; }
    if (this->pSoundDeviceManager != nullptr) { this->pSoundDeviceManager->pOutputDevice = nullptr; this->pSoundDeviceManager->cvPlayThreadIdleWait.notify_all(); }
    std::thread threadLoading([this]() {
        std::cout << "load thread (" << std::this_thread::get_id() << ") generated" << std::endl;
        if (this->inputTypeCombo == InputType::File) loadWaveFile(this->inputFileName.toStdString());
        if (this->outputTypeCombo == OutputType::Device) loadOutputDevice();
        if (this->inputTypeCombo == InputType::Device) loadInputDevice();

    //    if (this->inputTypeCombo != InputType::None) this->pSoundDataBase->setFormat(this->alFormat, this->batchSize);
        if (this->outputTypeCombo == OutputType::Device) this->pSoundDeviceManager->connectOutputDevice(this->pOutputDevice);
        if (this->inputTypeCombo == InputType::Device) this->pSoundDeviceManager->connectInputDevice(this->pInputDevice);
        if (this->inputTypeCombo != InputType::None)  {
            this->pSoundDeviceManager->frequency = this->dataSampleRate;
            this->pSoundDataBase->sampleRate = this->dataSampleRate;
        }

        // make play thread
        if (this->inputTypeLoad != InputType::None && this->outputTypeLoad != OutputType::None) {
            this->pSoundDeviceManager->makePlayThread();
        }
        // make record thread
        if (this->inputTypeLoad == InputType::Device && this->outputTypeLoad != OutputType::None) {
            this->pSoundDeviceManager->makeRecordThread();
        }

        this->pLoadingDialog->close();
        std::cout << "closed load thread (" << std::this_thread::get_id() << ")" << std::endl;
    });
    threadLoading.detach();
    this->pLoadingDialog->exec();
}
void DeviceControllers::unloadDevices()
{
    // free device
    if (pOutputDevice != nullptr) this->pSoundDeviceManager->stopPlay();
    if (pInputDevice != nullptr) this->pSoundDeviceManager->stopRecord();
//    this->pSoundDeviceManager->prepareUnload();
    this->pSoundDeviceManager->connectOutputDevice(nullptr);
    this->pSoundDeviceManager->connectInputDevice(nullptr);
    if (pOutputDevice != nullptr) { delete pOutputDevice; pOutputDevice = nullptr; }
    if (pInputDevice != nullptr) { delete pInputDevice; pInputDevice = nullptr; }
    this->pSoundDataBase->resetData();
    this->pSoundDataBase->resetFormat();
    this->pSoundDeviceManager->resetPlayer();

    // unset state
    this->inputTypeCombo = InputType::None;
    this->outputTypeCombo = OutputType::None;
    this->inputTypeLoad = InputType::None;
    this->outputTypeLoad = OutputType::None;
    this->oState = OutState::Stopped;
    this->iState = InState::Stopped;
    printInputDeviceCombo();
    printOutputDeviceCombo();
    printInputDeviceLoad();
    printOutputDeviceLoad();

    // unset meta data
    this->dataSampleRate = 0;
    this->pSoundDataBase->sampleRate = this->dataSampleRate;
    this->setSampleChannel("-");
    this->setSampleBits("-");

    // unset device lists
    setInputDeviceListModel(QList<QString>{});
    setOutputDeviceListModel(QList<QString>{});

    // unset sample rates
    setInputSampleRate(QString("-"));
    setOutputSampleRate(QString("-"));

    // notify thread to make it return
    this->pSoundDeviceManager->cvPlayThreadIdleWait.notify_all();
    this->pSoundDeviceManager->cvRecordThreadIdleWait.notify_all();

    // update filter profile
//    this->pFilterBoxDataBase->setActiveFilterIdx(0);
}

void DeviceControllers::selectOutputDevice(int idx)
{
    if (idx != -1 && idx < outputDeviceListModel.size()) this->outputDeviceName = outputDeviceListModel.at(idx).toUtf8().constData();
    else this->outputDeviceName = "";
    std::cout << "\t output device(combo) : " << this->outputDeviceName << std::endl;
}
void DeviceControllers::selectInputDevice(int idx)
{
    if (idx != -1 && idx < inputDeviceListModel.size()) this->inputDeviceName = inputDeviceListModel.at(idx).toUtf8().constData();
    else this->inputDeviceName = "";
    std::cout << "\t input device/file(combo) : " << this->inputDeviceName << std::endl;
}
void DeviceControllers::selectBatchSize(int idx)
{
    if (idx != -1 && idx < batchSizeListModel.size()) this->batchSize = batchSizeListModel.at(idx).toULongLong();
    else this->batchSize = 1024;
    std::cout << "\t batch size(combo) : " << this->batchSize << std::endl;
}

bool DeviceControllers::loadWaveFile(std::string fileName)
{
    std::ifstream ifs(fileName, std::ios::in | std::ios::binary);
    if (!ifs.good())
    {
        std::cout << "ERROR: File doesn't exist or otherwise can't load file\n"  + fileName;
        return false;
    }
    if (ifs.is_open()) {
        ifs.unsetf(std::ios::skipws);
        ifs.seekg (0, std::ios::beg);
        ifs.read(reinterpret_cast<char*>(&waveHeader), 16);
        ifs.read(reinterpret_cast<char*>(&waveHeader.Subchunk1Size), sizeof(uint32_t));
        ifs.read(reinterpret_cast<char*>(&waveHeader.AudioFormat), waveHeader.Subchunk1Size*sizeof(uint8_t));

        while (true) {
            ifs.read(reinterpret_cast<char*>(&waveHeader.Subchunk2ID[0]), 4*sizeof(uint8_t)); // read "data"
            ifs.read(reinterpret_cast<char*>(&waveHeader.Subchunk2Size), sizeof(uint32_t)); // read "subchunk_n size"
            if (waveHeader.Subchunk2ID[0] == 'd' && waveHeader.Subchunk2ID[1] == 'a' && waveHeader.Subchunk2ID[2] == 't' && waveHeader.Subchunk2ID[3] == 'a') break;
            ifs.ignore(waveHeader.Subchunk2Size);
        }

        std::vector<uint8_t> pcm(waveHeader.Subchunk2Size);
        std::cout << "vector size() = " << pcm.size() << std::endl;
        ifs.read(reinterpret_cast<char*>(pcm.data()), waveHeader.Subchunk2Size*sizeof(uint8_t));
        ifs.close();

        if (waveHeader.NumOfChan == 1) {
            this->alFormat = AL_FORMAT_MONO8;
            this->setSampleChannel("MONO");
        } else {
            this->alFormat = AL_FORMAT_STEREO8;
            this->setSampleChannel("STEREO");
        }
        if (waveHeader.bitsPerSample == 16) {
            ++this->alFormat;
            this->setSampleBits("16");
        } else {
            this->setSampleBits("8");
        }

        // set input type(load) state
        this->inputTypeLoad = InputType::File;
        printInputDeviceLoad();

        // set sample rate
        auto sampleRate = waveHeader.SamplesPerSec;
        setInputSampleRate(QString::number(sampleRate));
        this->dataSampleRate = sampleRate;
        this->pSoundDataBase->sampleRate = sampleRate;
        this->pSoundDeviceManager->frequency = sampleRate;

        // set SoundDataBase
        this->pSoundDataBase->setFormat(this->alFormat, this->batchSize);
        this->pSoundDataBase->pushPCMs(pcm.data(), waveHeader.Subchunk2Size*sizeof(uint8_t));

        // print
        std::cout << "wave file loaded : " << fileName << std::endl;
        std::cout << "  sampleRate :    " << sampleRate << std::endl;
        std::cout << "  channel count : " << waveHeader.NumOfChan << std::endl;
        std::cout << "  sample bits :   " << waveHeader.bitsPerSample << std::endl;

        return true;
    }
    return false;
}
void DeviceControllers::loadOutputDevice()
{
    // Construct output device with name
    if (this->pOutputDevice != nullptr) { delete this->pOutputDevice; this->pOutputDevice = nullptr; }
    this->pOutputDevice = new AlcOutputDevice { this->outputDeviceName };
    if (this->inputTypeCombo != InputType::File) { alFormat = AL_FORMAT_MONO16; this->setSampleBits("16"); this->setSampleChannel("MONO"); }
    this->pOutputDevice->setElementMaxAmplitude(alFormat);
    int64_t dataLowBound = -(pOutputDevice->dataElementMaxAmplitude + 1);
    if ((int)(this->alFormat)%2 == 0) dataLowBound = 0; // 8bit mono / 8bit stereo
    int64_t dataHighBound = pOutputDevice->dataElementMaxAmplitude;
    std:: cout << "dataAmplitude : " << "[" << dataLowBound << ", " << dataHighBound << "]" << std::endl;

    // set output type(load)
    this->outputTypeLoad = OutputType::Device;
    printOutputDeviceLoad();

    // set sample rate
    auto sampleRate = pOutputDevice->getDeviceFrequency();
    setOutputSampleRate(QString::number(sampleRate));

    // print
    std::cout << "output device loaded : " << this->outputDeviceName << std::endl;
    std::cout << "  device frequency :  " << sampleRate << std::endl;
}
void DeviceControllers::loadInputDevice()
{
    // Construct input device with name
    if (this->pInputDevice != nullptr) { delete this->pInputDevice; this->pInputDevice = nullptr; }

    std::string tempName = this->inputDeviceList.front();
    for (std::string idname : inputDeviceList) std::cout << "\t" << idname << std::endl;
    this->pInputDevice = new AlcInputDevice (
        this->pOutputDevice->pDevice,
        this->pOutputDevice->format,
                tempName.c_str());
//    this->pInputDevice = new AlcInputDevice (
//        this->pOutputDevice->pDevice,
//        this->pOutputDevice->format,
//        this->inputDeviceName);

    // set input type(load)
    this->inputTypeLoad = InputType::Device;
    printInputDeviceLoad();

    // set sample rate
    auto sampleRate = pOutputDevice->getDeviceFrequency();
    setInputSampleRate(QString::number(sampleRate));
    this->dataSampleRate = sampleRate;
    this->pSoundDataBase->sampleRate = sampleRate;
    this->pSoundDeviceManager->frequency = sampleRate;

    // set SoundDataBase
    this->pSoundDataBase->setFormat(this->alFormat, this->batchSize);

    // print
    std::cout << "input device loaded : " << tempName << std::endl;
    std::cout << "  device frequency :  " << sampleRate << std::endl;
}

void DeviceControllers::printInputDeviceCombo()
{
    std::cout << "input type(combo) : " << (this->inputTypeCombo == InputType::Device ? "Device" : (this->inputTypeCombo == InputType::File ? "File" : "None")) << std::endl;
}
void DeviceControllers::printOutputDeviceCombo()
{
    std::cout << "output type(combo) : " << (this->outputTypeCombo == OutputType::Device ? "Device" : "None") << std::endl;
}
void DeviceControllers::printInputDeviceLoad()
{
    std::cout << "input type(load) : " << (this->inputTypeLoad == InputType::Device ? "Device" : (this->inputTypeLoad == InputType::File ? "File" : "None")) << std::endl;
}
void DeviceControllers::printOutputDeviceLoad()
{
    std::cout << "output type(load) : " << (this->outputTypeLoad == OutputType::Device ? "Device" : "None") << std::endl;
}

void DeviceControllers::updateLoadingDialog()
{
    auto numerator = this->pSoundDataBase->batchReadCurrentCount;
    auto denominator = this->pSoundDataBase->batchReadTotalCount;
    int percentage = 100*numerator/denominator;
    this->pLoadingDialog->setPercentage(percentage);
}
