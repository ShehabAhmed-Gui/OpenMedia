#include "videomanager.h"
#include <thread>
#include <vector>
#include <mutex>

VideoManager::VideoManager(QSharedPointer<Settings> settings,
                           QObject *parent)
    : QObject{parent}
    , m_settings(settings)
{
    Loop savedState = static_cast<Loop>(m_settings->getSetting("Video", "loop").toInt());
    setLoopState(savedState);
}

VideoManager::~VideoManager()
{
}

VideoManager::Loop VideoManager::loopState() const
{
    return m_loopState;
}

void VideoManager::setLoopState(Loop newLoopState)
{
    if (m_loopState == newLoopState)
        return;

    m_loopState = newLoopState;
    emit loopStateChanged();

    // Update loop state in Settings
    m_settings->saveSetting("Video", "loop", static_cast<int>(newLoopState));
}
void VideoManager::extractVideoThumbnails(const QString &path)
{
    m_thumbnails.clear();
    std::mutex mutex;

    AVFormatContext *fmtCtx = nullptr;
    if (avformat_open_input(&fmtCtx, path.toUtf8().constData(), nullptr, nullptr) < 0)
        return;

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        avformat_close_input(&fmtCtx);
        return;
    }

    int videoStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (videoStreamIndex < 0) {
        avformat_close_input(&fmtCtx);
        return;
    }

    emit extractingInProgress();

    AVStream *videoStream = fmtCtx->streams[videoStreamIndex];
    double duration = fmtCtx->duration / (double)AV_TIME_BASE;

    const AVCodec *codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
    if (!codec) {
        avformat_close_input(&fmtCtx);
        return;
    }

    AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, videoStream->codecpar);
    codecCtx->thread_count = std::max(1u, std::thread::hardware_concurrency());
    codecCtx->thread_type  = FF_THREAD_FRAME;

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        avcodec_free_context(&codecCtx);
        avformat_close_input(&fmtCtx);
        return;
    }

    int thumbW = codecCtx->width / 4;
    int thumbH = codecCtx->height / 4;

    SwsContext *swsCtx = sws_getContext(
        codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
        thumbW, thumbH, AV_PIX_FMT_RGB24,
        SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
        );

    if (!swsCtx) {
        avcodec_free_context(&codecCtx);
        avformat_close_input(&fmtCtx);
        return;
    }

    // Multi-threaded extraction
    int numThreads = std::max(1u, std::thread::hardware_concurrency());
    double chunkDuration = duration / numThreads;

    std::vector<std::thread> threads;

    auto extractChunk = [&](double startTime, double endTime) {
        AVFormatContext *localFmtCtx = nullptr;
        if (avformat_open_input(&localFmtCtx, path.toUtf8().constData(), nullptr, nullptr) < 0)
            return;

        if (avformat_find_stream_info(localFmtCtx, nullptr) < 0) {
            avformat_close_input(&localFmtCtx);
            return;
        }

        AVCodecContext *localCodecCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(localCodecCtx, localFmtCtx->streams[videoStreamIndex]->codecpar);
        localCodecCtx->thread_count = 1;
        localCodecCtx->thread_type  = FF_THREAD_FRAME;
        avcodec_open2(localCodecCtx, codec, nullptr);

        AVPacket *packet = av_packet_alloc();
        AVFrame *frame = av_frame_alloc();

        SwsContext *localSwsCtx = sws_getContext(
            localCodecCtx->width, localCodecCtx->height, localCodecCtx->pix_fmt,
            thumbW, thumbH, AV_PIX_FMT_RGB24,
            SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
            );

        double t = startTime;
        while (t < endTime) {
            int64_t seekTarget = static_cast<int64_t>(t / av_q2d(videoStream->time_base));
            av_seek_frame(localFmtCtx, videoStreamIndex, seekTarget, AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(localCodecCtx);

            bool frameCaptured = false;
            int maxFrames = 30;

            while (!frameCaptured && maxFrames-- > 0 && av_read_frame(localFmtCtx, packet) >= 0) {
                if (packet->stream_index != videoStreamIndex) { av_packet_unref(packet); continue; }
                avcodec_send_packet(localCodecCtx, packet);
                while (avcodec_receive_frame(localCodecCtx, frame) >= 0) {
                    double ptsSec = (frame->best_effort_timestamp != AV_NOPTS_VALUE)
                    ? frame->best_effort_timestamp * av_q2d(videoStream->time_base)
                    : frame->pkt_dts * av_q2d(videoStream->time_base);

                    if (ptsSec >= t || !frameCaptured) {
                        QImage img(thumbW, thumbH, QImage::Format_RGB888);
                        uint8_t *dst[4] = { img.bits(), nullptr, nullptr, nullptr };
                        int dstLinesize[4] = { static_cast<int>(img.bytesPerLine()), 0, 0, 0 };

                        sws_scale(localSwsCtx, frame->data, frame->linesize, 0,
                                  localCodecCtx->height, dst, dstLinesize);

                        std::lock_guard<std::mutex> lock(mutex);
                        m_thumbnails[(quint64)(t)] = img.copy();
                        frameCaptured = true;
                        break;
                    }
                }
                av_packet_unref(packet);
            }

            t += 1.0; // next second
        }

        sws_freeContext(localSwsCtx);
        av_frame_free(&frame);
        av_packet_free(&packet);
        avcodec_free_context(&localCodecCtx);
        avformat_close_input(&localFmtCtx);
    };

    // Launch threads
    for (int i = 0; i < numThreads; ++i) {
        double start = i * chunkDuration;
        double end   = (i == numThreads - 1) ? duration : (i + 1) * chunkDuration;
        threads.emplace_back(extractChunk, start, end);
    }

    for (auto &t : threads)
        t.join();

    sws_freeContext(swsCtx);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&fmtCtx);

    qDebug() << "Extracted" << m_thumbnails.size() << "thumbnails";
    emit extractedVideoThumbnails();
}

QImage VideoManager::getVideoFrame(qint64 timestamp)
{
    QImage thumbnail = m_thumbnails[timestamp / 1000];

    if (thumbnail.isNull()) {
        qDebug() << "Could not find a frame at this timestamp";
        return QImage(QSize(250, 250), QImage::Format_RGB888);
    }

    return thumbnail;
}

