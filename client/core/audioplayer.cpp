#include "audioplayer.h"

#include <QDebug>
#include <QTimer>
#include <QtMath>

// Enough slack to survive a scheduling hiccup without adding noticeable lag.
static constexpr int SinkBufferMs = 120;
// Boost at MaxVolume. +6 dB sounds roughly 1.5x as loud and stays clean
// through the limiter.
static constexpr double MaxBoostDb = 6.0;

AudioPlayer::AudioPlayer(QSharedPointer<VideoState> videoState, QObject *parent)
    : QObject{ parent }
    , m_videoState(videoState)
{
    m_bufferDevice = new AudioBufferDevice(videoState, this);
    connect(m_bufferDevice, &AudioBufferDevice::prebuffered, this, &AudioPlayer::startSink);
    connect(&m_devices, &QMediaDevices::audioOutputsChanged, this, &AudioPlayer::onDefaultOutputDeviceChanged);
}

AudioPlayer::~AudioPlayer()
{
    stop();
}

bool AudioPlayer::configureFormat(const AVCodecParameters *parameters)
{
    const QAudioDevice device = m_devices.defaultAudioOutput();
    if (device.isNull()) {
        qWarning() << "No audio output device available";
        return false;
    }

    QAudioFormat format;
    format.setSampleRate(parameters->sample_rate);
    format.setChannelCount(parameters->ch_layout.nb_channels);
    format.setChannelConfig(QAudioFormat::defaultChannelConfigForChannelCount(
        parameters->ch_layout.nb_channels));
    format.setSampleFormat(QAudioFormat::Float);

    if (!device.isFormatSupported(format)) {
        format.setSampleFormat(QAudioFormat::Int16);
        if (!device.isFormatSupported(format)) {
            // The resampler adapts to whatever the device wants.
            format = device.preferredFormat();
        }
    }

    m_format = format;

    if (m_sink)
        m_sink->stop();
    delete m_sink;
    m_sink = new QAudioSink(device, m_format, this);
    m_sink->setBufferSize(m_format.bytesForDuration(SinkBufferMs * 1000));
    applyVolume();

    m_bufferDevice->configure(m_format, m_sink->bufferSize());

    qDebug() << "Audio output:" << m_format.sampleRate() << "Hz"
             << m_format.channelCount() << "ch" << m_format.sampleFormat();

    return true;
}

AVSampleFormat AudioPlayer::sampleFormat() const
{
    switch (m_format.sampleFormat()) {
    case QAudioFormat::UInt8:
        return AV_SAMPLE_FMT_U8;
    case QAudioFormat::Int16:
        return AV_SAMPLE_FMT_S16;
    case QAudioFormat::Int32:
        return AV_SAMPLE_FMT_S32;
    case QAudioFormat::Float:
        return AV_SAMPLE_FMT_FLT;
    default:
        return AV_SAMPLE_FMT_S16;
    }
}

void AudioPlayer::play()
{
    m_sinkStarted = false;
    m_startTimer.start();
    m_bufferDevice->restart();
    m_bufferDevice->start();
    // The sink is started from startSink() once enough audio is queued, so the
    // stream does not begin with an underrun.
}

void AudioPlayer::startSink()
{
    if (m_sinkStarted || !m_sink)
        return;

    // Give the video decoder a moment to produce its first frame, otherwise the
    // audio clock is already ahead of the picture when playback begins.
    if (m_videoState->hasVideo.load() && !m_videoState->videoPrimed.load()
        && m_startTimer.isValid() && m_startTimer.elapsed() < 1000) {
        QTimer::singleShot(5, this, &AudioPlayer::startSink);
        return;
    }

    m_sinkStarted = true;
    m_sink->start(m_bufferDevice);

    // The backend may not honour the requested size, and the clock depends on
    // knowing how much the device holds.
    m_bufferDevice->configure(m_format, m_sink->bufferSize());

    if (m_videoState->paused.load())
        m_sink->suspend();
}

void AudioPlayer::stop()
{
    m_sinkStarted = false;

    if (m_sink) {
        m_sink->stop();
        m_sink->reset();
    }

    if (m_bufferDevice)
        m_bufferDevice->stop();
}

void AudioPlayer::suspend()
{
    if (m_sink && m_sink->state() == QAudio::ActiveState)
        m_sink->suspend();
}

void AudioPlayer::resume()
{
    if (m_sink && m_sink->state() == QAudio::SuspendedState)
        m_sink->resume();
}

void AudioPlayer::flush()
{
    if (m_bufferDevice)
        m_bufferDevice->flush();
}

bool AudioPlayer::hasPendingAudio() const
{
    return m_bufferDevice && m_bufferDevice->hasPendingData();
}

double AudioPlayer::volume() const
{
    return m_volume;
}

void AudioPlayer::setVolume(double volume)
{
    volume = qBound(0.0, volume, MaxVolume);
    if (qFuzzyCompare(m_volume, volume))
        return;

    m_volume = volume;
    applyVolume();
    emit volumeChanged();
}

bool AudioPlayer::isMuted() const
{
    return m_muted;
}

void AudioPlayer::setMuted(bool muted)
{
    // QAudioSink has no mute of its own.
    if (m_muted == muted)
        return;

    m_muted = muted;
    applyVolume();
    emit muteChanged(muted);
}

void AudioPlayer::applyVolume()
{
    // QAudioSink clamps to 1.0, so anything past 100% is digital gain that the
    // buffer device's limiter keeps from clipping.
    const double boostDb = qMax(0.0, m_volume - 1.0) / (MaxVolume - 1.0) * MaxBoostDb;
    m_bufferDevice->setBoostGain(float(qPow(10.0, boostDb / 20.0)));

    if (!m_sink)
        return;

    const qreal linear = m_muted
        ? 0.0
        : QAudio::convertVolume(qMin(m_volume, 1.0), QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale);

    m_sink->setVolume(linear);
}

void AudioPlayer::onDefaultOutputDeviceChanged()
{
    if (!m_sink || !m_sinkStarted)
        return;

    const QAudioDevice device = m_devices.defaultAudioOutput();
    if (device.isNull() || !device.isFormatSupported(m_format))
        return;

    const bool wasSuspended = m_sink->state() == QAudio::SuspendedState;

    m_sink->stop();
    delete m_sink;

    m_sink = new QAudioSink(device, m_format, this);
    m_sink->setBufferSize(m_format.bytesForDuration(SinkBufferMs * 1000));
    applyVolume();
    m_sink->start(m_bufferDevice);

    if (wasSuspended)
        m_sink->suspend();
}
