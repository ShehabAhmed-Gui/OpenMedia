#ifndef DECODER_H
#define DECODER_H

#include <QDebug>
#include <QObject>
#include <QSharedPointer>

#include <atomic>

#include "../defs.h"
#include "playbacklogger.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

// Shared decode loop: pulls packets from a queue, feeds the codec and hands
// every decoded frame to the subclass. Handles seek serials and end of stream.
class Decoder : public QObject
{
    Q_OBJECT
public:
    explicit Decoder(QObject *parent = nullptr);
    ~Decoder() override;

    // Thread safe: the decode loop owns its thread, so stopping cannot go
    // through the event loop.
    void stop();
    // Clears a pending stop before a new file is opened. Only call it while
    // the decoder thread is idle.
    void prepare();

signals:
    void finished();

protected:
    bool openCodec(AVStream *stream, int threadCount);
    void closeCodec();
    void run(PacketQueue &queue);

    virtual void processFrame(AVFrame *frame) = 0;
    // Called after a seek, once the codec buffers have been flushed.
    virtual void onFlush() {}

    int serial() const { return m_serial; }

    QSharedPointer<VideoState> m_videoState;
    AVCodecContext *m_ctx = nullptr;
    AVFrame *m_frame = nullptr;
    std::atomic_bool m_running{ false };
    std::atomic_bool m_stopRequested{ false };

private:
    void decodePacket(AVPacket *pkt);

    // Set while the loop runs so stop() can unblock it from another thread.
    std::atomic<PacketQueue *> m_queue{ nullptr };
    int m_serial = -1;
};

#endif // DECODER_H
