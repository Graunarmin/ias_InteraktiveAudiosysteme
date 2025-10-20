#ifndef UTILS_H
#define UTILS_H

#include <QTime>
#include <QUdpSocket>
#include <QDebug>
#include <thread>
#include "portaudio.h"
#include "pa_mac_core.h"


/// ---- typedef ----
using spBaAudioData_t = std::shared_ptr<QByteArray>;
using spListSpByteArray_t = std::shared_ptr<QList<spBaAudioData_t>>;

/// ---- utility functions ----
void ConfigurePaStreamParameters(const std::shared_ptr<PaStreamParameters> &parameters,
                                 int channelCount, const PaDeviceIndex &deviceIndex, PaSampleFormat sampleFormat,
                                 PaMacCoreStreamInfo& coreAudioInfo, PaTime suggestedLatency);

void LogAudioDeviceInformation(int amountOfAudioDevices);

bool VerifyDeviceIndex(const QString &indexString, int amountOfAudioDevices, int& indexOut);

/*!
 * \brief asks the user for the IP address and port number.
 * \param ipIn Reference to the string in which to store the IP-Address
 * \param portIn Reference to the string in which to store the Port number
 */
void ReadInPort(QString& ipIn, QString& portIn);

/*! Verifies whether the given IP and Port are valid
 * \param ipIn The IP to verify
 * \param portIn The port to verify
 * \param ipOut Reference to the variable in which to store the valid IP
 * \param portOut Reference to the variable in which to store the valid port
 * \return True if both parameters are valid, false if at least one parameter is not.
 */
bool VerifyIpAndPort(const QString& ipIn, const QString &portIn, QHostAddress& ipOut, quint16& portOut);



#endif //UTILS_H
