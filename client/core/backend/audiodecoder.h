#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include "decoder.h"

#include "../audiobufferdevice.h"

extern "C" {
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libswresample/swresample.h>
}

class AudioDecoder : public Decoder
{
    Q_OBJECT
public:
    explicit AudioDecoder(QObject *parent = nullptr);
    ~AudioDecoder() override;

    // The decoder writes straight into the sink FIFO: routing PCM through the
    // GUI thread would add latency and lose the back pressure.
    void setOutput(AudioBufferDevice *device, int sampleRate, int channels, AVSampleFormat format);

    // Thread safe: picked up by the decode loop on the next frame.
    void setTempo(double tempo);

public slots:
    void open(QSharedPointer<VideoState> videoState);

protected:
    void processFrame(AVFrame *frame) override;
    void onFlush() override;

private:
    bool ensureResampler(const AVFrame *frame);
    // abuffer -> aformat -> atempo -> abuffersink, so speed changes keep pitch.
    bool ensureFilter(const AVFrame *frame);
    void releaseFilter();
    void emitPcm(AVFrame *frame);
    double framePts(const AVFrame *frame);
    void releaseResampler();

    AudioBufferDevice *m_device = nullptr;

    SwrContext *m_swr = nullptr;
    AVChannelLayout m_srcLayout{};
    int m_srcRate = 0;
    AVSampleFormat m_srcFormat = AV_SAMPLE_FMT_NONE;

    AVChannelLayout m_dstLayout{};
    int m_dstRate = 0;
    int m_dstChannels = 0;
    AVSampleFormat m_dstFormat = AV_SAMPLE_FMT_S16;

    AVFilterGraph *m_graph = nullptr;
    AVFilterContext *m_bufferSrc = nullptr;
    AVFilterContext *m_bufferSink = nullptr;
    AVFrame *m_filtered = nullptr;
    AVChannelLayout m_graphLayout{};
    AVSampleFormat m_graphFormat = AV_SAMPLE_FMT_NONE;
    int m_graphRate = 0;
    double m_graphTempo = 1.0;
    std::atomic<double> m_tempo{ 1.0 };

    AVRational m_timeBase{ 0, 1 };
    double m_lastPts = NAN;
    // Stream time of the next byte leaving the filter graph.
    double m_outPts = NAN;
};

#endif // AUDIODECODER_H
