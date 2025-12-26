#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>

#include "controllers/videocontroller.h"
#include "demuxer.h"
#include "videodecoder.h"
#include "audiodecoder.h"

#include "defs.h"

#include "audioplayer.h"
#include <QThread>

class Playback : public QObject
{
    Q_OBJECT
public:
    enum PlaybackState {
        Stopped,
        Playing
    };
    Q_ENUM(PlaybackState)
};

class MediaPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged FINAL)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL)

    Q_PROPERTY(double duration READ duration WRITE setDuration NOTIFY durationChanged FINAL)
    Q_PROPERTY(Playback::PlaybackState playbackState READ playbackState WRITE setPlaybackState NOTIFY playbackStateChanged FINAL)
    Q_PROPERTY(double position READ position WRITE setPosition NOTIFY positionChanged FINAL)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged FINAL)

public:
    explicit MediaPlayer(QSharedPointer<VideoController> videoController,
                         QObject *parent = nullptr);
    ~MediaPlayer();

    void open(const QString& file);
    void play();
    void stop();
    void pause_resume();
    void seek(double timestamp);

    long framesCount() const;

    QString source();
    void setSource(const QString &newSource);

    bool muted() const;
    void setMuted(bool newMuted);
    double volume() const;
    void setVolume(double newVolume);

    double duration() const;
    void setDuration(double newDuration);

    Playback::PlaybackState playbackState();
    void setPlaybackState(const Playback::PlaybackState &newPlaybackState);

    double position() const;
    void setPosition(double newPosition);

public slots:
    void onAudioFrameReady(const QByteArray &pcm);
    void onVideoFrameReady(const QImage &frame);

signals:
    void videoFrameReady(const QImage &image);
    void mediaChanged();
    void playbackStateChanged();

    void sourceChanged();

    void mutedChanged(bool muted);
    void volumeChanged();
    void durationChanged();
    void positionChanged();

private:
    QSharedPointer<VideoController> m_videoController;
    VideoState *videoState = nullptr;

    AVFormatContext *m_fmtCtx = nullptr;
    AVCodecContext *m_ctx;
    QSharedPointer<AudioPlayer> m_audioPlayer;

    int videoStreamIndex = -1;
    bool m_playing = false;

    QThread *videoThread;
    QThread *audioThread;
    QThread *demuxerThread;

    VideoDecoder *videoDecoder;
    AudioDecoder *audioDecoder;
    Demuxer *demuxer;

    bool m_muted;
    double m_volume;
    double m_duration;
    Playback::PlaybackState m_playbackState = Playback::Stopped;
    double m_position;
    QString m_source;
};

#endif // MEDIAPLAYER_H
