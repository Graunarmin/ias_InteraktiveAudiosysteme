#include "client.h"

/*
 * Wir wollen via UDP Daten an einen Client (Reflektor) schicken, der unter der gegebenen IP-Adresse
 * und dem Port erreichbar ist.
 * Verschickt werden sollen im Sekundentakt die Zahlen von 1 bis 10 in einer Endlosschleife.
 * Das Rückempfangen jedes Wertes soll mit einer Konsolenausgabe „Wert x empfangen“ quittiert werden.
 */

struct Client::Impl
{
    std::unique_ptr<QUdpSocket> upUdpSocket {std::make_unique<QUdpSocket>()};
    QHostAddress ip {QHostAddress()};
    quint16 port {0};

    /// --- Timer and send number ---
    std::unique_ptr<QTimer> upTimer {std::make_unique<QTimer>()};
    int timesRun {0};
    int data {0};

    Impl() = default;
};

Client::Client(QObject *parent)
    : QObject{parent}
    , m(std::make_shared<Impl>())
{ }

bool Client::InitializeWithTimer(const QString &ipIn, const QString &portIn)
{
    bool success = false;

    if(VerifyIpAndPort(ipIn, portIn, m->ip, m->port))
    {
        m->upTimer -> setInterval(1000);
        m->upUdpSocket->bind(QHostAddress::Any, m->port);

        /// connect timer to sending data
        connect(m->upTimer.get(), &QTimer::timeout, this, &Client::slotSendTimerData);

        /// connect socket-receive to confirmation function
        connect(m->upUdpSocket.get(), &QUdpSocket::readyRead, this, &Client::slotReceivedReflectedTimerData);
        success = true;
    }
    return success;
}

void Client::RunWithTimer() const {
    m->upTimer -> start();
    qInfo() << "Trying to send Data to port " << m->port << " at " << m->ip;
}

bool Client::InitializeForAudio(const QString &ipIn, const QString &portIn)
{
    bool success = false;
    if(VerifyIpAndPort(ipIn, portIn, m->ip, m->port))
    {
        m->upUdpSocket->bind(QHostAddress::Any, m->port);
        connect(m->upUdpSocket.get(), &QUdpSocket::readyRead, this, &Client::slotReceivedReflectedAudioData);
        success = true;
    }
    return success;
}

void Client::slotSendTimerData() const
{
    if(m->data < 10)
    {
        m->data += 1;
    }
    else
    {
        m->data = 1;
    }

    /// Reinterpret: cast pointer to int as pointer to bytes (char)
    const QByteArray baData = QByteArray::fromRawData(reinterpret_cast<const char *>(&(m->data)), sizeof(m->data));
    qDebug() << "Outgoing Byte Array: " << baData;
    const qint64 sentBytes = m->upUdpSocket -> writeDatagram(baData, m->ip, m->port);

    if(const int checkNumber = static_cast<int>(sentBytes); checkNumber < 0)
    {
        qInfo() << "There was an Error while sending Data to the Server.";
    }
    else
    {
        qInfo() << "Successfully sent " << checkNumber << "Bytes of data with value " << m->data << " to Server.";
    }
}

void Client::slotReceivedReflectedTimerData() const
{
    while (m->upUdpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m->upUdpSocket->receiveDatagram();
        QByteArray baData = datagram.data();
        const auto pointer = baData.data();
        const char number = *pointer;

        qDebug() << "Incoming Byte Array: " << baData;
        qInfo() << "Received Data of size " << baData.size() << " with value " << static_cast<int>(number);
    }
}

void Client::SendAudioData(const spByteArray_t& spAudioData) const
{
    const qint64 sentBytes = m->upUdpSocket -> writeDatagram(*spAudioData, m->ip, m->port);
    if(const int checkNumber = static_cast<int>(sentBytes); checkNumber < 0)
    {
        qInfo() << "Client: There was an Error while sending Data to the Server.";
    }
    else
    {
        qInfo() << "Client: Successfully sent" << checkNumber << "Bytes of data to Server.";
    }
}

void Client::slotReceivedReflectedAudioData()
{
    const auto spListByteArrays = std::make_shared<QList<spByteArray_t>>();

    while (m->upUdpSocket->hasPendingDatagrams())
    {
        const auto spByteArray = std::make_shared<QByteArray>();
        QNetworkDatagram datagram = m->upUdpSocket->receiveDatagram();
        spByteArray->append(datagram.data());
        spListByteArrays -> append(spByteArray);

        qInfo() << "Client: Received Datagram of" << datagram.data().size() << "Bytes from Server.";
    }
    qInfo() << "Client: Received" << spListByteArrays->length() << "Datagrams total";
    Q_EMIT sigReceivedAudioData(spListByteArrays);
}

void Client::ReadInPort(QString& ipIn, QString& portIn)
{
    QTextStream qin(stdin);
    qInfo() << "Please enter the Port Number: ";
    portIn = qin.readLine();

    qInfo() << "And now the IP-Address: ";
    ipIn = qin.readLine();
}

bool Client::VerifyIpAndPort(const QString& ipIn, const QString &portIn, QHostAddress& ipOut, quint16& portOut)
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
            qWarning() << "Invalid port:" << portIn;
            success = false;
        }
    }
    else
    {
        qWarning() << "Invalid port:" << portIn;
        success = false;
    }

    if(const auto ip = QHostAddress(ipIn); ip.isNull())
    {
        qWarning() << "Invalid IP-Address:" << ipIn;
        success = false;
    }
    else
    {
        ipOut = ip;
    }

    return success;
}








