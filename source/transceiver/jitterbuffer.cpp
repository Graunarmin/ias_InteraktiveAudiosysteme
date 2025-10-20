#include "jitterbuffer.h"

struct JitterBuffer::Impl
{
    QQueue<spAudioData_t> queuedPointersToData{};
    signed int bufferSize{50};

    Impl() = default;
};

JitterBuffer::JitterBuffer(QObject *parent)
    : QObject{parent}
    , m(std::make_unique<Impl>())
{}

JitterBuffer::~JitterBuffer()
{
    m->queuedPointersToData.clear();
}

bool JitterBuffer::Initialize(const QString& bufferSize)
{
    return SetBuffer(bufferSize);
}

void JitterBuffer::QueryBufferSize()
{
    qInfo() << "--> User input requiered: ";
    qInfo() << "The jitter buffer size (number of buffered packages) is currently set to " << m->bufferSize << ".";
    qInfo() << "Do you wish to change these settings? [y/n]";
    QTextStream qin(stdin);

    QString confirmation = qin.readLine();
    if(confirmation == "y")
    {
        qInfo() << "Please enter the new size for the jitterbuffer: ";
        QString newBufferSize = qin.readLine();
        SetBuffer(newBufferSize);
    }
}

void JitterBuffer::Add(spAudioData_t spAudioData)
{
    m->queuedPointersToData.enqueue(spAudioData);
}

spAudioData_t JitterBuffer::Peek()
{
    return m->queuedPointersToData.head();
}

spAudioData_t JitterBuffer::Pop()
{
    return m->queuedPointersToData.dequeue();
}

bool JitterBuffer::IsEmpty()
{
    return m->queuedPointersToData.isEmpty();
}

bool JitterBuffer::IsFull()
{
    return m->queuedPointersToData.size() >= m->bufferSize;
}

bool JitterBuffer::SetBuffer(const QString &newSize)
{
    bool success{false};
    m->bufferSize = newSize.toInt(&success);
    if (!success || m->bufferSize < 1)
    {
        qWarning() << "WARNING: Size given for jitterbuffer was invalid. Setting size to 1.";
        m->bufferSize = 1;
        success = true;
    }
    qInfo() << "Programm will buffer" << m->bufferSize << "packages before starting audio output.";
    return success;
}
