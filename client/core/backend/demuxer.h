#ifndef DEMUXER_H
#define DEMUXER_H

#include <QDebug>
#include <QImage>

#include "../defs.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class Demuxer
{
public:
    Demuxer();

    typedef enum DemuxerErrorType
    {
        NoError,
        ReadPacketError,
        MediaEOF
    } ErrorType;

    ErrorType readPacket(VideoState *state);
    void open(VideoState *state, const QString &file);
    int getVideoStreamIndex();
    int getAudioStreamIndex();

    QString typeToString(AVCodecParameters* codepar) {
        switch (codepar->codec_type) {
        case AVMEDIA_TYPE_VIDEO:
            return "Video";
            break;
        case AVMEDIA_TYPE_AUDIO:
            return "Audio";
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            return "Subtitle";
            break;
        default:
            break;
        }

        return "Unknown";
    }

private:
    bool checkStreamType(AVCodecParameters* codecpar);
    AVFormatContext *fc = nullptr;

    int st_index[AVMEDIA_TYPE_NB];
};

#endif // DEMUXER_H
