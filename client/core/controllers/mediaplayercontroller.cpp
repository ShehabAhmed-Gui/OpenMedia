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
    connect(m_mediaPlayer.get(), &MediaPlayer::audioArtworkReady, this, &MediaPlayerController::audioArtworkReady);

    connect(m_mediaPlayer.get(), &MediaPlayer::mutedChanged, this, &MediaPlayerController::mutedChanged);
    connect(m_mediaPlayer.get(), &MediaPlayer::sourceChanged, this, &MediaPlayerController::sourceChanged);
    connect(m_mediaPlayer.get(), &MediaPlayer::positionChanged, this, &MediaPlayerController::positionChanged);
    connect(m_mediaPlayer.get(), &MediaPlayer::volumeChanged, this, &MediaPlayerController::volumeChanged);
    connect(m_mediaPlayer.get(), &MediaPlayer::endOfMedia, this, &MediaPlayerController::endOfMedia);
    connect(m_mediaPlayer.get(), &MediaPlayer::playbackRateChanged, this, &MediaPlayerController::playbackRateChanged);
    connect(m_mediaPlayer.get(), &MediaPlayer::loopChanged, this, &MediaPlayerController::loopChanged);
}

void MediaPlayerController::start(const QString &file)
{
    m_mediaPlayer->start(file);
}

void MediaPlayerController::stop()
{
    m_mediaPlayer->stop();
}

void MediaPlayerController::seek(double timestamp)
{
    m_mediaPlayer->seek(timestamp);
}

void MediaPlayerController::seekBy(double deltaMs)
{
    m_mediaPlayer->seekBy(deltaMs);
}

double MediaPlayerController::playbackRate() const
{
    return m_mediaPlayer->playbackRate();
}

void MediaPlayerController::setPlaybackRate(double rate)
{
    m_mediaPlayer->setPlaybackRate(rate);
}

bool MediaPlayerController::loop() const
{
    return m_mediaPlayer->loop();
}

void MediaPlayerController::setLoop(bool loop)
{
    m_mediaPlayer->setLoop(loop);
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

double MediaPlayerController::maxVolume() const
{
    return m_mediaPlayer->maxVolume();
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
