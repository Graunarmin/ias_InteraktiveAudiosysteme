#ifndef SERVER_H
#define SERVER_H

#include "QObject"
#include "QUdpSocket"
#include <QDebug>

class Server : public QObject {
Q_OBJECT

public:
    // Constructor
    explicit Server(QObject *parent = nullptr);
    // Destructor
    ~Server();

    // Functions
    void initSocket();
    void SendData();
    void readPendingDatagrams();

signals:
    /*!
     * \brief intDataSignal
     * Send an int to a distant peer
     */
    void intDataSignal(int data);

public slots:
    /*!
     * \brief receivedReflectedData
     * Reaction to receiving data back from a distant server
     */
    //void receivedReflectedData();

private:
    QUdpSocket* _udpSocket;
    int _port;

    QString readInPort();
    int checkPortValidity(QString input);
};

#endif // SERVER_H
