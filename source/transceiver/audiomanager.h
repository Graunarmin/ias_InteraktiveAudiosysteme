#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QDebug>
#include <QQueue>
#include <QIODevice>
#include <QElapsedTimer>
#include <QMutexLocker>
#include "client.h"
#include "portAudioCallback.h"
#include "utils.h"
#include "opus.h"
#include "opus_custom.h"
#include "opus_types.h"

class AudioManager : public QObject
{
    Q_OBJECT
public:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager() override;

    bool Initialize(
        const QString& framesPerBuffer,
        const QString& sampleRate,
        const QString& audioChannels,
        const QString& inputDeviceIndex,
        const QString& outputDeviceIndex,
        const QString& ipIn,
        const QString& portIn);

    /*! Tries to open and start the portaudio stream. */
    void StartAudioStream() const;

    /*! Emits a signal to de-couple the next action from the callback function's context.
     * This makes the sending of data thread-safe.
     * @remark Thread-safe
     * @param spInputAudioData A shared pointer to the QByteArray with the data
     * that is supposed to go to the server.
     */
    void SendAudioInputToServer(const spAudioData_t &spInputAudioData) const;

    /*! Checks if the queue of returned audio data has any contents and
     * if so, it dequeues the first entry into 'receivedData'.
     * @remark Thread-safe
     * @param spReflectedAudioData A shared pointer to a QByteArray.
     * @return True if there was any data in the queue, false otherwise.
     */
    bool GetReceivedAudioData(spAudioData_t &spReflectedAudioData) const;
    spAudioData_t EncodeWithOpus(const spAudioData_t &spInputAudioData)const;
    bool DecodeWithOpus(spAudioData_t &spReflectedAudioData);


private:

    struct Impl;
    std::unique_ptr<Impl> m;

    /*! Checks if the parameters given are valid.
     * @param framesPerBuffer Number of frames that are collected until the callback function is called again.
     * @param sampleRate Number of times audio is written to the buffer per second
     * @param audioChannels Number of audio channels port audio is supposed to use for in- and output.
     * @return
     */
    bool ConfigureAudioParameters(const QString& framesPerBuffer, const QString& sampleRate, const QString& audioChannels);

    /*! Logs all available audio devices and asks the user for
     * the input and output device's indices.
     */
    bool ConfigureAudioDevices(const QString& inDeviceIndex, const QString& outDeviceIndex);

    /*! Initializes PortAudio - MUST be called before using PortAudio anywhere else.
     * \return true if successfully initialized, false otherwise.
     */
    void InitPortAudio();

    /*! Configures the in- and output streaming parameters for portaudio. */
    void ConfigurePortaudioParameters();

    void ConfigureOpus();


signals:
    /*! Signal to de-couple the contexts of the callback function and the AudioManager.
     * @param inputData A shared Pointer to the QByteArray with the data that is
     * supposed to be sent.
     */
    void sigSendAudioInputToServer(spAudioData_t inputData) const;

private slots:
    /*! Slot that calls the Client's 'SendAudioData()' function.
     * It also measures the time since the last time this slot was pinged.
     * @param spInputAudioData A shared pointer to the QByteArray with the data that is
     * supposed to be sent.
     */
    void slotSendAudioInputToServer(const spAudioData_t &spInputAudioData);

    /*! Slot that enqueues the received pointer to a QByteArray.
     * @param spReflectedAudioData A shared pointer to the QByteArray that was returned from the server.
     * @remark Thread-safe
     */
    void slotReceivedAudioData(const spAudioData_t& spReflectedAudioData);
};

#endif // AUDIOMANAGER_H
