#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QQmlEngine>
#include <QQuickItem>
#include <QSGSimpleTextureNode>
#include <QImage>

extern "C" {
#include <libavutil/frame.h>
}

class VideoItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
public:
    VideoItem();
    // Q_INVOKABLE void updateFrame(const QByteArray &y, const QByteArray &u,
    //                              const QByteArray &v, int width, int height);
    Q_INVOKABLE void updateFrame(const QImage &frame);
    // QQuickItem interface
protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    QImage m_frame;
    QByteArray m_y, m_u, m_v;
    int m_w = 0, m_h = 0;
};

#endif // VIDEOITEM_H
