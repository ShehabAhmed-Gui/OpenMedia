#include "mediaplayer.h"

MediaPlayer::MediaPlayer(QSharedPointer<VideoController> videoController,
                         QObject *parent)
    : QObject{parent}
    , m_videoController(videoController)
{
    videoState = new VideoState;
    m_audioPlayer.reset(new AudioPlayer(videoState, this));
    connect(m_audioPlayer.get(), &AudioPlayer::muteChanged, this, &MediaPlayer::mutedChanged);

    demuxerThread = new QThread(this);
    videoThread = new QThread(this);
    audioThread = new QThread(this);

    demuxer = new Demuxer;
    videoDecoder = new VideoDecoder;
    audioDecoder = new AudioDecoder;

    demuxer->moveToThread(demuxerThread);
    videoDecoder->moveToThread(videoThread);
    audioDecoder->moveToThread(audioThread);

    connect(demuxerThread, &QThread::finished, demuxer, &QObject::deleteLater);
    connect(videoThread, &QThread::finished, videoDecoder, &QObject::deleteLater);
    connect(audioThread, &QThread::finished, audioDecoder, &QObject::deleteLater);
    connect(videoDecoder, &VideoDecoder::videoFrameReady, this, &MediaPlayer::onVideoFrameReady);
    connect(audioDecoder, &AudioDecoder::audioFrameReady, this, &MediaPlayer::onAudioFrameReady);

    demuxerThread->start();
    videoThread->start();
    audioThread->start();
}

MediaPlayer::~MediaPlayer()
{
    videoDecoder->stop();
    videoThread->quit();
    videoThread->wait();

    demuxerThread->quit();
    demuxerThread->wait();

    audioDecoder->stop();
    audioThread->quit();
    audioThread->wait();
}

void MediaPlayer::open(const QString &file)
{
    demuxer->open(videoState, file);

    videoDecoder->open(videoState->video_st->codecpar);
    audioDecoder->open(videoState->audio_st->codecpar);

    emit durationChanged();
    emit sourceChanged();
}

void MediaPlayer::play()
{
    auto &audioStream = videoState->audio_st;

    m_audioPlayer->configureFormat(
        audioStream->codecpar->sample_rate,
        audioStream->codecpar->ch_layout.nb_channels
        );
    m_audioPlayer->play();

    m_playbackState = Playback::Playing;
    emit playbackStateChanged();

    QMetaObject::invokeMethod(
        demuxer,
        "start",
        videoState);

    QMetaObject::invokeMethod(
        audioDecoder,
        "start",
        videoState
        );

    QMetaObject::invokeMethod(
        videoDecoder,
        "start",
        videoState
        );
}

void MediaPlayer::stop()
{
    m_playbackState = Playback::Stopped;
    emit playbackStateChanged();
    m_audioPlayer->stop();

    videoDecoder->stop();
    audioDecoder->stop();
    demuxer->stop();

    videoState = new VideoState;
}

void MediaPlayer::pause_resume()
{
    if (m_audioPlayer->audioState() == QAudio::SuspendedState) {
        m_playbackState = Playback::Playing;
    } else if (m_audioPlayer->audioState() == QAudio::ActiveState) {
        m_playbackState = Playback::Stopped;
    }
    emit playbackStateChanged();
    m_audioPlayer->pause_resume();

    if (!videoState->paused) {
        videoState->paused = true;
        videoState->clock.pausedStartMs = videoState->clock.clock.elapsed();
    } else {
        videoState->paused = false;
        videoState->clock.pausedAccumulatedMs +=
            videoState->clock.now() - videoState->clock.pausedStartMs;
    }
}

void MediaPlayer::seek(double timestamp)
{
    // if (m_demuxer->seek(timestamp)) {
    //     // Flush decoder buffers
    //     m_videoDecoder.flushBuffers();
    //     m_audioDecoder.flushBuffers();
    // }
}

long MediaPlayer::framesCount() const
{
    if (!videoState) {
        return 0;
    }

    return videoState->video_st->nb_frames;
}

QString MediaPlayer::source()
{
    if (!videoState) {
        return QString("");
    }

    return videoState->fileName;
}

void MediaPlayer::setSource(const QString &newSource)
{
    videoState->fileName = newSource;
}

bool MediaPlayer::muted() const
{
    return m_audioPlayer->isMuted();
}

void MediaPlayer::setMuted(bool muted)
{
    m_audioPlayer->setMuted(muted);
}

double MediaPlayer::volume() const
{
    return m_audioPlayer->volume();
}

void MediaPlayer::setVolume(double volume)
{
    m_audioPlayer->setVolume(volume);
}

double MediaPlayer::duration() const
{
    if (!videoState || !videoState->video_st)
        return 0.0;

    AVRational tb = videoState->video_st->time_base;
    double seconds = videoState->video_st->duration * av_q2d(tb);

    // milliseconds
    return seconds * 1000.0;
}

void MediaPlayer::setDuration(double newDuration)
{
    // We don't need this.
}

Playback::PlaybackState MediaPlayer::playbackState()
{
    return m_playbackState;
}

void MediaPlayer::setPlaybackState(const Playback::PlaybackState &newPlaybackState)
{
    if (m_playbackState == newPlaybackState)
        return;
    m_playbackState = newPlaybackState;
    emit playbackStateChanged();
}

double MediaPlayer::position() const
{
    if (!videoState) {
        return -1;
    }

    AVRational tb = videoState->video_st->time_base;
    double seconds = videoState->position * av_q2d(tb);
    return qFloor(seconds * 1000);
}

void MediaPlayer::setPosition(double newPosition)
{
    if (qFuzzyCompare(m_position, newPosition))
        return;
    m_position = newPosition;
    emit positionChanged();
}

void MediaPlayer::onAudioFrameReady(const QByteArray &pcm)
{
    m_audioPlayer->pushPCM(pcm);
}

void MediaPlayer::onVideoFrameReady(const QImage &frame)
{
    emit videoFrameReady(frame);
    emit positionChanged();
}
