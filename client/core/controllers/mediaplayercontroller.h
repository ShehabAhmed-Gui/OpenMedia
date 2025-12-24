#ifndef MEDIAPLAYERCONTROLLER_H
#define MEDIAPLAYERCONTROLLER_H

#include <QObject>
#include "../mediaplayer.h"

class MediaPlayerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged FINAL)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL)

    Q_PROPERTY(Playback::PlaybackState playbackState READ playbackState WRITE setPlaybackState NOTIFY playbackStateChanged FINAL)
    Q_PROPERTY(double duration READ duration WRITE setDuration NOTIFY durationChanged FINAL)
    Q_PROPERTY(double position READ position WRITE setPosition NOTIFY positionChanged FINAL)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged FINAL)

public:
    explicit MediaPlayerController(QSharedPointer<MediaPlayer> mediaPlayer,
                                   QObject *parent = nullptr);

    Q_INVOKABLE void open(const QString& file);
    Q_INVOKABLE void play();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(double timestamp);
    Q_INVOKABLE void pause_resume();

    double volume();
    void setVolume(double volume);

    bool muted() const;
    void setMuted(bool newMuted);

    double duration();
    void setDuration(double newDuration);

    double position() const;
    void setPosition(double newPosition);

    QString source();
    void setSource(const QString &newSource);

    Playback::PlaybackState playbackState();
    void setPlaybackState(const Playback::PlaybackState &newPlaybackState);

signals:
    void videoFrameReady(const QImage &image);

    void durationChanged();
    void playbackStateChanged();
    void mediaChanged();
    void sourceChanged();

    void mutedChanged(bool muted);
    void volumeChanged();

    void positionChanged();

private:
    QSharedPointer<MediaPlayer> m_mediaPlayer;
    bool m_muted;
    double m_volume;
    double m_duration;
    double m_position;
    QString m_source;
    Playback::PlaybackState m_playbackState;
};

#endif // MEDIAPLAYERCONTROLLER_H
