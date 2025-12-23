#include "mediaplayer.h"

MediaPlayer::MediaPlayer(QSharedPointer<VideoManager> videoManager,
                         QObject *parent)
    : QObject{parent}
    , m_videoManager(videoManager)
{
    m_demuxer.reset(new Demuxer());
    m_audioPlayer.reset(new AudioPlayer(this));

    // audio signals
    connect(m_audioPlayer.get(), &AudioPlayer::muteChanged, this, &MediaPlayer::mutedChanged);

    workerThread = new QThread(this);
    mediaWorker = new MediaWorker(m_demuxer);

    mediaWorker->moveToThread(workerThread);
    connect(mediaWorker, &MediaWorker::audioFrameReady, this, &MediaPlayer::onAudioFrameReady);
    connect(mediaWorker, &MediaWorker::videoFrameReady, this, &MediaPlayer::onVideoFrameReady);
    connect(mediaWorker, &MediaWorker::finished, workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished, mediaWorker, &QObject::deleteLater);
    workerThread->start();
}

MediaPlayer::~MediaPlayer()
{
    workerThread->quit();
    workerThread->wait();
}

bool MediaPlayer::open(const QString &file)
{
    videoState = new VideoState;
    m_demuxer->open(videoState, file);
    QMetaObject::invokeMethod(mediaWorker, "open", videoState);

    // Extract video thumbnails
    m_videoManager->extractVideoThumbnails(file);
    emit durationChanged();
    return true;
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
    QMetaObject::invokeMethod(mediaWorker, "start", videoState);
}

void MediaPlayer::stop()
{
    m_playbackState = Playback::Stopped;
    emit playbackStateChanged();
    m_audioPlayer->stop();
    mediaWorker->stop();
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
    mediaWorker->pause_resume(videoState);
}

void MediaPlayer::seek(double timestamp)
{
    // if (m_demuxer->seek(timestamp)) {
    //     // Flush decoder buffers
    //     m_videoDecoder.flushBuffers();
    //     m_audioDecoder.flushBuffers();
    // }
}

QString MediaPlayer::source()
{
    if (!videoState) {
        return QString("");
    }
    return videoState->fileName;
}

long MediaPlayer::framesCount() const
{
    if (!videoState) {
        return 0;
    }

    return videoState->video_st->nb_frames;
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

    // TODO: get pos from avframe
    return 0.0;
}

void MediaPlayer::onAudioFrameReady(const QByteArray &pcm)
{
    m_audioPlayer->pushPCM(pcm);
}

void MediaPlayer::onVideoFrameReady(const QImage &frame)
{
    emit videoFrameReady(frame);
}

void MediaPlayer::setPosition(double newPosition)
{
    if (qFuzzyCompare(m_position, newPosition))
        return;
    m_position = newPosition;
    emit positionChanged();
}
