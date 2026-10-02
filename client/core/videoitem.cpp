#include "videoitem.h"
#include "videorenderer.h"

VideoItem::VideoItem(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
}

VideoItem::~VideoItem()
{
    av_frame_free(&m_pendingFrame);
}

void VideoItem::updateYUVFrame(AVFrame *frame)
{
    setFrame(frame, false);
}

void VideoItem::updateArtworkFrame(AVFrame *frame)
{
    setFrame(frame, true);
}

void VideoItem::setFrame(AVFrame *frame, bool artwork)
{
    if (!frame)
        return;

    // The renderer never picked up the previous frame: it is obsolete anyway.
    av_frame_free(&m_pendingFrame);

    m_pendingFrame = frame;
    m_artwork = artwork;
    // A frame to show outranks a clear that has not been picked up yet.
    m_clearRequested = false;
    update();
}

void VideoItem::clear()
{
    av_frame_free(&m_pendingFrame);
    // The frame on screen belongs to the renderer, which has to drop it itself.
    m_clearRequested = true;
    update();
}

AVFrame *VideoItem::takePendingFrame()
{
    AVFrame *frame = m_pendingFrame;
    m_pendingFrame = nullptr;
    return frame;
}

bool VideoItem::isArtwork() const
{
    return m_artwork;
}

bool VideoItem::takeClearRequest()
{
    const bool requested = m_clearRequested;
    m_clearRequested = false;
    return requested;
}

QQuickFramebufferObject::Renderer *VideoItem::createRenderer() const
{
    return new VideoRenderer();
}
