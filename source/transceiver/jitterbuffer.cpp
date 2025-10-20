#include "jitterbuffer.h"

struct JitterBuffer::Impl
{
    QQueue<spBaAudioData_t> queuedPointersToData{};
    signed int bufferSize{50};
    bool buffering{true};
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

void JitterBuffer::Initialize(const QString& bufferSize)
{
    SetBuffer(bufferSize);
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
        const QString newBufferSize = qin.readLine();
        SetBuffer(newBufferSize);
    }
}

void JitterBuffer::Add(const spBaAudioData_t& spAudioData)
{
    qDebug() << "Jitterbuffer: Adding" << spAudioData->size() << "bytes to Queue";
    m->queuedPointersToData.enqueue(spAudioData);
}

void JitterBuffer::Add(const spListSpByteArray_t& dataList)
{
    for(const auto& data : *dataList)
    {
        Add(data);
    }
}

spBaAudioData_t JitterBuffer::Peek()
{
    return m->queuedPointersToData.head();
}

spBaAudioData_t JitterBuffer::Pop()
{
    return m->queuedPointersToData.dequeue();
}

bool JitterBuffer::GetNextSample(spBaAudioData_t &spBufferedAudioSample)
{
    bool success = false;
    if(m->buffering)
    {
        if(IsFull())
        {
            qDebug() << "Jitter buffer full. Starting audio output ...";
            m->buffering = false;
            spBufferedAudioSample = m->queuedPointersToData.dequeue();
            success = true;
        }
        else
        {
            qDebug() << "Jitter buffer still waiting for more audio data. Current Buffer: "
                     << m->queuedPointersToData.size();
        }
    }
    else
    {
        if(IsEmpty())
        {
            qDebug() << "Jitter buffer ran empty. Buffering ...";
            m->buffering = true;
        }
        else
        {
            spBufferedAudioSample = m->queuedPointersToData.dequeue();
            success = true;
        }
    }
    return success;
}

bool JitterBuffer::IsEmpty()
{
    return m->queuedPointersToData.isEmpty();
}

bool JitterBuffer::IsFull()
{
    return m->queuedPointersToData.size() >= m->bufferSize;
}

int JitterBuffer::Size(){
    return (int) m->queuedPointersToData.size();
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
