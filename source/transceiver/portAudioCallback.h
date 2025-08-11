#ifndef PORTAUDIOCALLBACK_H
#define PORTAUDIOCALLBACK_H

#include "portaudio.h"
#include "pa_mac_core.h"
#include <QObject>
#include <QDebug>
#include "audiomanager.h"

class AudioManager;

int PortAudioCallback(const void *inputBuffer,
                      void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData);

struct CallbackData
{
    AudioManager* pAudioManager;
};

#endif //PORTAUDIOCALLBACK_H

