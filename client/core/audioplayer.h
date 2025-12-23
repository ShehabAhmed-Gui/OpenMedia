#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QAudioSink>
#include <QAudioFormat>
#include <QIODevice>
#include <QByteArray>
#include <QMediaDevices>
#include <QThread>

#include "audiobufferdevice.h"

class AudioPlayer : public QObject
{
    Q_OBJECT

public:
    explicit AudioPlayer(QObject *parent = nullptr);
    void play();
    void stop();
    void pause_resume();

    QAudioSink *getAudioSink();

    double volume();
    void setVolume(double volume);

    bool isMuted();
    void setMuted(bool muted);

    void configureFormat(int sampleRate, int channelCount);
    void pushPCM(const QByteArray &pcm);

    QAudio::State audioState();

signals:
    void muteChanged(bool muted);

private:
    void onDefaultOutputDeviceChanged();
    void reset();
    bool m_paused;
    bool m_muted = false;
    double m_volume;
    QMediaDevices devicesController;
    QAudioDevice audioOuput;
    QAudioSink *audioSink = nullptr;
    QAudioFormat m_format;
    QIODevice *audioDevice;

    AudioBufferDevice *bufferDevice;
};

#endif // AUDIOPLAYER_H
