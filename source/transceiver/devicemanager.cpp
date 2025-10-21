#include "devicemanager.h"

struct DeviceManager::Impl
{
    int amountOfAudioDevices;
    int inputDeviceIndex{0};
    int outputDeviceIndex{1};

    Impl() = default;
};

DeviceManager::DeviceManager(QObject *parent)
    : QObject{parent}
    , m(std::make_unique<Impl>())
{}

DeviceManager::~DeviceManager() = default;

int DeviceManager::InputIndex() const
{
    return m->inputDeviceIndex;
}

int DeviceManager::OutputIndex() const
{
    return m->outputDeviceIndex;
}


void DeviceManager::Initialize(const int deviceAmount, const QString &inDeviceIndex, const QString &outDeviceIndex)
{
    m->amountOfAudioDevices = deviceAmount;
    LogAudioDeviceInformation();
    SetDeviceIndices(inDeviceIndex, outDeviceIndex);

    qInfo() << "\n";
    qInfo() << "+++ User input required +++";
    qInfo() << "  Current Settings:";
    qInfo() << "    Input device index -" << m->inputDeviceIndex << " |   Output device index -" << m->outputDeviceIndex;
    qInfo() << "> Do you wish to change these settings? [y/n]";

    QTextStream qin(stdin);
    QString confirmation = qin.readLine();

    if(confirmation != "y") return;

    bool success = false;
    while(!success)
    {
        qInfo() << "> Please check the device list above and enter the index of the input device you wish to use: ";
        QString inputIndex = qin.readLine();
        success = VerifyDeviceIndex(inputIndex, m->inputDeviceIndex);
    }
    success = false;
    while(!success)
    {
        qInfo() << "> And the index or the output device: ";
        QString outputIndex = qin.readLine();
        success = VerifyDeviceIndex(outputIndex, m->outputDeviceIndex);
    }
    qDebug() << "--> New Settings: Input device index -" << m->inputDeviceIndex << "    |   Output device index -" << m->outputDeviceIndex;
    qInfo() << "\n";
}

void DeviceManager::LogAudioDeviceInformation() const
{
    qInfo() << "\n";
    qInfo() << "++++++++ LIST OF AUDIO DEVICES ++++++++";
    for(int i = 0; i <= (m->amountOfAudioDevices - 1); i++)
    {
        const auto deviceInfo = Pa_GetDeviceInfo(i);

        qInfo() << "  Device index " << i;
        qInfo() << "    Name                " << deviceInfo->name;
        qInfo() << "    Max. channels in    " << deviceInfo->maxOutputChannels;
        qInfo() << "    Max. channels out   " << deviceInfo->maxInputChannels;
        qInfo() << "    Default sample rate " << deviceInfo->defaultSampleRate;
        qInfo() << "----------------------------------------------------";
    }
}

void DeviceManager::SetDeviceIndices(const QString &inDeviceIndex, const QString &outDeviceIndex)
{
    bool success = false;
    int index = inDeviceIndex.toInt(&success);

    if (!success)
    {
        qWarning() << " --- WARNING --- The index for the input device is not a valid number. Input device index remains default.";
    }
    else
    {
        m->inputDeviceIndex = index;
    }

    index = outDeviceIndex.toInt(&success);
    if (!success)
    {
        qWarning() << " --- WARNING --- The index for the output device is not a valid number. Output device index remains default.";
    }
    else
    {
        m->outputDeviceIndex = index;
    }
}

bool DeviceManager::VerifyDeviceIndex(const QString &indexString, int& indexOut)
{
    bool inputWasNumber = false;
    const int index = indexString.toInt(&inputWasNumber);
    if (inputWasNumber) {
        if (index >= 0 && index < m->amountOfAudioDevices)
        {
            indexOut = index;
            return true;
        }
        indexOut = -1;
        qWarning() << "Audio device index out of range.";
        return false;
    }
    indexOut = -1;
    qWarning() << "Audio device index is not a valid number";
    return false;
}
