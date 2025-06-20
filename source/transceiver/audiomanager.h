#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QDebug>
#include <QTime>
#include "../../include/portaudio-snapshot/include/portaudio.h"
#include "../../include/portaudio-snapshot/include/pa_mac_core.h"
#include "callbackdata.h"

class AudioManager : public QObject
{
    Q_OBJECT
public:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager();

    bool Initialize(int frameSize = 512, int sampleRate = 48000, int audioChannels = 1);
    void Run();

private:

    struct Impl;
    std::shared_ptr<Impl> m;

    /*!
     * \brief InitPortAudio
     * Initializes PortAudio - MUST be called before using PortAudio anywhere else.
     * \return true if successfull initialization, false otherwise.
     */
    void InitPortAudio();
    void ConfigureAudioDevice();


signals:

};

#endif // AUDIOMANAGER_H
