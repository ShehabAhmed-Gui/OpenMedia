#include "audioplayer.h"
#include <qdebug.h>

AudioPlayer::AudioPlayer(QObject *parent)
    : QObject{parent}
{
    connect(&devicesController, &QMediaDevices::audioOutputsChanged, this, &AudioPlayer::onDefaultOutputDeviceChanged);
}

void AudioPlayer::play()
{
    audioSink->setBufferSize(8192 * 4);
    audioSink->setVolume(m_volume);
    setMuted(m_muted);

    bufferDevice = new AudioBufferDevice(this);
    bufferDevice->start();
    audioSink->start(bufferDevice);
}

void AudioPlayer::stop()
{
    reset();
    audioSink->stop();
}

void AudioPlayer::reset()
{
    audioSink->reset();
}

void AudioPlayer::pause_resume()
{
    switch (audioSink->state()) {
    case QAudio::SuspendedState:
        audioSink->resume();
        break;
    case QAudio::ActiveState:
        audioSink->suspend();
    default:
        break;
    }
}

QAudioSink *AudioPlayer::getAudioSink()
{
    return audioSink;
}

double AudioPlayer::volume()
{
    if (!audioSink) {
        return 0.0;
    }
    return audioSink->volume();
}

void AudioPlayer::setVolume(double volume)
{
    m_volume = volume;
    if (!audioSink) {
        return;
    }
    audioSink->setVolume(volume);
}

bool AudioPlayer::isMuted()
{
    return m_muted;
}

void AudioPlayer::setMuted(bool muted)
{
    // QAudioSink doesn't support mute
    // let's implement our own
    m_muted = muted;
    emit muteChanged(muted);
    if (!audioSink) {
        return;
    }
    audioSink->setVolume(muted? 0 : m_volume);
}

void AudioPlayer::configureFormat(int sampleRate, int channelCount)
{
    m_format.setSampleFormat(QAudioFormat::Int16);
    m_format.setSampleRate(sampleRate);
    m_format.setChannelCount(channelCount);

    audioOuput = devicesController.defaultAudioOutput();
    if (!audioOuput.isFormatSupported(m_format)) {
        m_format = audioOuput.preferredFormat();
    }

    audioSink = new QAudioSink(m_format, this);
}

void AudioPlayer::pushPCM(const QByteArray &pcm)
{
    bufferDevice->append(pcm);
}

QAudio::State AudioPlayer::audioState()
{
    return audioSink->state();
}

void AudioPlayer::onDefaultOutputDeviceChanged()
{
    if (audioSink) {
        stop();
        delete audioSink;
        audioSink = nullptr;
    }

    audioSink = new QAudioSink(m_format, this);
    play();
}
