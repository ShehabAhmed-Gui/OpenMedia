#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QThread>
#include <QTimer>

#include "audioplayer.h"
#include "defs.h"
#include "videomanager.h"

#include "audiodecoder.h"
#include "demuxer.h"
#include "videodecoder.h"

#include "thumbnailsextractor.h"

class Playback : public QObject
{
    Q_OBJECT
public:
    enum PlaybackState {
        Stopped,
        Playing,
        Paused
    };
    Q_ENUM(PlaybackState)
};

class MediaPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged FINAL)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL)
    Q_PROPERTY(double maxVolume READ maxVolume CONSTANT FINAL)

    Q_PROPERTY(double duration READ duration NOTIFY durationChanged FINAL)
    Q_PROPERTY(Playback::PlaybackState playbackState READ playbackState WRITE setPlaybackState NOTIFY playbackStateChanged FINAL)
    Q_PROPERTY(double position READ position WRITE setPosition NOTIFY positionChanged FINAL)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(double playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged FINAL)
    Q_PROPERTY(bool loop READ loop WRITE setLoop NOTIFY loopChanged FINAL)

public:
    explicit MediaPlayer(QSharedPointer<VideoManager> videoManager, QObject *parent = nullptr);
    ~MediaPlayer();

    void start(const QString &file);
    void stop();
    void pause_resume();
    void seek(double positionMs);
    // Relative seek that reads the current position itself, so it works the
    // same whether playing, paused or mid-seek.
    void seekBy(double deltaMs);

    long framesCount() const;

    QString source() const;
    void setSource(const QString &newSource);

    bool muted() const;
    void setMuted(bool newMuted);
    double volume() const;
    void setVolume(double newVolume);
    double maxVolume() const;

    double duration() const;

    Playback::PlaybackState playbackState() const;
    void setPlaybackState(const Playback::PlaybackState &newPlaybackState);

    double position() const;
    void setPosition(double newPosition);

    double playbackRate() const;
    void setPlaybackRate(double rate);

    bool loop() const;
    void setLoop(bool loop);

signals:
    void resetCompleted();
    void endOfMedia();
    void videoFrameReady(AVFrame *frame);
    // Still picture standing in for a file without a video track.
    void audioArtworkReady(AVFrame *frame);
    void mediaChanged();
    void playbackStateChanged();
    void sourceChanged();
    void mutedChanged(bool muted);
    void volumeChanged();
    void durationChanged();
    void positionChanged();
    void playbackRateChanged();
    void loopChanged();

private slots:
    void onStreamsReady(QSharedPointer<VideoState> videoState);
    void onVideoFrameReady(AVFrame *frame);
    void onDecoderFinished();
    void onDemuxerFinished();
    void onDrainTick();

private:
    void setPlaybackStateInternal(Playback::PlaybackState state);
    void finalizeStop();

    QSharedPointer<VideoManager> m_videoManager;
    QSharedPointer<VideoState> videoState;
    QSharedPointer<AudioPlayer> m_audioPlayer;

    QScopedPointer<ThumbnailsExtractor> m_thumbnailExtractor;

    QThread *demuxerThread;
    QThread *videoThread;
    QThread *audioThread;

    Demuxer *demuxer;
    VideoDecoder *videoDecoder;
    AudioDecoder *audioDecoder;

    QTimer m_positionTimer;
    QTimer m_drainTimer;

    QString m_pendingSource;
    QString m_source;

    double m_playbackRate = 1.0;
    bool m_loop = false;
    int m_activeDecoders = 0;
    int m_drainTicks = 0;
    bool m_demuxerRunning = false;
    bool m_decodersStarted = false;
    bool m_finalizePending = false;
    bool m_stopRequested = false;

    Playback::PlaybackState m_playbackState = Playback::Stopped;
};

#endif // MEDIAPLAYER_H
