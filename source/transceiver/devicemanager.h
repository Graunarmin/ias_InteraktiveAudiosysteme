#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QDebug>
#include "portaudio.h"
#include "pa_mac_core.h"

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager() override;

    void LogAudioDeviceInformation() const;

    void Initialize(int deviceAmount, const QString &inDeviceIndex, const QString &outDeviceIndex);

    int InputIndex() const;
    int OutputIndex() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m;

    void SetDeviceIndices(const QString &inDeviceIndex, const QString &outDeviceIndex);

    bool VerifyDeviceIndex(const QString &indexString, int& indexOut);
};

#endif // DEVICEMANAGER_H
