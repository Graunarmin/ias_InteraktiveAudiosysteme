#ifndef CODEC_H
#define CODEC_H

#include "QObject"
#include <QDebug>
#include "opus.h"
#include "opus_custom.h"
#include "opus_types.h"
#include "utils.h"

class Codec : public QObject
{
    Q_OBJECT
public:
    explicit Codec(QObject *parent = nullptr);
    ~Codec() override;

    bool Initialize(const QString &compressionFactor, int sampleRate, int framesPerBuffer, int channels);
    spByteArray_t Encode(const spByteArray_t &spInputData) const;
    spByteArray_t Decode(const spByteArray_t &spReflectedData) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m;

    void SetCompression(const QString& factor);
};

#endif // CODEC_H
