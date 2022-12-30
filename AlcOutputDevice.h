#ifndef ALCOUTPUTDEVICE_H
#define ALCOUTPUTDEVICE_H

#include <al.h>
#include <alc.h>

#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <queue>

class AlcOutputDevice {
public:
    static constexpr ALsizei bufferCount = 3;

private:
    std::string outputDeviceName;
    ALCcontext* pContext = nullptr;
    
    ALuint bufferIds[bufferCount];
    ALuint sourceIds;

    ALCenum alcError = ALC_NO_ERROR;
    ALenum alError = AL_NO_ERROR;

    void init(); // Context, Buffer, Source, Listener

public:
    AlcOutputDevice(); // Device, init()
    ~AlcOutputDevice();
    AlcOutputDevice(std::string outputDeviceName); // Device, init()
    
    void setElementMaxAmplitude(const ALenum alFormat); // set PCM data format
    void setBufferData(const void* data, std::size_t dataElementCount, ALsizei dataFrequency = 48000); // Attach PCM data to Buffer
    void setBufferData(ALuint bufferId, const void* data, std::size_t dataElementCount, ALsizei dataFrequency = 48000); // Attach PCM data to Buffer (Queued way)
    void attachBufferToSource(); // Attach Buffer to Source
    void attachBufferToSourceQueue(const ALuint* bufferId); // Attach Buffer to Source (Queued way)
    void detachBufferFromSourceQueue(ALint playedBufferCount); // Detach Buffer from Source (Queued way)
    void playSource(); // Play Source
    void pollingUntilDone(size_t milliseconds = 500); // hold polling to get finish state ()
    void stopSource(); // 의미없는듯

    ALint getSourceState();
    ALint getDeviceFrequency();
    ALint getPlayedBufferCount();

    ALCdevice* pDevice = nullptr;

    ALenum format;
    ALsizei elementSize;

    size_t  dataElementCount;
    int16_t dataElementMaxAmplitude;
    ALsizei dataFrequency;
    
    std::queue<ALuint> queueableBufferIds;
    ALuint unqueuedBufferIds[bufferCount];

    static std::vector<std::string> GetDeviceList() 
    {
        std::vector<std::string> outputDeviceNames;
        const ALCchar* pNames = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
        if (pNames == nullptr) {
            return outputDeviceNames;
        }
        outputDeviceNames.push_back(pNames);

        auto pNextName = pNames;
        while(*(pNextName += strlen(pNames) + 1) != '\0') {
            pNames = pNextName;
            outputDeviceNames.push_back(pNames);
        }

        return outputDeviceNames;
    }

    static void DisplayALError(std::string prefix, ALenum error) 
    {
        std::cout << prefix << error << '\n';
    }
};

#endif // ALCOUTPUTDEVICE_H
