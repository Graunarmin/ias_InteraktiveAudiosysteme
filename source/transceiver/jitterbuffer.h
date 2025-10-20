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

    bool Initialize(const QString& bufferSize);
    void QueryBufferSize();

    void Add(spAudioData_t spAudioData);

    spAudioData_t Pop();
    spAudioData_t Peek();

    bool IsEmpty();
    bool IsFull();

private:
    struct Impl;
    std::unique_ptr<Impl> m;

    bool SetBuffer(const QString& newSize);

};

#endif // JITTERBUFFER_H
