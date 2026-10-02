#ifndef MEDIAPLAYERCONTROLLER_H
#define MEDIAPLAYERCONTROLLER_H

#include <QObject>
#include "../mediaplayer.h"

class MediaPlayerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged FINAL)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL)
    Q_PROPERTY(double maxVolume READ maxVolume CONSTANT FINAL)

    Q_PROPERTY(Playback::PlaybackState playbackState READ playbackState WRITE setPlaybackState NOTIFY playbackStateChanged FINAL)
    Q_PROPERTY(double duration READ duration NOTIFY durationChanged FINAL)
    Q_PROPERTY(double position READ position WRITE setPosition NOTIFY positionChanged FINAL)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(double playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged FINAL)
    Q_PROPERTY(bool loop READ loop WRITE setLoop NOTIFY loopChanged FINAL)

public:
    explicit MediaPlayerController(QSharedPointer<MediaPlayer> mediaPlayer,
                                   QObject *parent = nullptr);

    Q_INVOKABLE void start(const QString& file);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(double timestamp);
    Q_INVOKABLE void seekBy(double deltaMs);
    Q_INVOKABLE void pause_resume();

    double volume();
    void setVolume(double volume);
    double maxVolume() const;

    bool muted() const;
    void setMuted(bool newMuted);

    double duration();

    double position() const;
    void setPosition(double newPosition);

    QString source();
    void setSource(const QString &newSource);

    double playbackRate() const;
    void setPlaybackRate(double rate);

    bool loop() const;
    void setLoop(bool loop);

    Playback::PlaybackState playbackState();
    void setPlaybackState(const Playback::PlaybackState &newPlaybackState);

signals:
    void videoFrameReady(AVFrame *frame);
    void audioArtworkReady(AVFrame *frame);
    void endOfMedia();

    void durationChanged();
    void playbackStateChanged();
    void mediaChanged();
    void sourceChanged();

    void mutedChanged(bool muted);
    void volumeChanged();

    void positionChanged();
    void playbackRateChanged();
    void loopChanged();

private:
    QSharedPointer<MediaPlayer> m_mediaPlayer;
};

#endif // MEDIAPLAYERCONTROLLER_H
