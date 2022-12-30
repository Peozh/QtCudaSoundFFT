#include "AlcInputDevice.h"
#include <chrono>
#include <thread>

void 
AlcInputDevice::init() 
{    
    // allocate data with type, frequency
    {
        if ((format == AL_FORMAT_MONO8) || (format == AL_FORMAT_STEREO8)) {
            data.resize(frequency*2);// = new int8_t[frequency*2];
        } else {
            data.resize(frequency*2*2);// = new int16_t[frequency*2];
        }
    }
    std::cout << "Initialized Input Device : " << inputDeviceName << '\n';
}

AlcInputDevice::AlcInputDevice(ALCdevice* pOutputDevice, ALCenum format)
    : format(format)
{
    // Open default input Device
    alcGetIntegerv(pOutputDevice, ALC_FREQUENCY, 1, &frequency);
    pDevice = alcCaptureOpenDevice(nullptr, frequency, format, frequency/2);
    if ((alcError = alcGetError(pDevice)) != ALC_NO_ERROR)
    {
        DisplayALError("AlcInputDevice : ", alcError);
        return;
    }
    if (pDevice == nullptr) std::cout << "!! input device's pDevice is nullptr. fail to open device." << std::endl;
    inputDeviceName = alcGetString(nullptr, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
    std::cout << "Opened Default Input Device : " << inputDeviceName << '\n';

    init();
}

AlcInputDevice::~AlcInputDevice() 
{
    // delete[] data;
    alcCaptureStop(pDevice);
    alcCaptureCloseDevice(pDevice); 
    std::cout << "Closed Input Device : " << inputDeviceName << std::endl;
}

AlcInputDevice::AlcInputDevice(ALCdevice* pOutputDevice, ALCenum format, std::string inputDeviceName) 
    : inputDeviceName(inputDeviceName), format(format)
{
    // Open requested output Device
    alcGetIntegerv(pOutputDevice, ALC_FREQUENCY, 1, &frequency);
    pDevice = alcCaptureOpenDevice(inputDeviceName.c_str(), frequency, format, frequency/2);
    if ((alcError = alcGetError(pDevice)) != ALC_NO_ERROR)
    {
        DisplayALError("AlcInputDevice : ", alcError);
        return;
    }
    if (pDevice == nullptr) std::cout << "!! input device's pDevice is nullptr. fail to open device." << std::endl;
    std::cout << "Opened Selected Input Device : " << inputDeviceName << '\n';
    
    init();
}


void AlcInputDevice::startCapture()
{
    alcCaptureStart(pDevice);
    capturing = true;
    if ((alcError = alcGetError(pDevice)) != ALC_NO_ERROR)
    {
        DisplayALError("startCapture : ", alcError);
        return;
    }
    std::cout << "Started Capture" << '\n';
}

void AlcInputDevice::stopCapture()
{
    alcCaptureStop(pDevice);
    capturing = false;
    if ((alcError = alcGetError(pDevice)) != ALC_NO_ERROR)
    {
        DisplayALError("stopCapture : ", alcError);
        return;
    }
    std::cout << "Stopped Capture" << '\n';
}

ALuint AlcInputDevice::getCapturedSampleCount()
{
    ALCint count;
    alcGetIntegerv(pDevice, ALC_CAPTURE_SAMPLES, 1, &count);
    return count;
}

void AlcInputDevice::attachCapturedSamplesToData(ALCsizei captureSize)
{
    alcCaptureSamples(pDevice, data.data(), captureSize);
}

bool AlcInputDevice::isCapturing() const
{
    return capturing;
}
