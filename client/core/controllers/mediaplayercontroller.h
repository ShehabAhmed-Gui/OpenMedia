#ifndef MEDIAPLAYERCONTROLLER_H
#define MEDIAPLAYERCONTROLLER_H

#include <QObject>
#include "../mediaplayer.h"

class MediaPlayerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged FINAL)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL)
    Q_PROPERTY(double duration READ duration WRITE setDuration NOTIFY durationChanged FINAL)

public:
    explicit MediaPlayerController(QSharedPointer<MediaPlayer> mediaPlayer,
                                   QObject *parent = nullptr);

    Q_INVOKABLE bool open(const QString& file);
    Q_INVOKABLE void play();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(double timestamp);
    Q_INVOKABLE void pause_resume();

    Q_INVOKABLE QString source();
    Q_INVOKABLE Playback::PlaybackState playbackState();

    double volume();
    void setVolume(double volume);

    bool muted() const;
    void setMuted(bool newMuted);

    double duration();
    void setDuration(double newDuration);

signals:
    void videoFrameReady(const QImage &image);

    void durationChanged();
    void playbackStateChanged();
    void mediaChanged();

    void mutedChanged(bool muted);
    void volumeChanged();

private:
    QSharedPointer<MediaPlayer> m_mediaPlayer;
    bool m_muted;
    double m_volume;
    double m_duration;
};

#endif // MEDIAPLAYERCONTROLLER_H
