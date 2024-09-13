#ifndef CLIENT_H
#define CLIENT_H

#include "QObject"
#include "QUdpSocket"
#include <QDebug>
#include <QTimer>
#include <QNetworkDatagram>
#include <memory>

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
    ~Client(){};

    /*!
     * \brief Initialize
     * Initialize the Client with IP-Address and Port-Number
     * \param ipIn
     * The IP-Address
     * \param portIn
     * The Port-Number
     * \return
     * True if IP and Port are Valid, False if not
     */
    bool Initialize(QString ipIn, QString portIn);

    /*!
     * \brief Run
     * Start sending data to the server every second
     */
    void Run();

signals:

public slots:

    /*!
     * \brief slotSendData
     * Sends data to the server
     */
    void slotSendData();

    /*!
     * \brief receivedReflectedData
     * Reads data from the server when there is data to read
     */
    void slotReceivedReflectedData();

private:
    /// struct for all member variables
    struct Impl;
    std::shared_ptr<Impl> m;

    /*!
     * \brief readInPort
     * Helper Function for asking the user for the IP-Address and Port-Number
     * \param ipIn
     * Reference to the string in which to store the IP-Address
     * \param portIn
     * Reference to the string in which to store the Port number
     */
    void readInPort(QString& ipIn, QString& portIn);

    /*!
     * \brief verifyParameters
     * Verifies whether the given IP and Port are valid
     * \param ipIn
     * The IP to verify
     * \param portIn
     * The port to verify
     * \param ipOut
     * reference to the variable in which to store the valid IP
     * \param portOut
     * reference to the variable in which to store the valid port
     * \return
     * True if both parameters are valid, False if at least one parameter is not.
     */
    bool verifyParameters(QString ipIn, QString portIn,  QHostAddress& ipOut, quint16& portOut);
};

#endif // CLIENT_H
