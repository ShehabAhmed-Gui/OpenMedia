#include "videomanager.h"

VideoManager::VideoManager(QSharedPointer<Settings> settings,
                           QObject *parent)
    : QObject{parent}
    , m_settings(settings)
    , fmtCtx(nullptr)
    , codecCtx(nullptr)
    , m_frame(nullptr)
    , swsCtx(nullptr)
    , videoStreamIndex(-1)
{
    Loop savedState = static_cast<Loop>(m_settings->getSetting("Video", "loop").toInt());
    setLoopState(savedState);
}

VideoManager::~VideoManager()
{
    // TODO: Fix crash on close
    if (codecCtx) {
        avcodec_free_context(&codecCtx);
    }

    if (fmtCtx) {
        avformat_close_input(&fmtCtx);
    }

    // if (swsCtx) {
    //     sws_freeContext(swsCtx);
    // }
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

void VideoManager::setSourceVideo(const QString &path)
{
    // Close previous video if any
    if (codecCtx) {
        avcodec_free_context(&codecCtx);
        codecCtx = nullptr;
    }
    if (fmtCtx) {
        avformat_close_input(&fmtCtx);
        fmtCtx = nullptr;
    }
    if (m_frame) {
        av_frame_free(&m_frame);
        m_frame = nullptr;
    }
    if (swsCtx) {
        sws_freeContext(swsCtx);
        swsCtx = nullptr;
    }

    // Open video file
    if (avformat_open_input(&fmtCtx, path.toUtf8().data(), nullptr, nullptr) != 0) {
        qWarning() << "Failed to open video:" << path;
        return;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        qWarning() << "Failed to find stream info";
        return;
    }

    const AVCodec *codec = nullptr;

    // Find video stream
    for (unsigned int i = 0; i < fmtCtx->nb_streams; i++) {
        codec = avcodec_find_decoder(fmtCtx->streams[i]->codecpar->codec_id);
        if (codec && codec->type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (!codec) {
        qWarning() << "No video stream found";
        return;
    }

    // Allocate codec context
    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) return;

    if (avcodec_parameters_to_context(codecCtx, fmtCtx->streams[videoStreamIndex]->codecpar) < 0) {
        qWarning() << "Failed to copy codec parameters";
        return;
    }

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        qWarning() << "Failed to open codec";
        return;
    }

    // Allocate reusable AVFrame
    m_frame = av_frame_alloc();
    if (!m_frame) {
        qWarning() << "Failed to allocate AVFrame";
        return;
    }

    // Allocate SwsContext for conversion to RGB
    AVPixelFormat srcFmt = (AVPixelFormat)codecCtx->pix_fmt;
    swsCtx = sws_getContext(
        codecCtx->width,
        codecCtx->height,
        srcFmt,
        codecCtx->width,
        codecCtx->height,
        AV_PIX_FMT_RGB24,
        SWS_BILINEAR,
        nullptr, nullptr, nullptr
        );
    if (!swsCtx) {
        qWarning() << "Failed to create SwsContext";
        return;
    }
}

QImage VideoManager::readVideoFrameAt(qint64 timestampMs)
{
    if (!fmtCtx || !codecCtx || !m_frame || !swsCtx)
        return QImage();

    // Convert ms -> stream timebase
    const AVRational stream_tb = fmtCtx->streams[videoStreamIndex]->time_base;
    int64_t seek_target = av_rescale_q(timestampMs, AVRational{1, 1000}, stream_tb);

    if (av_seek_frame(fmtCtx, videoStreamIndex, seek_target, AVSEEK_FLAG_BACKWARD) < 0) {
        qWarning() << "Failed to seek frame";
        return QImage();
    }

    avcodec_flush_buffers(codecCtx);

    AVPacket packet;
    av_init_packet(&packet);

    while (true) {
        int ret = av_read_frame(fmtCtx, &packet);
        if (ret < 0) break; // EOF or error

        if (packet.stream_index == videoStreamIndex) {
            ret = avcodec_send_packet(codecCtx, &packet);
            if (ret < 0) {
                av_packet_unref(&packet);
                break;
            }

            while (avcodec_receive_frame(codecCtx, m_frame) == 0) {
                // Convert PTS to ms
                int64_t frameMs = av_rescale_q(m_frame->pts, stream_tb, AVRational{1, 1000});

                if (frameMs >= timestampMs) {
                    // Convert to QImage
                    QImage img(m_frame->width, m_frame->height, QImage::Format_RGB888);
                    uint8_t* dest[4] = { img.bits(), nullptr, nullptr, nullptr };
                    int destLinesize[4] = { int(img.bytesPerLine()), 0, 0, 0 };
                    sws_scale(swsCtx, m_frame->data, m_frame->linesize, 0, m_frame->height, dest, destLinesize);

                    av_packet_unref(&packet);
                    return img;
                }
            }
        }

        av_packet_unref(&packet);
    }

    return QImage(); // failed
}
