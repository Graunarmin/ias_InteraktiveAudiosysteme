// ReSharper disable CppMemberFunctionMayBeConst
#include "audiomanager.h"

struct AudioManager::Impl
{
    /// ---- Error flags ----
    mutable bool audiofault {false};
    mutable bool soundIsRunning {false};
    mutable PaError portAudioError{};

    /// ---- Audio Configuration ----
    int amountOfAudioDevices {-1};
    /// Anzahl der Samples (== Frames für portaudio), die gesammelt werden, bis die Callbackfunktion erneut aufgerufen wird.
    /// Portaudio nennt diesen Wert "framesPerBuffer", Opus nennt ihn "Samples"
    int framesPerBuffer {512};
    /// Wie oft während einer Sekunde Audio im Buffer gespeichert wird
    int sampleRate {48000}; ///48 kHz
    int audioChannels {1};
    float suggestedLatency {0.001f};
    /// buffer size is framesPerBuffer * audioChannels * 2
    unsigned short bufferSize {};
    PaSampleFormat sampleFormat {paInt16};

    DeviceManager devices{};

    /// ---- Stream/Callback Configuration ----
    PaStream *stream{};
    std::shared_ptr<PaStreamParameters> inputParameters{std::make_shared<PaStreamParameters>()};
    std::shared_ptr<PaStreamParameters> outputParameters {std::make_shared<PaStreamParameters>()};
    std::shared_ptr<CallbackData> callbackData {std::make_shared<CallbackData>()};

    /// ---- Compression ----
    bool opusEncoding {true};
    const QList<QString> trueValues = {"true", "TRUE", "True", "t", "T", "1"};
    Codec codec{nullptr};

    /// ---- send/receive audio ----
    Client client{nullptr};
    JitterBuffer jitterBuffer {nullptr};
    QElapsedTimer intervalTimer{};
    qint64 callbackInterval{0};

    /// ---- utility ----
    QMutex mtxLocker{};

    Impl() = default;
};

AudioManager::AudioManager(QObject *parent)
    : QObject{parent}
    , m(std::make_unique<Impl>())
{ }

AudioManager::~AudioManager()
{
    if(m->soundIsRunning)
    {
        m->portAudioError = Pa_CloseStream(m->stream);
        if(m->portAudioError == paNoError)
        {
            m->soundIsRunning = false;
        }
    }
}

bool AudioManager::Initialize(
    const QString& ipIn,
    const QString& portIn,
    const QString& inputDeviceIndex,
    const QString& outputDeviceIndex,
    const QString& framesPerBuffer,
    const QString& sampleRate,
    const QString& audioChannels,
    const QString& jitterBufferSize,
    const QString& encodingEnabled,
    const QString& compressionFactor)
{
    // Register the types with Qt so we can use them for signals & slots
    qRegisterMetaType<spByteArray_t>("spByteArray_t");
    qRegisterMetaType<spListSpByteArray_t>("spListSpByteArray_t");

    m->callbackData->pAudioManager = this;

    if (!InitPortAudio()) return false;

    ConfigureClient(ipIn, portIn);

    ConfigureDevices(inputDeviceIndex, outputDeviceIndex);

    ConfigureAudioParameters(framesPerBuffer, sampleRate, audioChannels);

    ConfigurePortaudioParameters();

    ConfigureJitterBuffer(jitterBufferSize);

    ConfigureCodec(encodingEnabled, compressionFactor);

    connect(&m->client, &Client::sigReceivedAudioData, this, &AudioManager::slotClientReceivedAudioData);
    connect(this, &AudioManager::sigSendAudioInputToServer, this, &AudioManager::slotSendAudioInputToServer);

    qDebug() << "... Successfully initialized.";
    qInfo() << "\n";
    qInfo() << "+++ User input required +++";
    qInfo() << "> Start audio stream? [Press any key to continue]";
    QTextStream qin(stdin);
    QString confirmation = qin.readLine();

    return true;
}

void AudioManager::StartAudioStream() const
{
    m->portAudioError = Pa_OpenStream(&(m->stream),
                                    m->inputParameters.get(),
                                    m->outputParameters.get(),
                                    m->sampleRate,
                                    m->framesPerBuffer,
                                    paClipOff,
                                    PortAudioCallback,
                                    m->callbackData.get());

    if(m->portAudioError == paNoError)
    {
        m->audiofault = false;
        qInfo() << "Starting audio stream ...";
        /// Start timer that measures the time intervals between callback function calls
        //if(!(m->intervalTimer.isValid())) m->intervalTimer.start();
        m->portAudioError = Pa_StartStream(m->stream);
        if(m->portAudioError == paNoError)
        {
            qInfo() << "Sound running ...";
            m->soundIsRunning = true;
        }
        else
        {
            qInfo() << "Sound not running.";
            m->soundIsRunning = false;
        }
    }
    else
    {
        m->audiofault = true;
        m->soundIsRunning = false;
        const auto errorText = Pa_GetErrorText(m->portAudioError);
        qWarning() << "ERROR: Audiofault. " << errorText;
    }
}

void AudioManager::ProcessAudioInput(const spByteArray_t &spInputAudioData) const
{
    spByteArray_t dataReadyToSend {spInputAudioData};
    if (m->opusEncoding)
    {
        dataReadyToSend = m->codec.Encode(spInputAudioData);
    }
    SendAudioInputToServer(dataReadyToSend);
}

bool AudioManager::GetReceivedAudioData(spByteArray_t &spReceivedData) const
{
    const bool success = m->jitterBuffer.GetNextSample(spReceivedData);
    if (success && m->opusEncoding)
    {
        spReceivedData = m->codec.Decode(spReceivedData);
    }
    return success;
}


#pragma region PRIVATE MEMBER FUNCTIONS

bool AudioManager::InitPortAudio()
{
    m->portAudioError = Pa_Initialize();

    if(m->portAudioError != paNoError)
    {
        const auto errorText = Pa_GetErrorText(m->portAudioError);
        qWarning() << "ERROR: Could not initialize PortAudio: " << errorText;
    }

    m->amountOfAudioDevices = Pa_GetDeviceCount();

    if(m->amountOfAudioDevices < 0)
    {
        qWarning() << "ERROR: Number of Devices came back negative with " << m->amountOfAudioDevices;
        return false;
    }
    return m->portAudioError == paNoError;
}

void AudioManager::ConfigureClient(const QString &ipIn, const QString &portIn)
{
    if(!m->client.InitializeForAudio(ipIn, portIn))
    {
        //ToDo: Implement this
        qWarning() << "Audiomanager: ERROR. Could not initialize Client. Proceeding without sending data to server.";
    }
}

void AudioManager::ConfigureDevices(const QString &inputDeviceIndex, const QString &outputDeviceIndex)
{
    m->devices.Initialize(m->amountOfAudioDevices, inputDeviceIndex, outputDeviceIndex);
}


void AudioManager::ConfigureAudioParameters(const QString &framesPerBuffer, const QString &sampleRate,
                                            const QString &audioChannels)
{
    bool success {false};
    int tmp_number = framesPerBuffer.toInt(&success);
    if (!success)
    {
        qWarning() << "The parameter you gave for > frames per buffer < is not a valid number. Falling back to default:" << m->framesPerBuffer;
    }
    else
    {
        m->framesPerBuffer = tmp_number;
    }
    tmp_number = sampleRate.toInt(&success);
    if (!success)
    {
        qWarning() << "The parameter you gave for > sample rate < is not a valid number. Falling back to default:" << m->sampleRate;
    }
    else
    {
        m->sampleRate = tmp_number;
    }
    tmp_number = audioChannels.toInt(&success);
    if (!success)
    {
        qWarning() << "The parameter you gave for > audio channels < is not a valid number. Falling back to default:" << m->audioChannels;
    }
    else
    {
        m->audioChannels = tmp_number;
    }

    // Set buffer size (NOT the jitterbuffer)
    m->bufferSize = m->framesPerBuffer * m->audioChannels * 2;
}

void AudioManager::ConfigurePortaudioParameters()
{
    /// Configure input parameters
    static PaMacCoreStreamInfo coreAudioInputInfo;
    ConfigurePaStreamParameters(
        m->inputParameters,
        m->devices.InputIndex(),
        coreAudioInputInfo);

    /// Configure output parameters
    static PaMacCoreStreamInfo coreAudioOutputInfo;
    ConfigurePaStreamParameters(
        m->outputParameters,
        m->devices.OutputIndex(),
        coreAudioOutputInfo);
}

void AudioManager::ConfigurePaStreamParameters(const std::shared_ptr<PaStreamParameters> &parameters,
                                 const PaDeviceIndex &deviceIndex,
                                 PaMacCoreStreamInfo& coreAudioInfo)
{
    parameters->channelCount = m->audioChannels;
    parameters->device = deviceIndex;
    parameters->sampleFormat = paInt16;
    parameters->hostApiSpecificStreamInfo = nullptr;
    parameters->suggestedLatency = m->suggestedLatency;

    PaMacCore_SetupStreamInfo(&coreAudioInfo, paMacCorePro);
    parameters->hostApiSpecificStreamInfo = &coreAudioInfo;
}

void AudioManager::ConfigureCodec(const QString& encodingEnabled, const QString& compressionFactor)
{
    if (!m->trueValues.contains(encodingEnabled))
    {
        m->opusEncoding = false;
        qInfo() << "... Encoding disabled.";
        return;
    }
    m->opusEncoding = true;
    qInfo() << "... Encoding enabled.";

    m->codec.Initialize(compressionFactor, m->sampleRate, m->framesPerBuffer, m->audioChannels);
}

void AudioManager::ConfigureJitterBuffer(const QString &bufferSize)
{
    m->jitterBuffer.Initialize(bufferSize);
    m->jitterBuffer.QueryBufferSize();
}


void AudioManager::SendAudioInputToServer(const spByteArray_t &spInputAudioData) const
{
    //qDebug() << "Audiomanager: Emitting signal to send audio input to server.";
    Q_EMIT sigSendAudioInputToServer(spInputAudioData);
}

#pragma endregion

#pragma region SLOTS

void AudioManager::slotSendAudioInputToServer(const spByteArray_t &spInputAudioData) const
{
    m->client.SendAudioData(spInputAudioData);
    //m->callbackInterval = m->intervalTimer.restart();
    //qInfo() << "Current interval time: " << m->callbackInterval;
}

void AudioManager::slotClientReceivedAudioData(const spListSpByteArray_t& data) const
{
    /// the locker is unlocked whenever the function ends or returns
    //QMutexLocker locker(&m->mtxLocker);
    m-> jitterBuffer.Add(data);
}

#pragma endregion
