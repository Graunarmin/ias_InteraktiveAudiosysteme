#include "utils.h"

void ConfigurePaStreamParameters(const std::shared_ptr<PaStreamParameters> &parameters,
                                 const int channelCount,
                                 const PaDeviceIndex &deviceIndex,
                                 const PaSampleFormat sampleFormat,
                                 PaMacCoreStreamInfo& coreAudioInfo,
                                 const PaTime suggestedLatency)
    {
        parameters->channelCount = channelCount;
        parameters->device = deviceIndex;
        parameters->sampleFormat = sampleFormat;
        parameters->hostApiSpecificStreamInfo = nullptr;
        parameters->suggestedLatency = suggestedLatency;

        PaMacCore_SetupStreamInfo(&coreAudioInfo, paMacCorePro);
        parameters->hostApiSpecificStreamInfo = &coreAudioInfo;
    }

void LogAudioDeviceInformation(const int amountOfAudioDevices)
{
    qInfo() << "\n";
    qInfo() << "---- LIST OF AUDIO DEVICES ----";
    for(int i = 0; i <= (amountOfAudioDevices - 1); i++)
    {
        const auto deviceInfo = Pa_GetDeviceInfo(i);

        qInfo() << "Device " << i;
        qInfo() << "Name: " << deviceInfo->name;
        qInfo() << "Max. output channels: " << deviceInfo->maxOutputChannels;
        qInfo() << "Max. input channels: " << deviceInfo->maxInputChannels;
        qInfo() << "Default sample rate: " << deviceInfo->defaultSampleRate;
        qInfo() << "\n";
    }
}

bool VerifyDeviceIndex(const QString &indexString, const int amountOfAudioDevices, int& indexOut)
{
    bool inputWasNumber = false;
    const int index = indexString.toInt(&inputWasNumber);
    if (inputWasNumber) {
        if (index >= 0 && index < amountOfAudioDevices)
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



void ReadInPort(QString& ipIn, QString& portIn)
{
    QTextStream qin(stdin);
    qInfo() << "Please enter the Port Number: ";
    portIn = qin.readLine();

    qInfo() << "And now the IP-Address: ";
    ipIn = qin.readLine();
}

bool VerifyIpAndPort(const QString& ipIn, const QString &portIn, QHostAddress& ipOut, quint16& portOut)
{
    bool success = true;
    bool inputWasNumber = false;
    const int port = portIn.toInt(&inputWasNumber);

    if(inputWasNumber){
        if(port > 0 && port <= 0xffff)
        {
            portOut = static_cast<quint16>(port);
        }
        else
        {
            qWarning() << "Invalid port. Terminating.";
            success = false;
        }
    }
    else
    {
        qWarning() << "Invalid port. Terminating.";
        success = false;
    }

    if(const auto ip = QHostAddress(ipIn); ip.isNull())
    {
        qWarning() << "Invalid IP-Address. Terminating";
        success = false;
    }
    else
    {
        ipOut = ip;
    }

    return success;
}