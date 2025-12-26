#ifndef DEFS_H
#define DEFS_H

#include <QDateTime>
#include <qelapsedtimer.h>
#include <qimage.h>
#include <qobject.h>

#include <QLinkedList>

extern "C" {
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
}

// We use Monotonic clock to keep video synced
struct Clock {
    QElapsedTimer clock;
    int pausedStartMs = 0;
    int pausedAccumulatedMs = 0;
    int alignment_offset = 0;

    void start() {
        clock.start();
    }

    quint64 now() {
        return clock.elapsed();
    }
};

struct VideoState {
    QString fileName;
    QLinkedList<AVPacket *> videoq;
    QLinkedList<AVPacket *> audioq;

    double position = 0.0;

    int bytes_per_sample = 0;
    int64_t samples_played = 0;
    int sample_rate = 0;

    // We set it to true when we want to pause
    bool paused = false;

    int audio_st_index = -1;
    int video_st_index = -1;
    AVStream *video_st = nullptr;
    AVStream *audio_st = nullptr;
    AVFormatContext *fc = nullptr;

    Clock clock;
    double pts = -1;
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
