// ReSharper disable CppMemberFunctionMayBeConst
#include "codec.h"

#include <iostream>

using namespace std;

struct Codec::Impl
{
    int opusError{};

    int compressionFactor{8};
    int maxCompressedLength{128};
    int framesPerBuffer{512};
    int audioChannels{1};

    OpusCustomMode *opusMode{};
    OpusCustomEncoder *encoder{};
    OpusCustomDecoder *decoder{};

    Impl() = default;
};

Codec::Codec(QObject *parent)
    : QObject{parent}
    , m(std::make_unique<Impl>())
{ }

Codec::~Codec()
{
    opus_custom_encoder_destroy(m->encoder);
    opus_custom_decoder_destroy(m->decoder);
    opus_custom_mode_destroy(m->opusMode);
}

bool Codec::Initialize(const QString &compressionFactor, const int sampleRate, const int framesPerBuffer, const int channels)
{

    SetCompression(compressionFactor);
    m->framesPerBuffer = framesPerBuffer;
    m->audioChannels = channels;
    m->maxCompressedLength = m->framesPerBuffer*2 / m->compressionFactor;

    cout << "SR: " << sampleRate << " FPB: " << m->framesPerBuffer << " CH: " << m->audioChannels << endl;

    m->opusMode = opus_custom_mode_create(sampleRate, m->framesPerBuffer, &m->opusError);
    if (m->opusError != OPUS_OK)
    {
        qWarning() << "Codec: Cannot create Opus Mode - ERROR: " << opus_strerror(m->opusError);
        qWarning() << " --> Disabling Encoding.";
        return false;
    }

    m->encoder = opus_custom_encoder_create(m->opusMode, m->audioChannels, &m->opusError);
    if (m->opusError != OPUS_OK) {
        qWarning() << "Codec: Cannot create Opus Encoder - ERROR: " << opus_strerror(m->opusError);
        qWarning() << " --> Disabling Encoding.";
        return false;
    }

    m->decoder = opus_custom_decoder_create(m->opusMode, m->audioChannels, &m->opusError);
    if(m->opusError != OPUS_OK)
    {
        qWarning() << "Codec: OpusCustomDecoder creation failed: " << opus_strerror(m->opusError);
        qWarning() << " --> Disabling Encoding.";
        return false;
    }
    return true;
}

spByteArray_t Codec::Encode(const spByteArray_t &spInputData) const
{
    const auto input = reinterpret_cast<const opus_int16 *>(spInputData->data());
    auto *encodedAudioData = new unsigned char [m->maxCompressedLength]();

    //int length = opus_custom_encode(m->encoder, input, m->framesPerBuffer * m->audioChannels, encodedAudioData, m->maxCompressedLength);
    //int length = opus_custom_encode(m->encoder, input, m->framesPerBuffer * m->audioChannels, encodedAudioData, m->maxCompressedLength);
 
    int length = opus_custom_encode(m->encoder, (short *) spInputData->data(), m->framesPerBuffer * m->audioChannels, encodedAudioData, m->maxCompressedLength);

    cout << "FPB: " << m->framesPerBuffer << " CH: " << m->audioChannels << " VALUE: " << m->maxCompressedLength << " LENGTH: " << length << endl;

    if (length < 0)
    {
        qWarning() << "Codec: Opus encountered an ERROR while encoding input data.";
    }

    auto spEncodedInputData = std::make_shared<QByteArray>(reinterpret_cast<const char *> (encodedAudioData), length);
    opus_custom_encoder_ctl(m->encoder, OPUS_RESET_STATE);
    return spEncodedInputData;
}

spByteArray_t Codec::Decode(const spByteArray_t &spReflectedData) const
{
    opus_custom_decoder_ctl(m->decoder, OPUS_SET_BITRATE(OPUS_BITRATE_MAX));
    const auto input = reinterpret_cast<const unsigned char *>(spReflectedData->data());

    //const int lengthToDecode{m->maxCompressedLength};
    const int lengthToDecode{static_cast<int>(spReflectedData->size())};

    auto *decodedAudioData = new opus_int16[m->framesPerBuffer * m->audioChannels * sizeof(opus_int16)];

    int samples = opus_custom_decode(m->decoder, input, lengthToDecode, decodedAudioData, m->framesPerBuffer);
  
      if (samples < 0)
    {
        qWarning() << "Codec: Opus encountered an ERROR while decoding data.";
    }

    auto spDecodedData = std::make_shared<QByteArray>(reinterpret_cast<const char *> (decodedAudioData), samples * 2 );
    opus_custom_decoder_ctl(m->decoder, OPUS_RESET_STATE);
    return spDecodedData;

}

void Codec::SetCompression(const QString& factor)
{
    bool success{false};
    int tmp = factor.toInt(&success);
    if (!success || tmp < 1)
    {
        qWarning() << "Compression factor must be an integer greater than 0. Setting factor to default:" << m->compressionFactor;
    }
    else
    {
        m->compressionFactor = tmp;
    }
}
