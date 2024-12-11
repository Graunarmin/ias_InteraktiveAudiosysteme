#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QDebug>
#include "../../include/portaudio-snapshot/include/portaudio.h"
#include "callbackdata.h"

class AudioManager : public QObject
{
    Q_OBJECT
public:
    explicit AudioManager(QObject *parent = nullptr);

    bool Initialize(bool audiofault, bool soundIsRunning, int frameSize, int sampleRate, int audioChannels);
    void Run();

private:

    struct Impl;
    std::shared_ptr<Impl> m;

    /*!
     * \brief InitPortAudio
     * Initializes PortAudio - MUST be called before using PortAudio anywhere else.
     * \return true in successfull initialization, false otherwise.
     */
    bool InitPortAudio();
    void LogAudioDeviceInformation();
    bool ConfigureAudioDevice();
    void ConfigurePaStreamParameters(PaStreamParameters &parameters,
                                     int channelCount, PaDeviceIndex deviceIndex, PaSampleFormat sampleFormat);


signals:

};

#endif // AUDIOMANAGER_H
