#include "audiomanager.h"

static int PortAudioCallback(const void* input, void* output,
                             unsigned long frameCount,
                             const PaStreamCallbackTimeInfo timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void* userData);


struct AudioManager::Impl
{
    bool audiofault = {false};
    bool soundIsRunning = {false};

    int amountOfAudioDevices = {-1};
    /// Anzahl der Samples, die jedesmal generiert wird
    int frameSize = {512};
    /// 48 kHz
    int sampleRate = {48000};
    int audioChannels = {1};
    unsigned short bufferSize = {0};

    PaSampleFormat sampleFormat = {paInt16};

    std::shared_ptr<CallbackData> additionalData = {std::make_shared<CallbackData>()};

    std::shared_ptr<PaStreamParameters> inputParameters = {std::make_shared<PaStreamParameters>()};
    std::shared_ptr<PaStreamParameters> outputParameters = {std::make_shared<PaStreamParameters>()};

    PaStream* stream;

    Impl(){}
};

AudioManager::AudioManager(QObject *parent)
    : QObject{parent}
    , m(std::make_shared<Impl>())
{ }

bool AudioManager::Initialize(bool audiofault = false, bool soundIsRunning = false,
                              int frameSize = 512, int sampleRate = 48000, int audioChannels = 1)
{
    if(!InitPortAudio()) return false

    LogAudioDeviceInformation();

    m->audiofault = audiofault;
    m->soundIsRunning = soundIsRunning;
    m->frameSize = frameSize;
    m->sampleRate = sampleRate;
    m->audioChannels = audioChannels;
    m->bufferSize = m->frameSize * m->audioChannels * 2;

    ConfigureAudioDevice();
}

void AudioManager::Run()
{
    PaError paError = Pa_OpenStream(&(m->stream),
                                    &(m->inputParameters.get()),
                                    &(m->outputParameters.get()),
                                    m->sampleRate,
                                    m->frameSize,
                                    paClipOff,
                                    PortAudioCallback,
                                    *(m->additionalData.get()));

    if(paError != paNoError)
    {
        m->audiofault = true;
        return;
    }
    m->audiofault = false;
    paError = Pa_StartStream(*(m->stream.get()));

    if(paError != paNoError){
        m->soundIsRunning = false;
        qInfo() << "Sound is not running.";
        return;
    }

    m->soundIsRunning = true;
    qInfo() << "Sound running ...";

    // ToDo: Combine with Client to send Data to server
}



bool AudioManager::InitPortAudio()
{
    PaError success = Pa_Initialize();

    if(success != paNoError)
    {
        qWarning() << "Could not initialize PortAudio.";
        return false;
    }

    m->amountOfAudioDevices = Pa_GetDeviceCount();

    if(m->amountOfAudioDevices < 0)
    {
        qWarning() << "ERROR: Number of Devices came back negative with " << m->amountOfAudioDevices;
        return false;
    }
    return true;
}

void AudioManager::LogAudioDeviceInformation()
{
    for(int i = 0; i <= m->AmountOfAudioDevices; i++)
    {
        auto deviceInfo = Pa_GetDeviceInfo(i);

        qInfo() << "Device " << i;
        qInfo() << "Name: " << deviceInfo->name;
        qInfo() << "Max. output channels: " << deviceInfo->maxOutputChannels;
        qInfo() << "Max. input channels: " << deviceInfo->maxInputChannels;
        qInfo() << "Default sample rate: " << deviceInfo->defaultSampleRate;
    }
}

bool AudioManager::ConfigureAudioDevice()
{
    ConfigurePaStreamParameters(m->inputParameters, m->audioChannels, 2, paInt16);
#ifdef __MACOSX_CORE__
    static PaMacCoreStreamInfo coreAudioInputInfo;
    PaMacCore_SetupStreamInfo(&coreAudioInputInfo, paMacCorePro);
    inputParameters.hostApiSpecificStreamInfo = &coreAudioInputInfo;
#endif

    ConfigurePaStreamParameters(m->outputParameters, m->audioChannels, 3, paInt16);
#ifdef __MACOSX_CORE__
    static PaMacCoreStreamInfo coreAudioOutputInfo;
    PaMacCore_SetupStreamInfo(&coreAudioOutputInfo, paMacCorePro);
    outputParameters.hostApiSpecificStreamInfo = &coreAudioOutputInfo;
#endif

    inputParameters->suggestedLatency = 0.001;
    outputParameters->suggestedLatency = 0.001;
}

void AudioManager::ConfigurePaStreamParameters(std::shared_ptr<PaStreamParameters> parameters,
                                               int channelCount, PaDeviceIndex deviceIndex, PaSampleFormat sampleFormat)
{
    // ToDo: Fill parameter with zeros
    parameters->channelCount = channelCount;
    parameters->device = deviceIndex;
    parameters->sampleFormat = sampleFormat;
    parameters->hostApiSpecificStreamInfo = NULL;
}




