#ifndef CLIENT_H
#define CLIENT_H

#include "QObject"
#include <QDebug>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QIODevice>
#include <QTimer>

#include <utility>
#include <memory>

#include "utils.h"

class Client : public QObject {
    Q_OBJECT

public:
    /// Delete the standard constructor and copy constructors
    Client() = delete;
    Client(Client const&) = delete;
    void operator= (Client const&) = delete;

    /// Constructor
    explicit Client(QObject *parent = nullptr);

    /// Destructor
    ~Client() override = default;

    /*!
     * \brief Initializes the Client with IP address and port number.
     * Creates a timer with an interval of a certain amount of milliseconds,
     * then connects the timeout-signal of the timer to the sendData function (slot).
     * This way, every intervall the sendData() function will be called.
     * \param ipIn
     * The IP-Address
     * \param portIn
     * The Port-Number
     * \return
     * True if IP and port are valid, false if not.
     */
    bool InitializeWithTimer(const QString &ipIn, const QString &portIn);

    /*! Initializes the Client with IP address and port number.
     * Subscribes to the server's readyRead() signal.
     * @param ipIn The IP address
     * @param portIn The port number
     * @return True if IP and port are valid, false if not.
     */
    bool InitializeForAudio(const QString &ipIn, const QString &portIn);

    /*!
     * \brief Start sending a number to the server every second.
     */
    void RunWithTimer() const;

    /*! Sends a QByteArray to the server via udp socket.
     *
     * @param spAudioData A shared pointer to the QByteArray full of
     * audio samples.
     */
    void SendAudioData(const spBaAudioData_t& spAudioData) const;

signals:
    /*! A signal that is emitted whenever the udp socket has received
     * data back from the server.
     * @param audioData A shared pointer to the QByteArray the server sent back.
     */
    void signalReceivedAudioData(spBaAudioData_t audioData);
    void sigReceivedAudioData(spListSpByteArray_t data);

private:
    /// struct for all member variables
    struct Impl;
    std::shared_ptr<Impl> m;

private slots:

    /*! Sends data to the server. */
    void slotSendTimerData() const;

    /*! Reads data from the server when there is data to read. */
    void slotReceivedReflectedTimerData() const;

    /*! Slot that creates a shared pointer to a QByteArray,
     * fills that array with the received data in the udp socket,
     * and then emits the 'ReceivedAudioData' signal, which carries the shared pointer.
     */
    void slotReceivedReflectedAudioData();
};

#endif // CLIENT_H
