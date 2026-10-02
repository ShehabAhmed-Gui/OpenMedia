#include "demuxer.h"
#include "playbacklogger.h"

#include <QThread>

Demuxer::Demuxer(QObject *parent)
    : QObject(parent)
{
}

Demuxer::~Demuxer()
{
    close();
}

void Demuxer::open(QSharedPointer<VideoState> videoState, const QString file)
{
    if (m_stopRequested) {
        emit finished();
        return;
    }

    // Closing frees the AVStreams the decoders point at, so the previous file
    // is only released once everything that used it has finished.
    close();

    m_videoState = videoState;

    // Roughly two seconds of video and a little less of audio: enough to ride
    // out a slow read without letting the decoders run away from the clock.
    videoState->videoq.init(150, 16 * 1024 * 1024);
    videoState->audioq.init(300, 4 * 1024 * 1024);

    int ret = avformat_open_input(&m_fmtCtx, file.toUtf8().constData(), nullptr, nullptr);
    if (ret < 0) {
        PlaybackLogger::printStringError(ret, "Failed to open media file:");
        emit failed(QStringLiteral("Cannot open %1").arg(file));
        emit finished();
        return;
    }

    ret = avformat_find_stream_info(m_fmtCtx, nullptr);
    if (ret < 0) {
        PlaybackLogger::printStringError(ret, "Failed to find stream info:");
        close();
        emit failed(QStringLiteral("No stream information in %1").arg(file));
        emit finished();
        return;
    }

    videoState->fileName = file;
    selectStreams();

    if (!videoState->hasAudio && !videoState->hasVideo) {
        close();
        emit failed(QStringLiteral("No playable stream in %1").arg(file));
        emit finished();
        return;
    }

    emit streamsReady(videoState);
    run();
}

void Demuxer::selectStreams()
{
    VideoState *state = m_videoState.get();

    const int videoIndex = av_find_best_stream(m_fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    const int audioIndex = av_find_best_stream(m_fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    if (videoIndex >= 0) {
        AVStream *stream = m_fmtCtx->streams[videoIndex];

        if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
            // Cover art is exposed as a video stream with a single packet: it
            // is not a video track, it is the picture to put on screen while
            // the audio plays.
            const AVPacket &picture = stream->attached_pic;
            if (picture.data && picture.size > 0) {
                state->coverArt = QByteArray(reinterpret_cast<const char *>(picture.data),
                                             picture.size);
            }
        } else {
            state->video_st_index = videoIndex;
            state->video_st = stream;
            state->frames_count = static_cast<int>(stream->nb_frames);
            state->hasVideo = true;
        }
    }

    if (audioIndex >= 0) {
        state->audio_st_index = audioIndex;
        state->audio_st = m_fmtCtx->streams[audioIndex];
        state->hasAudio = true;
    }

    if (m_fmtCtx->duration != AV_NOPTS_VALUE) {
        state->duration = m_fmtCtx->duration / static_cast<double>(AV_TIME_BASE);
    } else if (state->video_st && state->video_st->duration != AV_NOPTS_VALUE) {
        state->duration = state->video_st->duration * av_q2d(state->video_st->time_base);
    } else if (state->audio_st && state->audio_st->duration != AV_NOPTS_VALUE) {
        state->duration = state->audio_st->duration * av_q2d(state->audio_st->time_base);
    }

    qDebug().noquote() << "Opened" << state->fileName
                       << "duration" << state->duration << "s"
                       << "video" << state->video_st_index
                       << "audio" << state->audio_st_index;
}

void Demuxer::run()
{
    m_running = true;
    m_eofReached = false;
    VideoState *state = m_videoState.get();
    AVPacket *pkt = av_packet_alloc();

    while (m_running && pkt) {
        if (state->seekRequested.exchange(false))
            applySeek();

        // Stay alive after the last packet so seeking still works while the
        // decoders play out what they have.
        if (m_eofReached) {
            QThread::msleep(10);
            continue;
        }

        const int ret = av_read_frame(m_fmtCtx, pkt);
        if (ret < 0) {
            if (ret != AVERROR_EOF)
                PlaybackLogger::printStringError(ret, "av_read_frame failed:");
            // Let the decoders drain what is still queued and finish.
            state->videoq.setEof();
            state->audioq.setEof();
            m_eofReached = true;
            continue;
        }

        const bool isVideo = pkt->stream_index == state->video_st_index;
        const bool isAudio = pkt->stream_index == state->audio_st_index;

        if (!isVideo && !isAudio) {
            av_packet_unref(pkt);
            continue;
        }

        // Hand the reference over to the queue instead of copying the payload.
        AVPacket *queued = av_packet_alloc();
        if (!queued) {
            av_packet_unref(pkt);
            break;
        }
        av_packet_move_ref(queued, pkt);

        PacketQueue &queue = isVideo ? state->videoq : state->audioq;
        if (!queue.put(queued))
            break;
    }

    av_packet_free(&pkt);

    qDebug() << "Demuxer finished";
    emit finished();
}

bool Demuxer::applySeek()
{
    VideoState *state = m_videoState.get();

    double target = state->seekTarget.load();
    if (state->duration > 0.0)
        target = qBound(0.0, target, state->duration);
    else
        target = qMax(0.0, target);

    const int64_t ts = static_cast<int64_t>(target * AV_TIME_BASE);
    const int ret = avformat_seek_file(m_fmtCtx, -1, INT64_MIN, ts, INT64_MAX, 0);
    if (ret < 0) {
        PlaybackLogger::printStringError(ret, "Seek failed:");
        return false;
    }

    // Bumping the serial tells the decoders to throw away their buffers.
    state->videoq.flush();
    state->audioq.flush();

    state->audioClock.invalidate();
    state->externalClock.set(target);
    state->refreshFrame.store(true);
    m_eofReached = false;

    return true;
}

void Demuxer::prepare()
{
    m_stopRequested = false;
}

void Demuxer::stop()
{
    m_stopRequested = true;
    m_running = false;
    if (!m_videoState)
        return;

    // Unblocks a put() that is waiting on a full queue.
    m_videoState->videoq.abort();
    m_videoState->audioq.abort();
}

void Demuxer::close()
{
    if (m_fmtCtx)
        avformat_close_input(&m_fmtCtx);
}
