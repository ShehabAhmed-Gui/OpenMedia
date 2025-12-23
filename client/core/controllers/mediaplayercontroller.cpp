#include "mediaplayercontroller.h"

MediaPlayerController::MediaPlayerController(QSharedPointer<MediaPlayer> mediaPlayer,
                                             QObject *parent)
    : QObject{parent}
    , m_mediaPlayer(mediaPlayer)
{
    connect(m_mediaPlayer.get(), &MediaPlayer::playbackStateChanged, this, &MediaPlayerController::playbackStateChanged);
    connect(m_mediaPlayer.get(), &MediaPlayer::mediaChanged, this, &MediaPlayerController::mediaChanged);
    connect(m_mediaPlayer.get(), &MediaPlayer::durationChanged, this, &MediaPlayerController::durationChanged);
    connect(m_mediaPlayer.get(), &MediaPlayer::videoFrameReady, this, &MediaPlayerController::videoFrameReady);

    connect(m_mediaPlayer.get(), &MediaPlayer::mutedChanged, this, &MediaPlayerController::mutedChanged);
}

double MediaPlayerController::duration()
{
    return m_mediaPlayer->duration();
}

bool MediaPlayerController::open(const QString &file)
{
    return m_mediaPlayer->open(file);
}

void MediaPlayerController::play()
{
    m_mediaPlayer->play();
}

void MediaPlayerController::stop()
{
    m_mediaPlayer->stop();
}

void MediaPlayerController::seek(double timestamp)
{
    m_mediaPlayer->seek(timestamp);
}

void MediaPlayerController::pause_resume()
{
    m_mediaPlayer->pause_resume();
}

QString MediaPlayerController::source()
{
    return m_mediaPlayer->source();
}

Playback::PlaybackState MediaPlayerController::playbackState()
{
    return m_mediaPlayer->playbackState();
}

double MediaPlayerController::volume()
{
    return m_mediaPlayer->volume();
}

void MediaPlayerController::setVolume(double volume)
{
    m_mediaPlayer->setVolume(volume);
}

void MediaPlayerController::setMuted(bool muted)
{
    m_mediaPlayer->setMuted(muted);
}

bool MediaPlayerController::muted() const
{
    return m_mediaPlayer->muted();
}

void MediaPlayerController::setDuration(double newDuration)
{
    m_mediaPlayer->setDuration(newDuration);
}
