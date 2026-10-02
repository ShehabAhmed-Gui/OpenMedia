#include "frameconverter.h"

#include <QDebug>

FrameConverter::FrameConverter() {}

FrameConverter::~FrameConverter()
{
    sws_freeContext(m_sws);
    sws_freeContext(m_imageSws);
}

bool FrameConverter::needsConversion(const AVFrame *frame)
{
    if (!frame)
        return false;

    return frame->format != AV_PIX_FMT_YUV420P && frame->format != AV_PIX_FMT_YUVJ420P;
}

bool FrameConverter::toYUV420P(const AVFrame *src, AVFrame *dst)
{
    if (!src || !dst || src->width <= 0 || src->height <= 0)
        return false;

    m_sws = sws_getCachedContext(
        m_sws,
        src->width, src->height, static_cast<AVPixelFormat>(src->format),
        src->width, src->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!m_sws) {
        qWarning() << "Cannot convert pixel format"
                   << av_get_pix_fmt_name(static_cast<AVPixelFormat>(src->format));
        return false;
    }

    dst->format = AV_PIX_FMT_YUV420P;
    dst->width = src->width;
    dst->height = src->height;

    if (av_frame_get_buffer(dst, 32) < 0)
        return false;

    if (av_frame_copy_props(dst, src) < 0) {
        av_frame_unref(dst);
        return false;
    }

    if (sws_scale(m_sws, src->data, src->linesize, 0, src->height, dst->data, dst->linesize) <= 0) {
        av_frame_unref(dst);
        return false;
    }

    // swscale writes limited range output, and picks BT.601 coefficients when
    // the source carries no matrix of its own.
    dst->color_range = AVCOL_RANGE_MPEG;
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(static_cast<AVPixelFormat>(src->format));
    if (desc && (desc->flags & AV_PIX_FMT_FLAG_RGB))
        dst->colorspace = AVCOL_SPC_BT470BG;

    return true;
}

QImage FrameConverter::toQImage(const AVFrame *frame)
{
    if (!frame || frame->width <= 0 || frame->height <= 0)
        return QImage();

    m_imageSws = sws_getCachedContext(
        m_imageSws,
        frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
        frame->width, frame->height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!m_imageSws)
        return QImage();

    QImage image(frame->width, frame->height, QImage::Format_RGB888);
    uint8_t *dstData[4] = { image.bits(), nullptr, nullptr, nullptr };
    int dstLinesize[4] = { static_cast<int>(image.bytesPerLine()), 0, 0, 0 };

    if (sws_scale(m_imageSws, frame->data, frame->linesize, 0, frame->height, dstData, dstLinesize) <= 0)
        return QImage();

    return image;
}
