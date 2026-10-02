#include "audiobufferdevice.h"

#include <algorithm>
#include <cmath>

static constexpr qint64 BlockFrames = 1024;

namespace {

void toFloat(QAudioFormat::SampleFormat format, const char *in, float *out, qint64 samples)
{
    switch (format) {
    case QAudioFormat::UInt8:
        for (qint64 i = 0; i < samples; ++i)
            out[i] = (static_cast<uchar>(in[i]) - 128) / 128.0f;
        break;
    case QAudioFormat::Int16:
        for (qint64 i = 0; i < samples; ++i) {
            qint16 s;
            memcpy(&s, in + i * sizeof(s), sizeof(s));
            out[i] = s / 32768.0f;
        }
        break;
    case QAudioFormat::Int32:
        for (qint64 i = 0; i < samples; ++i) {
            qint32 s;
            memcpy(&s, in + i * sizeof(s), sizeof(s));
            out[i] = float(s / 2147483648.0);
        }
        break;
    case QAudioFormat::Float:
        memcpy(out, in, samples * sizeof(float));
        break;
    default:
        break;
    }
}

// Same scale as toFloat() so unboosted integer audio round-trips exactly.
void fromFloat(QAudioFormat::SampleFormat format, const float *in, char *out, qint64 samples)
{
    switch (format) {
    case QAudioFormat::UInt8:
        for (qint64 i = 0; i < samples; ++i)
            out[i] = char(qBound(0L, lrintf(in[i] * 128.0f) + 128, 255L));
        break;
    case QAudioFormat::Int16:
        for (qint64 i = 0; i < samples; ++i) {
            const qint16 s = qint16(qBound(-32768L, lrintf(in[i] * 32768.0f), 32767L));
            memcpy(out + i * sizeof(s), &s, sizeof(s));
        }
        break;
    case QAudioFormat::Int32:
        for (qint64 i = 0; i < samples; ++i) {
            const qint32 s = qint32(qBound(-2147483648LL, llrint(in[i] * 2147483648.0), 2147483647LL));
            memcpy(out + i * sizeof(s), &s, sizeof(s));
        }
        break;
    case QAudioFormat::Float:
        memcpy(out, in, samples * sizeof(float));
        break;
    default:
        break;
    }
}

} // namespace

AudioBufferDevice::AudioBufferDevice(QSharedPointer<VideoState> videoState, QObject *parent)
    : QIODevice{ parent }
    , m_videoState(videoState)
{
}

void AudioBufferDevice::start()
{
    if (!isOpen())
        open(QIODevice::ReadOnly);
}

void AudioBufferDevice::stop()
{
    abort();
    close();
}

void AudioBufferDevice::configure(const QAudioFormat &format, qint64 deviceBufferBytes)
{
    QMutexLocker lock(&m_mutex);
    m_bytesPerSecond = qMax(1, format.bytesForDuration(1000000));
    m_deviceBufferBytes = deviceBufferBytes;
    // Half a second of audio buffered ahead, a fifth of a second before the
    // sink is allowed to start.
    m_maxBytes = m_bytesPerSecond / 2;
    m_prebufferBytes = qMin(m_bytesPerSecond / 5, m_maxBytes);

    m_sampleFormat = format.sampleFormat();
    m_frameBytes = qMax(1, format.bytesPerFrame());

    // This also runs after the sink has started pulling, so only rebuild the
    // limiter when the layout changed, or audio would drop mid-stream.
    if (format.sampleRate() != m_sampleRate || format.channelCount() != m_channels) {
        m_sampleRate = format.sampleRate();
        m_channels = format.channelCount();
        m_limiter.configure(m_sampleRate, m_channels);
        m_scratch.assign(BlockFrames * m_channels, 0.0f);
        m_tailFrames = 0;
    }
}

void AudioBufferDevice::setBoostGain(float gain)
{
    m_limiter.setGain(gain);
}

bool AudioBufferDevice::append(const QByteArray &pcm, double pts, double streamSpan)
{
    if (pcm.isEmpty())
        return true;

    bool notifyReady = false;
    {
        QMutexLocker lock(&m_mutex);

        while (!m_abort && m_queuedBytes >= m_maxBytes)
            m_cond.wait(&m_mutex);

        if (m_abort)
            return false;

        m_chunks.enqueue({ pcm, pts, streamSpan / pcm.size(), 0 });
        m_queuedBytes += pcm.size();

        if (!m_prebuffered && m_queuedBytes >= m_prebufferBytes) {
            m_prebuffered = true;
            notifyReady = true;
        }
    }

    if (notifyReady)
        emit prebuffered();

    return true;
}

qint64 AudioBufferDevice::readData(char *data, qint64 maxlen)
{
    QMutexLocker lock(&m_mutex);

    maxlen -= maxlen % m_frameBytes;

    qint64 written = 0;
    while (written < maxlen && !m_chunks.isEmpty()) {
        Chunk &chunk = m_chunks.head();
        const qint64 available = chunk.data.size() - chunk.offset;
        const qint64 count = qMin(maxlen - written, available);

        memcpy(data + written, chunk.data.constData() + chunk.offset, count);

        chunk.offset += count;
        written += count;
        m_queuedBytes -= count;
        m_playPts = chunk.pts + double(chunk.offset) * chunk.ptsPerByte;
        m_ptsPerByte = chunk.ptsPerByte;

        if (chunk.offset >= chunk.data.size())
            m_chunks.dequeue();
    }

    // The limiter holds back its look-ahead worth of audio. Once the queue runs
    // dry, feed it silence so the end of the stream still comes out.
    if (written > 0)
        m_tailFrames = m_limiter.latencyFrames();
    const qint64 realFrames = written / m_frameBytes;
    const qint64 padFrames = qMin((maxlen - written) / m_frameBytes, m_tailFrames);
    m_tailFrames -= padFrames;

    const qint64 totalFrames = realFrames + padFrames;
    for (qint64 done = 0; done < totalFrames;) {
        const qint64 frames = qMin(totalFrames - done, BlockFrames);
        const qint64 real = qBound<qint64>(0, realFrames - done, frames);
        char *bytes = data + done * m_frameBytes;

        toFloat(m_sampleFormat, bytes, m_scratch.data(), real * m_channels);
        std::fill(m_scratch.begin() + real * m_channels, m_scratch.begin() + frames * m_channels, 0.0f);
        m_limiter.process(m_scratch.data(), int(frames));
        fromFloat(m_sampleFormat, m_scratch.data(), bytes, frames * m_channels);
        done += frames;
    }

    if (written > 0) {
        // What we hand over now only becomes audible once the sink has played
        // what it already holds and the limiter has let it through, so the
        // clock trails the last queued sample by that much.
        const qint64 queuedInDevice = qMax<qint64>(0, m_deviceBufferBytes - maxlen);
        const qint64 limiterBytes = qint64(m_limiter.latencyFrames()) * m_frameBytes;
        const double latency = double(written + queuedInDevice + limiterBytes) * m_ptsPerByte;
        m_videoState->audioClock.set(m_playPts - latency);
        m_cond.wakeAll();
    }

    return totalFrames * m_frameBytes;
}

qint64 AudioBufferDevice::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data)
    Q_UNUSED(len)
    return 0;
}

qint64 AudioBufferDevice::bytesAvailable() const
{
    QMutexLocker lock(&m_mutex);
    return m_queuedBytes + m_tailFrames * m_frameBytes + QIODevice::bytesAvailable();
}

void AudioBufferDevice::flush()
{
    QMutexLocker lock(&m_mutex);
    m_chunks.clear();
    m_queuedBytes = 0;
    m_limiter.reset();
    m_tailFrames = 0;
    m_videoState->audioClock.invalidate();
    m_cond.wakeAll();
}

void AudioBufferDevice::abort()
{
    QMutexLocker lock(&m_mutex);
    m_abort = true;
    m_chunks.clear();
    m_queuedBytes = 0;
    m_limiter.reset();
    m_tailFrames = 0;
    m_videoState->audioClock.invalidate();
    m_cond.wakeAll();
}

void AudioBufferDevice::restart()
{
    QMutexLocker lock(&m_mutex);
    m_abort = false;
    m_prebuffered = false;
    m_chunks.clear();
    m_queuedBytes = 0;
    m_limiter.reset();
    m_tailFrames = 0;
    m_playPts = 0.0;
    m_ptsPerByte = 0.0;
}

bool AudioBufferDevice::hasPendingData() const
{
    QMutexLocker lock(&m_mutex);
    return m_queuedBytes > 0 || m_tailFrames > 0;
}
