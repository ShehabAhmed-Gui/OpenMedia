#ifndef DEMUXER_H
#define DEMUXER_H

#include <QDebug>
#include <QObject>
#include <QSharedPointer>

#include <atomic>

#include "../defs.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class Demuxer : public QObject
{
    Q_OBJECT
public:
    explicit Demuxer(QObject *parent = nullptr);
    ~Demuxer();

    // Thread safe: the read loop keeps the demuxer thread busy, so stopping
    // cannot go through the event loop.
    void stop();
    // Clears a pending stop before a new file is opened. Only call it while
    // the demuxer thread is idle.
    void prepare();

public slots:
    // Opens the file, publishes the streams and then runs the read loop until
    // end of file or stop().
    void open(QSharedPointer<VideoState> videoState, const QString file);

signals:
    void finished();
    void failed(const QString &reason);
    void streamsReady(QSharedPointer<VideoState> videoState);

private:
    void run();
    bool applySeek();
    void selectStreams();
    void close();

    QSharedPointer<VideoState> m_videoState;
    AVFormatContext *m_fmtCtx = nullptr;
    std::atomic_bool m_running{ false };
    std::atomic_bool m_stopRequested{ false };
    bool m_eofReached = false;
};

#endif // DEMUXER_H
