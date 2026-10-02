#ifndef FRAMECONVERTER_H
#define FRAMECONVERTER_H

#include <QImage>

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

class FrameConverter
{
public:
    FrameConverter();
    ~FrameConverter();

    // The renderer uploads 8 bit planar YUV420 directly; anything else has to
    // go through swscale first.
    static bool needsConversion(const AVFrame *frame);

    // dst must be a freshly allocated, unreferenced frame.
    bool toYUV420P(const AVFrame *src, AVFrame *dst);

    QImage toQImage(const AVFrame *frame);

private:
    SwsContext *m_sws = nullptr;
    SwsContext *m_imageSws = nullptr;
};

#endif // FRAMECONVERTER_H
