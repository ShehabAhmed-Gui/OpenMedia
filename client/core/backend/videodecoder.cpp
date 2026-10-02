#include "videodecoder.h"

#include <QThread>

// A seek skips ahead to the target, which can take a whole GOP.
static constexpr int MaxSeekDrops = 240;

VideoDecoder::VideoDecoder(QObject *parent)
    : Decoder(parent)
{
}

void VideoDecoder::open(QSharedPointer<VideoState> videoState)
{
    m_videoState = videoState;
    m_lastPts = NAN;
    m_droppedInRow = 0;

    if (!videoState->video_st || !openCodec(videoState->video_st, 0)) {
        emit finished();
        return;
    }

    m_timeBase = videoState->video_st->time_base;

    const AVRational fps = videoState->video_st->avg_frame_rate;
    m_frameDuration = (fps.num > 0 && fps.den > 0) ? av_q2d(AVRational{ fps.den, fps.num }) : 0.04;

    run(videoState->videoq);
}

void VideoDecoder::processFrame(AVFrame *frame)
{
    VideoState *state = m_videoState.get();
    const double pts = framePts(frame);

    // A seek lands on the preceding keyframe: decode up to the target without
    // showing anything.
    if (pts + m_frameDuration <= state->skipUntil.load()) {
        m_lastPts = pts;
        return;
    }

    // Anchor the fallback clock on the first frame of the sequence: it keeps
    // video paced while the audio clock has no reference yet.
    if (!state->externalClock.isValid())
        state->externalClock.set(pts);

    const double master = state->masterClock();
    const double diff = pts - master;
    const int maxDrops = state->refreshFrame.load() ? MaxSeekDrops : Sync::MaxConsecutiveDrops;

    // Late frame: skip presenting it so playback catches up with the audio.
    if (!std::isnan(master) && diff < -Sync::DropThreshold && m_droppedInRow < maxDrops) {
        ++m_droppedInRow;
        m_lastPts = pts;
        return;
    }
    m_droppedInRow = 0;

    if (!waitForPts(pts))
        return;

    AVFrame *out = av_frame_alloc();
    if (!out)
        return;

    if (FrameConverter::needsConversion(frame)) {
        if (!m_converter.toYUV420P(frame, out)) {
            av_frame_free(&out);
            return;
        }
    } else {
        av_frame_move_ref(out, frame);
    }

    m_lastPts = pts;
    state->refreshFrame.store(false);
    state->videoPrimed.store(true);

    emit videoFrameReady(out);
}

bool VideoDecoder::waitForPts(double pts)
{
    VideoState *state = m_videoState.get();

    while (m_running) {
        // A seek happened: this frame belongs to the previous sequence.
        if (state->videoq.serial() != serial())
            return false;

        if (state->paused.load()) {
            // Still show the frame the user seeked to while paused.
            if (state->refreshFrame.load())
                return true;
            QThread::msleep(10);
            continue;
        }

        const double master = state->masterClock();
        if (std::isnan(master))
            return true;

        const double diff = pts - master;
        if (diff <= Sync::DisplayTolerance || diff > Sync::NoSyncThreshold)
            return true;

        QThread::msleep(static_cast<unsigned long>(qBound(1.0, diff * 1000.0, 10.0)));
    }

    return false;
}

double VideoDecoder::framePts(const AVFrame *frame) const
{
    int64_t ts = frame->best_effort_timestamp;
    if (ts == AV_NOPTS_VALUE)
        ts = frame->pts;

    if (ts == AV_NOPTS_VALUE)
        return std::isnan(m_lastPts) ? 0.0 : m_lastPts + m_frameDuration;

    return ts * av_q2d(m_timeBase);
}

void VideoDecoder::onFlush()
{
    m_lastPts = NAN;
    m_droppedInRow = 0;
}
