#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QDebug>
#include <QIODevice>
#include <QElapsedTimer>
#include <QMutexLocker>
#include "client.h"
#include "portAudioCallback.h"
#include "utils.h"
#include "jitterbuffer.h"
#include "codec.h"
#include "devicemanager.h"

class AudioManager : public QObject
{
    Q_OBJECT
public:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager() override;

    bool Initialize(
        const QString& ipIn,
        const QString& portIn,
        const QString& inputDeviceIndex,
        const QString& outputDeviceIndex,
        const QString& framesPerBuffer,
        const QString& sampleRate,
        const QString& audioChannels,
        const QString& jitterBufferSize,
        const QString& encodingEnabled,
        const QString& compressionFactor);

    /*! Tries to open and start the portaudio stream. */
    void StartAudioStream() const;

    void ProcessAudioInput(const spByteArray_t &spInputAudioData) const;

    /*! Checks if the queue of returned audio data has any contents and
     * if so, it dequeues the first entry into 'spReceivedData'.
     * @remark Thread-safe
     * @param spReceivedData A shared pointer to a QByteArray.
     * @return True if there was any data in the queue, false otherwise.
     */
    bool GetReceivedAudioData(spByteArray_t &spReceivedData) const;


private:

    struct Impl;
    std::unique_ptr<Impl> m;

    void ConfigureClient(const QString& ipIn, const QString& portIn);

    /*! Initializes PortAudio - MUST be called before using PortAudio anywhere else.
     * @returns false if portaudio was not initialized successfully or if the number of audio devices came back negative.
     */
    bool InitPortAudio();

    void ConfigureDevices(const QString& inputDeviceIndex, const QString& outputDeviceIndex);

    /*! Checks if the parameters given are valid.
     * @param framesPerBuffer Number of frames that are collected until the callback function is called again.
     * @param sampleRate Number of times audio is written to the buffer per second
     * @param audioChannels Number of audio channels port audio is supposed to use for in- and output.
     * @return
     */
    void ConfigureAudioParameters(const QString &framesPerBuffer, const QString &sampleRate,
                                  const QString &audioChannels);

    /*! Configures the in- and output streaming parameters for portaudio.
     * Only call after configuring all other audio parameters for they are needed here.
     */
    void ConfigurePortaudioParameters();

    void ConfigurePaStreamParameters(const std::shared_ptr<PaStreamParameters> &parameters,
                                 const PaDeviceIndex &deviceIndex,
                                 PaMacCoreStreamInfo& coreAudioInfo);

    void ConfigureCodec(const QString& encodingEnabled, const QString& compressionFactor);

    void ConfigureJitterBuffer(const QString& bufferSize);

    /*! Emits a signal to de-couple the next action from the callback function's context.
     * This makes the sending of data thread-safe.
     * @remark Thread-safe
     * @param spInputAudioData A shared pointer to the QByteArray with the data
     * that is supposed to go to the server.
     */
    void SendAudioInputToServer(const spByteArray_t &spInputAudioData) const;


signals:
    /*! Signal to de-couple the contexts of the callback function and the AudioManager.
     * @param inputData A shared Pointer to the QByteArray with the data that is
     * supposed to be sent.
     */
    void sigSendAudioInputToServer(spByteArray_t inputData) const;

private slots:
    /*! Slot that calls the Client's 'SendAudioData()' function.
     * It also measures the time since the last time this slot was pinged.
     * @param spInputAudioData A shared pointer to the QByteArray with the data that is
     * supposed to be sent.
     */
    void slotSendAudioInputToServer(const spByteArray_t &spInputAudioData) const;

    /*! Slot that enqueues the received pointers in the list into a QByteArray.
     * @param data A shared pointer to List of shared pointers to the QByteArrays that were returned from the server at a time.
     * @remark Thread-safe
     */
    void slotClientReceivedAudioData(const spListSpByteArray_t& data) const;
};

#endif // AUDIOMANAGER_H
