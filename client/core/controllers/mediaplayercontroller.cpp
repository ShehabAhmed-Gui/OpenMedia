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
    connect(m_mediaPlayer.get(), &MediaPlayer::sourceChanged, this, &MediaPlayerController::sourceChanged);
    connect(m_mediaPlayer.get(), &MediaPlayer::positionChanged, this, &MediaPlayerController::positionChanged);
}

void MediaPlayerController::open(const QString &file)
{
    m_mediaPlayer->open(file);
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

double MediaPlayerController::volume()
{
    return m_mediaPlayer->volume();
}

void MediaPlayerController::setVolume(double volume)
{
    m_mediaPlayer->setVolume(volume);
}

bool MediaPlayerController::muted() const
{
    return m_mediaPlayer->muted();
}

void MediaPlayerController::setMuted(bool muted)
{
    m_mediaPlayer->setMuted(muted);
}

double MediaPlayerController::duration()
{
    return m_mediaPlayer->duration();
}

void MediaPlayerController::setDuration(double newDuration)
{
    m_mediaPlayer->setDuration(newDuration);
}

double MediaPlayerController::position() const
{
    return m_mediaPlayer->position();
}

void MediaPlayerController::setPosition(double newPosition)
{
    m_mediaPlayer->setPosition(newPosition);
}

QString MediaPlayerController::source()
{
    return m_mediaPlayer->source();
}

void MediaPlayerController::setSource(const QString &newSource)
{
    m_mediaPlayer->setSource(newSource);
}

Playback::PlaybackState MediaPlayerController::playbackState()
{
    return m_mediaPlayer->playbackState();
}

void MediaPlayerController::setPlaybackState(const Playback::PlaybackState &newPlaybackState)
{
    m_mediaPlayer->setPlaybackState(newPlaybackState);
}
