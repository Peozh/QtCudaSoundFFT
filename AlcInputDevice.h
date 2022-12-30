#ifndef ALCINPUTDEVICE_H
#define ALCINPUTDEVICE_H

#include <al.h>
#include <alc.h>

#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <array>

class AlcInputDevice {

private:
    std::string inputDeviceName;
    ALCdevice* pDevice = nullptr;
    
    ALCenum alcError = ALC_NO_ERROR;
    ALenum alError = AL_NO_ERROR;

    ALCint frequency;
    ALCenum format;

    bool capturing;

    void init(); // Context, Buffer, Source, Listener

public:
    std::vector<uint8_t> data;

    AlcInputDevice(ALCdevice* pOutputDevice, ALCenum format); // Device, init()
    ~AlcInputDevice();
    AlcInputDevice(ALCdevice* pOutputDevice, ALCenum format, std::string inputDeviceName); // Device, init()

    void startCapture();
    void stopCapture();

    ALuint getCapturedSampleCount();
    void attachCapturedSamplesToData(ALCsizei captureSize);
    bool isCapturing() const;

    
    static std::vector<std::string> GetDeviceList() 
    {
        std::vector<std::string> inputDeviceNames;
        const ALCchar* pNames = alcGetString(nullptr, ALC_CAPTURE_DEVICE_SPECIFIER);
        if (pNames == nullptr || *pNames == '\0') {
            return inputDeviceNames;
        }
        inputDeviceNames.push_back(pNames);

        auto pNextName = pNames;
        while(*(pNextName += strlen(pNames) + 1) != '\0') {
            pNames = pNextName;
            inputDeviceNames.push_back(pNames);
        }
        return inputDeviceNames;
    }

    static void DisplayALError(std::string prefix, ALenum error) 
    {
        std::cout << prefix << error << '\n';
    }
};

#endif // ALCINPUTDEVICE_H
