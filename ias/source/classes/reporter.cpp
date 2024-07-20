#include "reporter.h"

Reporter::Reporter(QObject *parent)
    : QObject{parent}
{}

void Reporter::sendSignal()
{
    emit report();
}
