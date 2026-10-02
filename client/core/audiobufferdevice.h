#ifndef AUDIOBUFFERDEVICE_H
#define AUDIOBUFFERDEVICE_H

#include <QAudioFormat>
#include <QIODevice>
#include <QMutex>
#include <QMutexLocker>
#include <QQueue>
#include <QWaitCondition>

#include "audiolimiter.h"
#include "defs.h"

// FIFO between the audio decoder and QAudioSink.
//
// It is also where the master clock comes from: every read tells us how much
// audio the device has taken, which maps back onto the stream timestamp that is
// audible right now. Writes block while the FIFO is full, so decoding can never
// run away from playback.
class AudioBufferDevice : public QIODevice
{
    Q_OBJECT
public:
    explicit AudioBufferDevice(QSharedPointer<VideoState> videoState, QObject *parent = nullptr);

    void start();
    void stop();

    void configure(const QAudioFormat &format, qint64 deviceBufferBytes);

    // Linear gain applied before the limiter. Thread safe.
    void setBoostGain(float gain);

    // Called from the audio decoder thread. Blocks while the FIFO is full.
    // streamSpan is how much stream time the samples cover, which differs from
    // their playback duration once the tempo filter is in use.
    bool append(const QByteArray &pcm, double pts, double streamSpan);

    void flush();   // drop everything, used on seek
    void abort();   // unblock append() and refuse further data
    void restart(); // re-arm after abort()

    bool hasPendingData() const;

signals:
    // Enough audio is queued for the sink to start without an instant underrun.
    void prebuffered();

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
    qint64 bytesAvailable() const override;
    bool isSequential() const override { return true; }

private:
    struct Chunk {
        QByteArray data;
        double pts;
        double ptsPerByte;
        qint64 offset;
    };

    mutable QMutex m_mutex;
    QWaitCondition m_cond;
    QQueue<Chunk> m_chunks;

    QSharedPointer<VideoState> m_videoState;

    qint64 m_queuedBytes = 0;
    qint64 m_maxBytes = 0;
    qint64 m_prebufferBytes = 0;
    qint64 m_deviceBufferBytes = 0;
    int m_bytesPerSecond = 0;
    double m_playPts = 0.0;
    double m_ptsPerByte = 0.0;
    bool m_abort = false;
    bool m_prebuffered = false;

    AudioLimiter m_limiter;
    std::vector<float> m_scratch;
    QAudioFormat::SampleFormat m_sampleFormat = QAudioFormat::Unknown;
    int m_sampleRate = 0;
    int m_channels = 0;
    int m_frameBytes = 1;
    qint64 m_tailFrames = 0;
};

#endif // AUDIOBUFFERDEVICE_H
