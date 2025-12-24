#include "mediaworker.h"
#include <qthread.h>

MediaWorker::MediaWorker(QSharedPointer<VideoController> videoController,
                         QSharedPointer<Demuxer> demuxer,
                         QObject *parent)
    : QObject{parent}
    , m_demuxer(demuxer)
    , m_videoController(videoController)
{
}

void MediaWorker::open(VideoState *state)
{
    if (state->video_st) {
        if (!videoDecoder.open(state->video_st->codecpar)) {
            qCritical() << "Failed to create video decoder";
            return;
        }
    }

    if (state->audio_st) {
        if (!audioDecoder.open(state->audio_st->codecpar)) {
            qCritical() << "Failed to create audio decoder";
            return;
        }
    }

    QMetaObject::invokeMethod(
        m_videoController.get(),
        "extractVideoThumbnails",
        Q_ARG(QString, state->fileName)
        );

    clock.start();
}

void MediaWorker::start(VideoState *state)
{
    m_running = true;
    int ret = 0;

    while(m_running) {
        while (state->paused && m_running) {
            QThread::msleep(10);
        }

        Demuxer::ErrorType error = m_demuxer->readPacket(state);
        if (error != Demuxer::NoError) {
            break;
        }

        if (!state->videoq.isEmpty()) {
            AVPacket *vp = state->videoq.takeFirst();
            if (vp) {
                QImage frame;
                videoDecoder.decode_video_frame(state, frame, vp);

                qint64 pts_ms = av_rescale_q(
                    state->pts,
                    state->video_st->time_base,
                    AVRational{1, 1000}
                    );

                if (clock.alignment_offset == 0) {
                    clock.alignment_offset =
                        clock.clock.elapsed()
                        - clock.pausedAccumulatedMs
                        - pts_ms;
                }

                qint64 now_ms =
                    clock.clock.elapsed()
                    - clock.pausedAccumulatedMs
                    - clock.alignment_offset;

                qint64 delay = pts_ms - now_ms;

                if (delay > 0 && delay < 1000) {
                    QThread::msleep(delay);
                }

                state->position = vp->pts;
                emit videoFrameReady(frame);
                av_packet_free(&vp);
            }
        }

        if (!state->audioq.isEmpty()) {
            AVPacket *ap = state->audioq.takeFirst();
            if (ap) {
                QByteArray pcm;
                audioDecoder.decode_audio_frame(pcm, ap);

                emit audioFrameReady(pcm);
                av_packet_free(&ap);
            }
        }
    }

    videoDecoder.flush();
    audioDecoder.flush();
}

void MediaWorker::stop()
{
    m_running = false;
}

void MediaWorker::pause_resume(VideoState *state)
{
    if (!state->paused) {
        state->paused = true;
        clock.pausedStartMs = clock.clock.elapsed();
    } else {
        state->paused = false;
        clock.pausedAccumulatedMs += clock.now() - clock.pausedStartMs;
    }
}
