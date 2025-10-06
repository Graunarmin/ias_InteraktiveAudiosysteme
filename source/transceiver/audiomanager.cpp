// ReSharper disable CppMemberFunctionMayBeConst
#include "audiomanager.h"

struct AudioManager::Impl
{
    /// ---- Error flags ----
    mutable bool audiofault {false};
    mutable bool soundIsRunning {false};
    mutable PaError portAudioError{};
    mutable int opusError {};

    /// ---- Audio Configuration ----
    int amountOfAudioDevices {-1};
    /// Anzahl der Samples (Frames?), die gesammelt werden, bis die Callbackfunktion das nächste Mal aufgerufen wird
    /// Portaudio nennt diesen Wert "framesPerBuffer", Opus nennt ihn "Samples"
    int framesPerBuffer {512};
    /// Wie oft während einer Sekunde Audio im Buffer gespeichert wird
    int sampleRate {48000}; ///48 kHz
    float suggestedLatency {0.001f};
    int audioChannels {1};
    /// buffer size is framesPerBuffer * audioChannels * 2
    unsigned short bufferSize {};
    PaSampleFormat sampleFormat {paInt16};
    int inputDeviceIndex {0};
    int outputDeviceIndex {1};

    /// ---- Stream/Callback Configuration ----
    PaStream *stream{};
    std::shared_ptr<PaStreamParameters> inputParameters{std::make_shared<PaStreamParameters>()};
    std::shared_ptr<PaStreamParameters> outputParameters {std::make_shared<PaStreamParameters>()};
    std::shared_ptr<CallbackData> callbackData {std::make_shared<CallbackData>()};

    /// ---- OPUS ----
    const QList<QString> trueValues = {"true", "TRUE", "True", "t", "T", "1"};
    bool opusEncoding {true};
	const int compressionFactor = 8;
    const int maxCompressedLength{framesPerBuffer*2/compressionFactor};
    OpusCustomMode *opusMode {};
    OpusCustomEncoder *encoder {};
    OpusCustomDecoder *decoder {};

    /// ---- Send audio ----
    Client client{nullptr};
    QQueue<spAudioData_t> queuedPointersToReturnedAudioDataBuffers{};
    QElapsedTimer intervalTimer{};
    qint64 callbackInterval{0};
	signed int jitterBufferSize {50};
	bool waitingForJitterBuffer {true};
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
    const QString& encodingEnabled,
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
    ConfigureOpusEncoding(encodingEnabled);

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

void AudioManager::ProcessAudioInput(const spAudioData_t &spInputAudioData) const
{
    spAudioData_t dataReadyToSend {spInputAudioData};
    if (m->opusEncoding)
    {
        dataReadyToSend = EncodeWithOpus(spInputAudioData);
    }
    SendAudioInputToServer(dataReadyToSend);
}

bool AudioManager::GetReceivedAudioData(spAudioData_t &spReflectedAudioData) const
{
    bool success = false;
    //QMutexLocker locker(&m->mtxLocker);

	if (waitingForJitterBuffer) {
		if (m->queuedPointersToReturnedAudioDataBuffers.count() >= m->jitterBufferSize) {
			m->waitingForJitterBuffer = false;
			qDebug() << "Jitter buffer reached limit. Start audio output.";
			if (m->opusEncoding)
        	{
            	spReflectedAudioData = DecodeWithOpus(m->queuedPointersToReturnedAudioDataBuffers.dequeue());
        	}
        	else
        	{
            	spReflectedAudioData = m->queuedPointersToReturnedAudioDataBuffers.dequeue();
        	}
        	success = true;
		}
		else
		{
			qDebug() << "Jitter buffer not reached limit. Waiting for audio data.";
		}
	} else {
		if (m->queuedPointersToReturnedAudioDataBuffers.isEmpty()) {
			m->waitingForJitterBuffer = true;
			qDebug() << "Jitter buffer ran empty.";
		}
		else
    	{
        	if (m->opusEncoding)
        	{
            	spReflectedAudioData = DecodeWithOpus(m->queuedPointersToReturnedAudioDataBuffers.dequeue());
        	}
        	else
        	{
            	spReflectedAudioData = m->queuedPointersToReturnedAudioDataBuffers.dequeue();
        	}
        	success = true;
    	}
	}



    return success;
}


#pragma region PRIVATE MEMBER FUNCTIONS

void AudioManager::InitPortAudio()
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
    }
}

bool AudioManager::ConfigureAudioDevices(const QString& inDeviceIndex, const QString& outDeviceIndex)
{
    LogAudioDeviceInformation(m->amountOfAudioDevices);
    bool success = false;
    QTextStream qin(stdin);

    qInfo() << "--> User input required: ";
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
        qInfo() << "\n";
        qInfo() << "Proceeding with initialization ...";
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
    qInfo() << "\n";
    qInfo() << "Proceeding with initialization ...";
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

void AudioManager::ConfigurePortaudioParameters()
{
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

void AudioManager::ConfigureOpusEncoding(const QString& encodingEnabled)
{
    if (!m->trueValues.contains(encodingEnabled))
    {
        m->opusEncoding = false;
        qInfo() << "Encoding disabled.";
        return;
    }
    m->opusEncoding = true;
    qInfo() << "Encoding enabled.";

    m->opusMode = opus_custom_mode_create(m->sampleRate, m->framesPerBuffer, &m->opusError);
    if (m->opusError != OPUS_OK) {
        qWarning() << "Audiomanager: Cannot create Opus Mode - Error: " << opus_strerror(m->opusError);
        qWarning() << "Disabling Encoding.";
        m->opusEncoding = false;
    }

    m->encoder = opus_custom_encoder_create(m->opusMode, m->audioChannels, &m->opusError);
    if (m->opusError != OPUS_OK) {
        qWarning() << "Audiomanager: Cannot create Opus Encoder - Error: " << opus_strerror(m->opusError);
        qWarning() << "Disabling Encoding.";
        m->opusEncoding = false;
    }

    m->decoder = opus_custom_decoder_create(m->opusMode, m->audioChannels, &m->opusError);
    if(m->opusError != OPUS_OK)
    {
        qWarning() << "Audiomanager: OpusCustomDecoder creation failed: " << opus_strerror(m->opusError);
        qWarning() << "Disabling Encoding.";
        m->opusEncoding = false;
    }
}

spAudioData_t AudioManager::EncodeWithOpus(const spAudioData_t &spInputAudioData) const
{
    const auto inputForOpus = reinterpret_cast<const opus_int16 *> (spInputAudioData->data());
    auto *encodedAudioData = new unsigned char[m->maxCompressedLength]();

    qDebug() << "Audiomanager: Encoding audio data with a maximum of " << m->maxCompressedLength << " bytes.";

    int length = opus_custom_encode(m->encoder, inputForOpus, m->framesPerBuffer * m->audioChannels,
                                    encodedAudioData, m->maxCompressedLength);
    if (length < 0)
    {
        qWarning() << "Audiomanager: Opus encountered an error while encoding input data.";
    }

    auto spEncodedInputData = std::make_shared<QByteArray>(reinterpret_cast<const char *> (encodedAudioData), length);

    opus_custom_encoder_ctl(m->encoder, OPUS_RESET_STATE);
    return spEncodedInputData;
}

spAudioData_t AudioManager::DecodeWithOpus(const spAudioData_t &spReflectedAudioData) const
{
    opus_custom_decoder_ctl(m->decoder, OPUS_SET_BITRATE(OPUS_BITRATE_MAX));
    const auto inputForOpus = reinterpret_cast<const unsigned char *> (spReflectedAudioData->data());

    // CHECK: not sure if length() is cast to int correctly
    const int lengthToDecode {static_cast<int>(spReflectedAudioData->length())};
    auto *decodedAudioData = new opus_int16[m->framesPerBuffer*m->audioChannels*sizeof(opus_int16)]();

    qDebug() << "Audiomanager: Decoding data of size " << lengthToDecode << " bytes.";
    int samples = opus_custom_decode(m->decoder, inputForOpus, lengthToDecode,
                                    decodedAudioData, m->framesPerBuffer);

    if (samples < 0)
    {
        qWarning() << "Audiomanager: Opus encountered an error while decoding data.";
    }

    // CHECK: Not sure if "samples" is the size needed here, should samples be equal to framesPerBuffer?
    auto spDecodedData = std::make_shared<QByteArray>(reinterpret_cast<const char *> (decodedAudioData), samples*2);

    opus_custom_decoder_ctl(m->decoder, OPUS_RESET_STATE);
    return spDecodedData;
}

void AudioManager::SendAudioInputToServer(const spAudioData_t &spInputAudioData) const
{
    //qDebug() << "Audiomanager: Emitting signal to send audio input to server.";
    Q_EMIT sigSendAudioInputToServer(spInputAudioData);
}

#pragma endregion

#pragma region SLOTS

void AudioManager::slotSendAudioInputToServer(const spAudioData_t &spInputAudioData) const
{
    m->client.SendAudioData(spInputAudioData);
    //m->callbackInterval = m->intervalTimer.restart();
    //qInfo() << "Current interval time: " << m->callbackInterval;
}

void AudioManager::slotReceivedAudioData(const spAudioData_t& spReflectedAudioData) const
{
    /// the locker is unlocked whenever the function ends or returns
    //QMutexLocker locker(&m->mtxLocker);
    m->queuedPointersToReturnedAudioDataBuffers.enqueue(spReflectedAudioData);
}

#pragma endregion





