#include "observer.h"

// --- CONSTRUCTOR ---
Observer::Observer(QObject *parent)
    : QObject{parent}
{}


// --- FUNCTIONS ---
/*!
 * \brief Observer::subscribeToEvent
 *
 * Connect the signal from the reporter to a slot from the observer:
 * connect(sender, signal, receiver, slot)
 * \param reporter - the reporter we want to observe
 */
void Observer::subscribeToEvent(Reporter *reporter)
{
    QObject::connect(reporter, &Reporter::report, this, &Observer::onSignalObserved);
}


// --- SLOTS ---
/*!
 * \brief Observer::onSignalObserved
 *
 * Reaction to receiving a signal on this slot
 */
void Observer::onSignalObserved()
{
    qInfo() << "I received a callback!";
    emit onObserve();
}
