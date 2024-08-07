#ifndef SERVER_H
#define SERVER_H

#include "QObject"
#include "QUdpSocket"
#include <QDebug>
#include <QTimer>

class Server : public QObject {
Q_OBJECT

public:
    // Constructor
    explicit Server(QObject *parent = nullptr);
    // Destructor
    ~Server();

    // Functions
    void initSocket();
    void initTimer(int intervall);
    void readPendingDatagrams();

signals:

public slots:
    /*!
     * \brief receivedReflectedData
     * Reaction to receiving data back from a distant server
     */
    //void receivedReflectedData();
    void sendData();

private:
    QUdpSocket* _udpSocket;
    int _port;
    QTimer* _timer;
    int _data;

    QString readInPort();
    int checkPortValidity(QString input);
    void startDataTransmission();
};

#endif // SERVER_H
