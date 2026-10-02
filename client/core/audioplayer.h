#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QElapsedTimer>
#include <QMediaDevices>
#include <QObject>

#include "audiobufferdevice.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/samplefmt.h>
}

class AudioPlayer : public QObject
{
    Q_OBJECT

public:
    static constexpr double MaxVolume = 1.5;

    explicit AudioPlayer(QSharedPointer<VideoState> videoState, QObject *parent = nullptr);
    ~AudioPlayer();

    // Negotiates a format the output device accepts. The decoder resamples to
    // whatever comes out of here, which is what keeps playback at real speed.
    bool configureFormat(const AVCodecParameters *parameters);

    void play();
    void stop();
    void suspend();
    void resume();
    void flush();

    AudioBufferDevice *bufferDevice() const { return m_bufferDevice; }
    bool hasPendingAudio() const;

    int sampleRate() const { return m_format.sampleRate(); }
    int channelCount() const { return m_format.channelCount(); }
    AVSampleFormat sampleFormat() const;

    double volume() const;
    void setVolume(double volume);

    bool isMuted() const;
    void setMuted(bool muted);

signals:
    void muteChanged(bool muted);
    void volumeChanged();

private:
    void startSink();
    void applyVolume();
    void onDefaultOutputDeviceChanged();

    QMediaDevices m_devices;
    QAudioSink *m_sink = nullptr;
    AudioBufferDevice *m_bufferDevice = nullptr;
    QAudioFormat m_format;
    QSharedPointer<VideoState> m_videoState;

    QElapsedTimer m_startTimer;
    bool m_muted = false;
    bool m_sinkStarted = false;
    double m_volume = 1.0;
};

#endif // AUDIOPLAYER_H
