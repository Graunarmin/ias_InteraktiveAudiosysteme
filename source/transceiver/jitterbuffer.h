#ifndef JITTERBUFFER_H
#define JITTERBUFFER_H

#include <QObject>
#include <QDebug>
#include <QQueue>
#include <QIODevice>

#include "utils.h"

class JitterBuffer : public QObject
{
    Q_OBJECT
public:
    explicit JitterBuffer(QObject *parent = nullptr);
    ~JitterBuffer() override;

    void Initialize(const QString& bufferSize);
    void QueryBufferSize();

    void Add(const spBaAudioData_t& spAudioData);
    void Add(const spListSpByteArray_t& dataList);

    bool GetNextSample(spBaAudioData_t &spBufferedAudioSample);

private:
    struct Impl;
    std::unique_ptr<Impl> m;

    bool SetBufferSize(const QString& newSize);

};

#endif // JITTERBUFFER_H
