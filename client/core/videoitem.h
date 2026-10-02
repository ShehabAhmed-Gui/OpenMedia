#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QQuickFramebufferObject>

extern "C" {
#include <libavutil/frame.h>
}

class VideoItem : public QQuickFramebufferObject
{
    Q_OBJECT

public:
    explicit VideoItem(QQuickItem *parent = nullptr);
    ~VideoItem() override;

    // Takes ownership of the frame.
    Q_INVOKABLE void updateYUVFrame(AVFrame *frame);
    // Same, for the still picture that stands in for a file without a video
    // track: it is fitted inside the item rather than filling it, so a cover
    // does not get cropped by the shape of the window.
    Q_INVOKABLE void updateArtworkFrame(AVFrame *frame);
    // Drops what is on screen, so nothing of the previous file survives into
    // the next one.
    Q_INVOKABLE void clear();

    Renderer *createRenderer() const override;

    // Handed to the renderer from synchronize(), where the GUI thread is
    // blocked, so the frame is never read and freed at the same time.
    AVFrame *takePendingFrame();
    bool isArtwork() const;
    bool takeClearRequest();

private:
    void setFrame(AVFrame *frame, bool artwork);

    AVFrame *m_pendingFrame = nullptr;
    bool m_artwork = false;
    bool m_clearRequested = false;
};

#endif // VIDEOITEM_H
