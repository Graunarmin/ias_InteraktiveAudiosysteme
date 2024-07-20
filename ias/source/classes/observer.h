#ifndef OBSERVER_H
#define OBSERVER_H

#include <QObject>
#include <QDebug>

#include "reporter.h"

class Observer : public QObject
{
    Q_OBJECT
public:
    explicit Observer(QObject *parent = nullptr);

    void subscribeToEvent(Reporter *reporter);

signals:
    void onObserve();

public slots:
    void onSignalObserved();
};

#endif // OBSERVER_H
