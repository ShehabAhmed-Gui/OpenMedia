#ifndef MEDIAWORKER_H
#define MEDIAWORKER_H

#include <QObject>
#include "demuxer.h"
#include "decoder.h"
#include "defs.h"

#include <QJsonObject>
#include <qelapsedtimer.h>

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

class MediaWorker : public QObject
{
    Q_OBJECT
public:
    explicit MediaWorker(QSharedPointer<Demuxer> demuxer,
                         QObject *parent = nullptr);

    void stop();
    void pause_resume(VideoState *state);

public slots:
    void open(VideoState *state);
    void start(VideoState *state);

signals:
    void videoFrameReady(const QImage &frame);
    void audioFrameReady(const QByteArray &pcm);

    // Used to quit the thread
    void finished();

private:
    Clock clock;
    QSharedPointer<Demuxer> m_demuxer;
    Decoder videoDecoder;
    Decoder audioDecoder;
    bool m_running = false;
};

#endif // MEDIAWORKER_H
