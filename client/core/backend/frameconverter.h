#ifndef FRAMECONVERTER_H
#define FRAMECONVERTER_H

#include <QByteArray>
#include <QImage>

extern "C" {
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

class FrameConverter
{
public:
    FrameConverter();

    static QByteArray rawtoPcm(AVFrame *audio);
    static QImage rawtoQImage(AVFrame *frame);
};

#endif // FRAMECONVERTER_H
