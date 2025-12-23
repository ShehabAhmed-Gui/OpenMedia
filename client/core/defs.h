#ifndef DEFS_H
#define DEFS_H

#include <QDateTime>
#include <qimage.h>
#include <qobject.h>

#include <QLinkedList>

extern "C" {
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
}

struct VideoState {
    QString fileName;
    QLinkedList<AVPacket *> videoq;
    QLinkedList<AVPacket *> audioq;

    // We set it to true when we want to pause
    bool paused = false;

    int audio_st_index = -1;
    int video_st_index = -1;
    AVStream *video_st = nullptr;
    AVStream *audio_st = nullptr;
    AVFormatContext *fc = nullptr;

    double audio_clock = 0.0;
    double video_clock;
    double pts = -1;
    double frame_timer;
    double frame_last_delay = 40e-3;
};

struct AudioData {
    QByteArray pcm;
    int sample_rate;
    int channels;
    int bytes_per_sample;
    int nb_samples;
    AVRational time_base;
    double pts;
};

#endif // DEFS_H
