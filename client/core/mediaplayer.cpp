#include "mediaplayer.h"

#include "artworkframe.h"

#include <QtMath>

MediaPlayer::MediaPlayer(QSharedPointer<VideoManager> videoManager, QObject *parent)
    : QObject{ parent }
    , m_videoManager(videoManager)
{
    videoState.reset(new VideoState);
    m_audioPlayer.reset(new AudioPlayer(videoState));

    connect(m_audioPlayer.get(), &AudioPlayer::muteChanged, this, &MediaPlayer::mutedChanged);
    connect(m_audioPlayer.get(), &AudioPlayer::volumeChanged, this, &MediaPlayer::volumeChanged);

    demuxerThread = new QThread(this);
    videoThread = new QThread(this);
    audioThread = new QThread(this);

    demuxer = new Demuxer;
    videoDecoder = new VideoDecoder;
    audioDecoder = new AudioDecoder;

    demuxer->moveToThread(demuxerThread);
    videoDecoder->moveToThread(videoThread);
    audioDecoder->moveToThread(audioThread);

    m_thumbnailExtractor.reset(new ThumbnailsExtractor(this));

    connect(demuxerThread, &QThread::finished, demuxer, &QObject::deleteLater);
    connect(videoThread, &QThread::finished, videoDecoder, &QObject::deleteLater);
    connect(audioThread, &QThread::finished, audioDecoder, &QObject::deleteLater);

    connect(demuxer, &Demuxer::streamsReady, this, &MediaPlayer::onStreamsReady);
    connect(demuxer, &Demuxer::finished, this, &MediaPlayer::onDemuxerFinished);
    connect(videoDecoder, &Decoder::finished, this, &MediaPlayer::onDecoderFinished);
    connect(audioDecoder, &Decoder::finished, this, &MediaPlayer::onDecoderFinished);
    connect(videoDecoder, &VideoDecoder::videoFrameReady, this, &MediaPlayer::onVideoFrameReady);

    m_positionTimer.setInterval(100);
    connect(&m_positionTimer, &QTimer::timeout, this, &MediaPlayer::positionChanged);

    m_drainTimer.setInterval(50);
    connect(&m_drainTimer, &QTimer::timeout, this, &MediaPlayer::onDrainTick);

    demuxerThread->start();
    videoThread->start();
    audioThread->start();
}

MediaPlayer::~MediaPlayer()
{
    demuxer->stop();
    videoDecoder->stop();
    audioDecoder->stop();
    m_audioPlayer->stop();

    demuxerThread->quit();
    videoThread->quit();
    audioThread->quit();

    demuxerThread->wait();
    videoThread->wait();
    audioThread->wait();
}

void MediaPlayer::start(const QString &file)
{
    if (file.isEmpty())
        return;

    // Something is still running: tear it down first and pick this up again
    // from finalizeStop().
    if (m_demuxerRunning || m_activeDecoders > 0 || m_playbackState != Playback::Stopped) {
        m_pendingSource = file;
        stop();
        return;
    }

    m_pendingSource.clear();
    m_stopRequested = false;
    m_finalizePending = false;
    m_decodersStarted = false;
    m_drainTicks = 0;
    m_activeDecoders = 0;
    m_demuxerRunning = true;

    videoState->reset();
    videoState->setSpeed(m_playbackRate);
    m_source = file;

    // The worker threads are idle here, so clearing their stop flags directly
    // is safe.
    demuxer->prepare();
    videoDecoder->prepare();
    audioDecoder->prepare();

    QMetaObject::invokeMethod(
        demuxer,
        [this, file] { demuxer->open(videoState, file); },
        Qt::QueuedConnection);

    setPlaybackStateInternal(Playback::Playing);
    m_positionTimer.start();

    emit sourceChanged();
    emit mediaChanged();
}

void MediaPlayer::onStreamsReady(QSharedPointer<VideoState> state)
{
    // Playback was stopped while the file was still being opened.
    if (m_stopRequested)
        return;

    if (state->hasAudio) {
        if (m_audioPlayer->configureFormat(state->audio_st->codecpar)) {
            audioDecoder->setOutput(
                m_audioPlayer->bufferDevice(),
                m_audioPlayer->sampleRate(),
                m_audioPlayer->channelCount(),
                m_audioPlayer->sampleFormat());

            audioDecoder->setTempo(m_playbackRate);
            m_audioPlayer->play();

            ++m_activeDecoders;
            QMetaObject::invokeMethod(
                audioDecoder,
                [this, state] { audioDecoder->open(state); },
                Qt::QueuedConnection);
        } else {
            // No usable output device: fall back to the free running clock.
            state->hasAudio = false;
        }
    }

    if (state->hasVideo) {
        ++m_activeDecoders;
        QMetaObject::invokeMethod(
            videoDecoder,
            [this, state] { videoDecoder->open(state); },
            Qt::QueuedConnection);

        // VideoManager lives on VideoController's worker thread.
        QMetaObject::invokeMethod(
            m_videoManager.get(),
            "extractVideoThumbnails",
            Qt::QueuedConnection,
            Q_ARG(QString, state->fileName));
    } else {
        // Nothing will decode video for this file, so the item would sit on
        // whatever played before it: put the cover art up instead, or the
        // music glyph when the file carries none.
        emit audioArtworkReady(ArtworkFrame::create(state->coverArt));
    }

    m_decodersStarted = m_activeDecoders > 0;
    if (!m_decodersStarted) {
        // Nothing can be played back from this file.
        stop();
        return;
    }

    emit durationChanged();
    emit positionChanged();
}

void MediaPlayer::stop()
{
    if (!m_demuxerRunning && m_activeDecoders == 0) {
        if (m_playbackState != Playback::Stopped) {
            m_stopRequested = true;
            finalizeStop();
        } else if (!m_pendingSource.isEmpty()) {
            const QString next = m_pendingSource;
            m_pendingSource.clear();
            start(next);
        }
        return;
    }

    m_stopRequested = true;

    // Releases whatever is blocked on the pause or on a full buffer.
    videoState->setPaused(false);
    m_audioPlayer->stop();

    demuxer->stop();
    videoDecoder->stop();
    audioDecoder->stop();
}

void MediaPlayer::onDecoderFinished()
{
    if (--m_activeDecoders > 0)
        return;

    m_activeDecoders = 0;
    m_drainTicks = 0;
    // Let the audio that is still queued play out before declaring the end.
    m_drainTimer.start();
}

void MediaPlayer::onDemuxerFinished()
{
    m_demuxerRunning = false;

    if (m_finalizePending)
        finalizeStop();
    else if (!m_decodersStarted)
        finalizeStop(); // the file could not be opened
}

void MediaPlayer::onDrainTick()
{
    if (!m_stopRequested && m_audioPlayer->hasPendingAudio() && ++m_drainTicks < 60)
        return;

    m_drainTimer.stop();
    m_finalizePending = true;

    // The demuxer idles at end of file so seeking keeps working; it only goes
    // away once playback is really over.
    if (m_demuxerRunning)
        demuxer->stop();
    else
        finalizeStop();
}

void MediaPlayer::finalizeStop()
{
    if (m_playbackState == Playback::Stopped && m_pendingSource.isEmpty())
        return;

    m_finalizePending = false;
    m_positionTimer.stop();
    m_drainTimer.stop();
    m_audioPlayer->stop();

    const bool endedNaturally = !m_stopRequested;

    videoState->reset();
    setPlaybackStateInternal(Playback::Stopped);
    emit positionChanged();
    emit resetCompleted();

    if (!m_pendingSource.isEmpty()) {
        const QString next = m_pendingSource;
        m_pendingSource.clear();
        start(next);
        return;
    }

    if (endedNaturally && m_loop && !m_source.isEmpty()) {
        //start(m_source);
        return;
    }

    if (endedNaturally)
        emit endOfMedia();
}

void MediaPlayer::pause_resume()
{
    if (m_playbackState == Playback::Stopped)
        return;

    const bool pause = m_playbackState == Playback::Playing;

    videoState->setPaused(pause);
    if (pause)
        m_audioPlayer->suspend();
    else
        m_audioPlayer->resume();

    setPlaybackStateInternal(pause ? Playback::Paused : Playback::Playing);
}

void MediaPlayer::seek(double positionMs)
{
    if (m_playbackState == Playback::Stopped)
        return;

    double target = positionMs / 1000.0;
    target = videoState->duration > 0.0 ? qBound(0.0, target, videoState->duration)
                                        : qMax(0.0, target);

    videoState->seekTarget.store(target);
    videoState->skipUntil.store(target);
    videoState->seekRequested.store(true);
    videoState->refreshFrame.store(true);

    // Re-anchor the clocks so the UI and the video decoder work off the new
    // position instead of the timestamps we are leaving behind.
    videoState->audioClock.invalidate();
    videoState->externalClock.set(target);

    // Unblocks the demuxer if it is waiting on a full queue, and throws away
    // the packets and the PCM belonging to the old position.
    videoState->videoq.flush();
    videoState->audioq.flush();
    m_audioPlayer->flush();

    emit positionChanged();
}

void MediaPlayer::seekBy(double deltaMs)
{
    if (m_playbackState == Playback::Stopped)
        return;

    seek(position() + deltaMs);
}

double MediaPlayer::playbackRate() const
{
    return m_playbackRate;
}

void MediaPlayer::setPlaybackRate(double rate)
{
    rate = qBound(0.5, rate, 4.0);
    if (qFuzzyCompare(m_playbackRate, rate))
        return;

    m_playbackRate = rate;
    videoState->setSpeed(rate);
    audioDecoder->setTempo(rate);

    emit playbackRateChanged();
}

bool MediaPlayer::loop() const
{
    return m_loop;
}

void MediaPlayer::setLoop(bool loop)
{
    if (m_loop == loop)
        return;

    m_loop = loop;
    emit loopChanged();
}

void MediaPlayer::onVideoFrameReady(AVFrame *frame)
{
    if (!frame)
        return;

    // Frames still in flight when playback stopped have nobody to display them.
    if (m_playbackState == Playback::Stopped) {
        av_frame_free(&frame);
        return;
    }

    emit videoFrameReady(frame);
}

void MediaPlayer::setPlaybackStateInternal(Playback::PlaybackState state)
{
    if (m_playbackState == state)
        return;

    m_playbackState = state;
    emit playbackStateChanged();
}

long MediaPlayer::framesCount() const
{
    return videoState ? videoState->frames_count : 0;
}

QString MediaPlayer::source() const
{
    return m_source;
}

void MediaPlayer::setSource(const QString &newSource)
{
    if (m_source == newSource)
        return;

    start(newSource);
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

double MediaPlayer::maxVolume() const
{
    return AudioPlayer::MaxVolume;
}

double MediaPlayer::duration() const
{
    return videoState ? videoState->duration * 1000.0 : 0.0;
}

Playback::PlaybackState MediaPlayer::playbackState() const
{
    return m_playbackState;
}

void MediaPlayer::setPlaybackState(const Playback::PlaybackState &newPlaybackState)
{
    if (m_playbackState == newPlaybackState)
        return;

    switch (newPlaybackState) {
    case Playback::Stopped:
        stop();
        break;
    case Playback::Playing:
        if (m_playbackState == Playback::Paused)
            pause_resume();
        break;
    case Playback::Paused:
        if (m_playbackState == Playback::Playing)
            pause_resume();
        break;
    }
}

double MediaPlayer::position() const
{
    if (!videoState || m_playbackState == Playback::Stopped)
        return 0.0;

    double clock = videoState->masterClock();
    if (std::isnan(clock))
        return 0.0;

    if (videoState->duration > 0.0)
        clock = qBound(0.0, clock, videoState->duration);

    return clock * 1000.0;
}

void MediaPlayer::setPosition(double newPosition)
{
    seek(newPosition);
}
