#ifndef SERVER_H
#define SERVER_H

#include "QObject"
#include "QUdpSocket"

class Server : public QObject
{
Q_OBJECT
public:
    // Variables
    QUdpSocket* udpSocket;

    // Default Constructor
    Server();
    // Custom Constructor
    explicit Server(QObject *parent = nullptr);
    // Destructor
    ~Server();

    // Functions
    void initSocket();
    void readPendingDatagrams();

signals:

};

#endif // SERVER_H
