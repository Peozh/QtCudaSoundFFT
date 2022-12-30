#include "AlcOutputDevice.h"
#include <chrono>
#include <thread>

void 
AlcOutputDevice::init() 
{
    // get Context
    {
        if (pDevice != nullptr) 
        { 
            pContext = alcCreateContext(pDevice, nullptr); 
            alcMakeContextCurrent(pContext);
            if ((alcError = alcGetError(pDevice)) != ALC_NO_ERROR)
            {
                DisplayALError("alcMakeContextCurrent : ", alcError);
                return;
            }
            std::cout << "Created Context" << '\n';
        }
    }
    
    // make Buffer names
    {
        alError = alGetError(); // clear error
        alGenBuffers(bufferCount, bufferIds);
        if ((alError = alGetError()) != AL_NO_ERROR)
        {
            DisplayALError("alGenBuffers : ", alError);
            return;
        }
        
        std::cout << "Generated Buffer" << '\n';

        for (size_t idx = 0; idx < bufferCount; ++idx) {
            queueableBufferIds.push(bufferIds[idx]);
        }
    }

    // set Sources
    {
        alError = alGetError(); // clear error
        ALsizei sourceCount = ALsizei{1};
        alGenSources(sourceCount, &sourceIds);
        if ((alError = alGetError()) != AL_NO_ERROR)
        {
            DisplayALError("alGenSources 1 : ", alError);
            return;
        } 
        alSource3f(sourceIds, AL_POSITION, 0.0f, 0.0f, 0.0f);
        alSource3f(sourceIds, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        alSourcef(sourceIds, AL_GAIN, 1.0f); // 음 크기
        alSourcef(sourceIds, AL_PITCH, ALfloat{1.0f}); // 음높이(재생속도) 빠른 재생 / 늘여서 재생
        
        std::cout << "Generated Source" << '\n';
    }

    // set listener
    {
        alError = alGetError(); // clear error
        ALfloat listenerPos[]={0.0, 0.0, 0.0}; 
        ALfloat listenerVel[]={0.0, 0.0, 0.0}; 
        ALfloat listenerOri[]={0.0, 0.0, -1.0, 0.0, 1.0, 0.0}; 
        // Position ... 
        alListenerfv(AL_POSITION, listenerPos); 
        if ((alError = alGetError()) != AL_NO_ERROR) 
        { 
            DisplayALError("alListenerfv POSITION : ", alError); 
            return; 
        } 
        // Velocity ... 
        alListenerfv(AL_VELOCITY, listenerVel); 
        if ((alError = alGetError()) != AL_NO_ERROR) 
        { 
            DisplayALError("alListenerfv VELOCITY : ", alError); 
            return; 
        } 
        // Orientation ... 
        alListenerfv(AL_ORIENTATION, listenerOri); 
        if ((alError = alGetError()) != AL_NO_ERROR) 
        { 
            DisplayALError("alListenerfv ORIENTATION : ", alError); 
            return; 
        }
        
        std::cout << "Set Listener" << '\n';
    }
    std::cout << "Initialized Output Device : " << outputDeviceName << '\n';
}

AlcOutputDevice::AlcOutputDevice() 
{
    // Open default output Device
    pDevice = alcOpenDevice(nullptr);
    if ((alcError = alcGetError(pDevice)) != ALC_NO_ERROR)
    {
        DisplayALError("AlcOutputDevice : ", alcError);
        return;
    }
    outputDeviceName = alcGetString(nullptr, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
    std::cout << "Opened Default Output Device : " << outputDeviceName << '\n';
    
    init();
}

AlcOutputDevice::~AlcOutputDevice() 
{
    alSourceStop(sourceIds);
    alSourcei(sourceIds, AL_BUFFER, 0);

    alDeleteSources(1, &sourceIds);
    alDeleteBuffers(bufferCount, bufferIds);
    alcMakeContextCurrent(nullptr);
    if ((alcError = alcGetError(pDevice)) != ALC_NO_ERROR)
    {
        DisplayALError("alcMakeContextCurrent : ", alcError);
        return;
    }
    alcDestroyContext(pContext);
    alcCloseDevice(pDevice); 
    std::cout << "Closed Output Device : " << outputDeviceName << std::endl;
}

AlcOutputDevice::AlcOutputDevice(std::string outputDeviceName) 
    : outputDeviceName(outputDeviceName)
{
    // Open requested output Device
    pDevice = alcOpenDevice(outputDeviceName.c_str());
    if ((alcError = alcGetError(pDevice)) != ALC_NO_ERROR)
    {
        DisplayALError("AlcOutputDevice : ", alcError);
        return;
    }
    std::cout << "Opened Selected Output Device : " << outputDeviceName << std::endl;

    init();
}

void 
AlcOutputDevice::setElementMaxAmplitude(
    const ALenum alFormat) 
{
    // Set PCM data format
    this->format = alFormat;
    if ((format == AL_FORMAT_MONO8) || (format == AL_FORMAT_STEREO8)) {
        dataElementMaxAmplitude = (1ULL << 8) - 1;
        elementSize = 1;
    }
    else if ((format == AL_FORMAT_MONO16) || (format == AL_FORMAT_STEREO16)) {
        dataElementMaxAmplitude = (1ULL << 15) - 1;
        elementSize = 2;
    }
    std::cout << "Set Source Format" << '\n';
}


void 
AlcOutputDevice::setBufferData(
    const void* data, 
    std::size_t dataElementCount,
    ALsizei dataFrequency)
{
    this->dataElementCount = dataElementCount;
    this->dataFrequency = dataFrequency;

    // Attach PCM data to Buffer
    ALsizei dataTotalByteSize = elementSize * dataElementCount;
    alBufferData(bufferIds[0], format, data, dataTotalByteSize, dataFrequency);
    if ((alError = alGetError()) != AL_NO_ERROR)
    {
        DisplayALError("alBufferData buffer 0 : ", alError);
        alDeleteBuffers(1, &bufferIds[0]);
        return;
    }
}

void 
AlcOutputDevice::setBufferData(
    ALuint bufferId,
    const void* data, 
    std::size_t dataElementCount,
    ALsizei dataFrequency)
{
    this->dataElementCount = dataElementCount;
    this->dataFrequency = dataFrequency;

    // Attach PCM data to Buffer
    ALsizei dataTotalByteSize = elementSize * dataElementCount;
    alBufferData(bufferId, format, data, dataTotalByteSize, dataFrequency);
    if ((alError = alGetError()) != AL_NO_ERROR)
    {
        DisplayALError("alBufferData buffer 0 : ", alError);
        alDeleteBuffers(1, &bufferId);
        return;
    }
}

// todo : check what buffer to use
void
AlcOutputDevice::attachBufferToSource()
{
    // Attach Buffer to Source
    alSourcei(sourceIds, AL_BUFFER, bufferIds[0]);
    if ((alError = alGetError()) != AL_NO_ERROR)
    {
        DisplayALError("alSourcei AL_BUFFER 0 : ", alError);
        return;
    }
}
void
AlcOutputDevice::attachBufferToSourceQueue(const ALuint* bufferId)
{
    // Attach Buffer to Source (Queued way)
    alSourceQueueBuffers(sourceIds, 1, bufferId);
    if ((alError = alGetError()) != AL_NO_ERROR)
    {
        DisplayALError("attachBufferToSourceQueue  : ", alError);
        return;
    }
}

void 
AlcOutputDevice::detachBufferFromSourceQueue(ALint playedBufferCount)
{
    // Detach Buffer from Source (Queued way)
    alSourceUnqueueBuffers(sourceIds, playedBufferCount, unqueuedBufferIds);
    for (int idx = 0; idx < playedBufferCount; ++idx) {
        // Push the recovered buffers back on the queue
        queueableBufferIds.push(unqueuedBufferIds[idx]);
    }
} 

void
AlcOutputDevice::playSource()
{
    // play source
    alSourcePlay(sourceIds);
    // while (getSourceState() != AL_STOPPED) { // polling until finished
    //     std::chrono::milliseconds timespan(500); // 500 or whatever
    //     std::this_thread::sleep_for(timespan);
    // }
    std::cout << "Playing Source" << '\n';
}

// Parameter milliseconds is check period. Hold main thread not to be terminated
void 
AlcOutputDevice::pollingUntilDone(size_t milliseconds)
{
    while (getSourceState() != AL_STOPPED) { // polling until finished
        std::chrono::milliseconds timespan(milliseconds); // 500 or whatever
        std::this_thread::sleep_for(timespan);
    }
    std::cout << "Played Source" << '\n';
}

void
AlcOutputDevice::stopSource()
{
    // stop source
    alSourceStopv(1, &sourceIds); // 소스가 정지되면 모든 대기열 attached buffer 가 처리된 상태로 간주됨
    // https://stackoverflow.com/questions/16835316/openal-unqueueing-error-code-incomplete-documentation

//    detachBufferFromSourceQueue(getPlayedBufferCount()); // 쓰레드 하나에서 이미 재생하고 있으므로, 버퍼를 모두 빼고 새로운 쓰레드에서 재생하면 오류 발생 가능성

//    alSourcei(sourceIds, AL_BUFFER, 0); // 버퍼 연결 해제 및 삭제



//    alError = alGetError(); // clear error
//    alGenBuffers(this->bufferCount, bufferIds);
//    if ((alError = alGetError()) != AL_NO_ERROR)
//    {
//        DisplayALError("alGenBuffers : ", alError);
//        return;
//    }

//    std::cout << "Generated Buffer" << '\n';

//    while(queueableBufferIds.size() > 0) queueableBufferIds.pop();

//    for (size_t idx = 0; idx < this->bufferCount; ++idx) {
//        queueableBufferIds.push(bufferIds[idx]);
//    }
}
    
ALint
AlcOutputDevice::getSourceState() 
{
    ALint sourceState;
    alGetSourcei(sourceIds,  AL_SOURCE_STATE, &sourceState);
    return sourceState;
}

ALint
AlcOutputDevice::getDeviceFrequency()
{
    ALint frequency;
    alcGetIntegerv(pDevice, ALC_FREQUENCY, 1, &frequency);
    return frequency;
}

ALint 
AlcOutputDevice::getPlayedBufferCount()
{
    ALint count;
    alGetSourcei(sourceIds, AL_BUFFERS_PROCESSED, &count);
    return count;
}
