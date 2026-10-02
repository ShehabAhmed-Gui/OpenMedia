#include "artworkframe.h"
#include "frameconverter.h"

#include <QDebug>
#include <QPainter>
#include <QPainterPath>

namespace {
// Cover art comes in every size; past this it is detail no window can show.
constexpr int MaxCoverSize = 1080;
// The glyph sits on a square canvas: the renderer fits the picture inside the
// item and paints the same color around it, so the seam stays invisible
// whatever shape the window has.
constexpr int GlyphCanvasSize = 1080;
constexpr double GlyphScale = 0.3;
// Theme.base and Theme.textFaint from the QML theme.
constexpr QRgb BackgroundColor = 0xFF07090C;
constexpr QRgb GlyphColor = 0xFF5C6773;
}

AVFrame *ArtworkFrame::create(const QByteArray &coverArt)
{
    QImage image = decodeCoverArt(coverArt);
    if (image.isNull())
        image = glyph();

    return toYUVFrame(image);
}

QImage ArtworkFrame::decodeCoverArt(const QByteArray &coverArt)
{
    if (coverArt.isEmpty())
        return QImage();

    QImage image;
    if (!image.loadFromData(coverArt)) {
        qWarning() << "Cannot decode the cover art:" << coverArt.size() << "bytes";
        return QImage();
    }

    if (image.width() > MaxCoverSize || image.height() > MaxCoverSize)
        image = image.scaled(MaxCoverSize, MaxCoverSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    return image;
}

QImage ArtworkFrame::glyph()
{
    QImage image(GlyphCanvasSize, GlyphCanvasSize, QImage::Format_RGB32);
    image.fill(BackgroundColor);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    // Drawn after ui/icons/svg/music_media.svg, which has a 24x24 viewbox.
    const double side = GlyphCanvasSize * GlyphScale;
    painter.translate((GlyphCanvasSize - side) / 2.0, (GlyphCanvasSize - side) / 2.0);
    painter.scale(side / 24.0, side / 24.0);

    QPainterPath stem;
    stem.moveTo(9.5, 17.5);
    stem.lineTo(9.5, 6.6);
    stem.lineTo(18.5, 4.7);
    stem.lineTo(18.5, 15.4);

    QPen pen(QColor::fromRgb(GlyphColor), 1.7);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    painter.setPen(pen);
    painter.drawPath(stem);

    painter.setBrush(QColor::fromRgb(GlyphColor));
    painter.drawEllipse(QPointF(7.0, 17.5), 2.5, 2.3);
    painter.drawEllipse(QPointF(16.0, 15.4), 2.5, 2.3);

    return image;
}

AVFrame *ArtworkFrame::toYUVFrame(const QImage &image)
{
    // Chroma is subsampled by two, so an odd side would have to be padded.
    const QImage source = image.convertToFormat(QImage::Format_RGBA8888)
                              .copy(0, 0, image.width() & ~1, image.height() & ~1);

    if (source.isNull() || source.width() <= 0 || source.height() <= 0)
        return nullptr;

    AVFrame *rgb = av_frame_alloc();
    AVFrame *yuv = av_frame_alloc();

    if (!rgb || !yuv) {
        av_frame_free(&rgb);
        av_frame_free(&yuv);
        return nullptr;
    }

    // Borrows the pixels of the image: the frame owns no buffer of its own, so
    // freeing it releases nothing that is still in use.
    rgb->format = AV_PIX_FMT_RGBA;
    rgb->width = source.width();
    rgb->height = source.height();
    rgb->data[0] = const_cast<uint8_t *>(source.constBits());
    rgb->linesize[0] = static_cast<int>(source.bytesPerLine());

    FrameConverter converter;
    const bool converted = converter.toYUV420P(rgb, yuv);

    av_frame_free(&rgb);

    if (!converted) {
        av_frame_free(&yuv);
        return nullptr;
    }

    return yuv;
}
