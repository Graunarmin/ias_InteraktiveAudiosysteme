#ifndef SERVER_H
#define SERVER_H

#include "QObject"
#include "QUdpSocket"
#include <QDebug>
#include <QTimer>
#include <QNetworkDatagram>
#include <memory>

class MyClient : public QObject {
    Q_OBJECT

public:
    MyClient() = delete;
    MyClient(MyClient const&) = delete;
    void operator= (MyClient const&) = delete;

    // Constructor
    explicit MyClient(QObject *parent = nullptr);

    // Destructor
    ~MyClient(){};

    bool Initialize(QString ipIn, QString portIn);
    void Run();

signals:

public slots:

    void slotSendData();

    /*!
     * \brief receivedReflectedData
     * Reaction to receiving data back from a distant server
     */
    void slotReceivedReflectedData();

private:
    // struct for all member variables
    struct Impl;
    std::shared_ptr<Impl> m;

    // Functions
    void readInPort(QString& ipIn, QString& portIn);
    bool verifyParameters(QString ipIn, QString portIn,  QHostAddress& ipOut, quint16& portOut);
    void readPendingDatagrams();
};

#endif // SERVER_H
