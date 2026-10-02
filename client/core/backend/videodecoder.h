#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include "decoder.h"
#include "frameconverter.h"

class VideoDecoder : public Decoder
{
    Q_OBJECT
public:
    explicit VideoDecoder(QObject *parent = nullptr);

public slots:
    void open(QSharedPointer<VideoState> videoState);

signals:
    // Ownership of the frame moves to the receiver.
    void videoFrameReady(AVFrame *frame);

protected:
    void processFrame(AVFrame *frame) override;
    void onFlush() override;

private:
    double framePts(const AVFrame *frame) const;
    // Holds the frame back until the master clock reaches its presentation
    // time. Returns false when the frame became obsolete meanwhile.
    bool waitForPts(double pts);

    FrameConverter m_converter;
    AVRational m_timeBase{ 0, 1 };
    double m_lastPts = NAN;
    double m_frameDuration = 0.04;
    int m_droppedInRow = 0;
};

#endif // VIDEODECODER_H
