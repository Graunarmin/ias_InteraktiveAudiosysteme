#ifndef REPORTER_H
#define REPORTER_H

#include <QObject>

class Reporter : public QObject
{
    Q_OBJECT
public:
    explicit Reporter(QObject *parent = nullptr);
    void sendSignal();

signals:
    void report();
};

#endif // REPORTER_H
