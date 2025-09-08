#include "audiomanager.h"

struct AudioManager::Impl
{
    /// ---- Error flags ----
    mutable bool audiofault {false};
    mutable bool soundIsRunning {false};
    mutable PaError portAudioError{};

    /// ---- Audio Configuration ----
    int amountOfAudioDevices {-1};
    /// Anzahl der Samples (Frames?), die gesammelt werden, bis die Callbackfunktion das nächste Mal aufgerufen wird
    int framesPerBuffer {512};
    /// Wie oft während einer Sekunde Audio im Buffer gespeichert wird
    int sampleRate {48000}; ///48 kHz
    float suggestedLatency {0.001f};
    int audioChannels {1};
    /// buffer size is computed later as framesPerBuffer * audioChannels * 2
    unsigned short bufferSize {};
    PaSampleFormat sampleFormat {paInt16};
    int inputDeviceIndex {0};
    int outputDeviceIndex {1};

    /// OPUS
    int opusError {};
    const int maxCompressedLength{framesPerBuffer * 4};

    OpusCustomMode *opusMode {};
    OpusCustomEncoder *encoder {};
    OpusCustomDecoder *decoder {};


    /// ---- Stream/Callback Configuration ----
    PaStream *stream{};
    std::shared_ptr<PaStreamParameters> inputParameters{std::make_shared<PaStreamParameters>()};
    std::shared_ptr<PaStreamParameters> outputParameters {std::make_shared<PaStreamParameters>()};
    std::shared_ptr<CallbackData> callbackData {std::make_shared<CallbackData>()};

    /// ---- Send audio ----
    Client client{nullptr};
    QQueue<spAudioData_t> queuedPointersToReturnedAudioDataBuffers{};
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
    opus_custom_encoder_destroy(m->encoder);
    opus_custom_decoder_destroy(m->decoder);
    opus_custom_mode_destroy(m->opusMode);
}

bool AudioManager::Initialize(
    const QString& framesPerBuffer,
    const QString& sampleRate,
    const QString& audioChannels,
    const QString& inputDeviceIndex,
    const QString& outputDeviceIndex,
    const QString& ipIn,
    const QString& portIn)
{
    qRegisterMetaType<spAudioData_t>("spAudioData_t");

    m->callbackData->pAudioManager = this;

    if(!m->client.InitializeForAudio(ipIn, portIn)) return false;

    InitPortAudio();
    if(m->portAudioError != paNoError) return false;

    if (!ConfigureAudioDevices(inputDeviceIndex, outputDeviceIndex)) return false;
    if (!ConfigureAudioParameters(framesPerBuffer, sampleRate, audioChannels)) return false;

    ConfigurePortaudioParameters();

    ConfigureOpus();

    connect(&m->client, &Client::signalReceivedAudioData,this, &AudioManager::slotReceivedAudioData);
    connect(this, &AudioManager::sigSendAudioInputToServer, this, &AudioManager::slotSendAudioInputToServer);

    qDebug() << "Successfully initialized.";
    return true;
}

void AudioManager::StartAudioStream() const {
    qDebug() << "Running ...";

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
        qDebug() << "Starting Stream ...";
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

bool AudioManager::GetReceivedAudioData(spAudioData_t &spReflectedAudioData) const
{
    bool success = false;
    //QMutexLocker locker(&m->mtxLocker);

    if (!m->queuedPointersToReturnedAudioDataBuffers.empty())
    {
        spReflectedAudioData = m->queuedPointersToReturnedAudioDataBuffers.dequeue();
        success = true;
    }
    return success;
}

void AudioManager::SendAudioInputToServer(const spAudioData_t &spInputAudioData) const
{
    // ToDo: Add option for encoding
    auto encodedData = EncodeWithOpus(spInputAudioData);
    Q_EMIT sigSendAudioInputToServer(encodedData);
}

spAudioData_t AudioManager::EncodeWithOpus(const spAudioData_t &spInputAudioData) const
{
    const opus_int16* inputForOpus = reinterpret_cast<const opus_int16 *> (spInputAudioData->data());
    unsigned char * encodedData = new unsigned char[m->maxCompressedLength]();

    int length = opus_custom_encode(m->encoder, inputForOpus, m->framesPerBuffer,
                                    encodedData, m->maxCompressedLength);
    if (length < 0)
    {
        qInfo() << "Audiomanager: Opus encountered an error while encoding input Data.";
    }
    const char * encodedInput = reinterpret_cast<const char *> (encodedData);
    auto spInput = std::make_shared<QByteArray>(encodedInput, length);

    opus_custom_encoder_ctl(m->encoder, OPUS_RESET_STATE);
    return spInput;
}

bool AudioManager::DecodeWithOpus(spAudioData_t &spReflectedAudioData)
{
    return false;
}


#pragma region PRIVATE MEMBER FUNCTIONS

void AudioManager::InitPortAudio() {
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
    }
}

bool AudioManager::ConfigureAudioDevices(const QString& inDeviceIndex, const QString& outDeviceIndex)
{
    LogAudioDeviceInformation(m->amountOfAudioDevices);
    bool success = false;
    QTextStream qin(stdin);

    qInfo() << "The index for the input device is currently set to " << m->inputDeviceIndex<<
        " and the index for the output device is set to " << m->outputDeviceIndex << ".";
    qInfo() << "Do you wish to change these settings? [y/n]";
    QString confirmation = qin.readLine();
    if(confirmation != "y")
    {
        m->inputDeviceIndex = inDeviceIndex.toInt(&success);
        if (!success)
        {
            qWarning() << "The index for the input device is not a valid number.";
            return success;
        }

        m->outputDeviceIndex = outDeviceIndex.toInt(&success);
        if (!success)
        {
            qWarning() << "The index for the output device is not a valid number.";
            return success;
        }
        return success;
    }

    while(!success)
    {
        qInfo() << "Please check the device list above and enter the index of the input device you wish to use: ";
        QString inputIndex = qin.readLine();
        success = VerifyDeviceIndex(inputIndex, m->amountOfAudioDevices, m->inputDeviceIndex);
    }
    success = false;
    while(!success)
    {
        qInfo() << "And the index or the output device: ";
        QString outputIndex = qin.readLine();
        success = VerifyDeviceIndex(outputIndex, m->amountOfAudioDevices, m->outputDeviceIndex);
    }
    qDebug() << "New index for input device: " << m->inputDeviceIndex << ", new index for output device: " << m->outputDeviceIndex;
    return success;
}

bool AudioManager::ConfigureAudioParameters(const QString& framesPerBuffer, const QString& sampleRate, const QString& audioChannels)
{
    bool success {false};
    m->framesPerBuffer = framesPerBuffer.toInt(&success);
    if (!success)
    {
        qWarning() << "The parameter you gave for > frames per buffer < is not a valid number.";
        return success;
    }
    m->sampleRate = sampleRate.toInt(&success);
    if (!success)
    {
        qWarning() << "The parameter you gave for > sample rate < is not a valid number.";
        return success;
    }
    m->audioChannels = audioChannels.toInt(&success);
    if (!success)
    {
        qWarning() << "The parameter you gave for > audio channels < is not a valid number.";
        return success;
    }
    m->bufferSize = m->framesPerBuffer * m->audioChannels * 2;
    return success;
}

void AudioManager::ConfigurePortaudioParameters() {
    /// Configure input parameters
    static PaMacCoreStreamInfo coreAudioInputInfo;
    ConfigurePaStreamParameters(
        m->inputParameters,
        m->audioChannels,
        m->inputDeviceIndex,
        paInt16,
        coreAudioInputInfo,
        m->suggestedLatency);

    /// Configure output parameters
    static PaMacCoreStreamInfo coreAudioOutputInfo;
    ConfigurePaStreamParameters(
        m->outputParameters,
        m->audioChannels,
        m->outputDeviceIndex,
        paInt16,
        coreAudioOutputInfo,
        m->suggestedLatency);
}

void AudioManager::ConfigureOpus()
{
    m->opusMode = opus_custom_mode_create(m->sampleRate, m->framesPerBuffer, &m->opusError);
    if (m->opusError != OPUS_OK) {
        qInfo() << "Audiomanager: Cannot create Opus Mode - Error: " << opus_strerror(m->opusError);
        exit(EXIT_FAILURE);
    }

    m->encoder = opus_custom_encoder_create(m->opusMode, m->audioChannels, &m->opusError);
    if (m->opusError != OPUS_OK) {
        qInfo() << "Audiomanager: Cannot create Opus Encoder - Error: " << opus_strerror(m->opusError);
        exit(EXIT_FAILURE);
    }

    m->decoder = opus_custom_decoder_create(m->opusMode, m->audioChannels, &m->opusError);
    if(m->opusError != OPUS_OK)
    {
        qInfo() << "Audiomanager: OpusCustomDecoder creation failed: " << opus_strerror(m->opusError);
    }
}

#pragma endregion

#pragma region SLOTS

void AudioManager::slotSendAudioInputToServer(const spAudioData_t &spInputAudioData)
{
    m->client.SendAudioData(spInputAudioData);
    //m->callbackInterval = m->intervalTimer.restart();
    //qInfo() << "Current interval time: " << m->callbackInterval;
}

void AudioManager::slotReceivedAudioData(const spAudioData_t& spReflectedAudioData)
{
    /// the locker is unlocked whenever the function ends or returns
    //QMutexLocker locker(&m->mtxLocker);
    m->queuedPointersToReturnedAudioDataBuffers.enqueue(spReflectedAudioData);
}

#pragma endregion





