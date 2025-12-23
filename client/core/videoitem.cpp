#include "videoitem.h"
#include <qthread.h>

VideoItem::VideoItem()
{
    setFlag(QQuickItem::ItemHasContents, true);
}

void VideoItem::updateFrame(const QImage &frame)
{
    m_frame = frame;
    update();
}

// void VideoItem::updateFrame(const QByteArray &y, const QByteArray &u,
//                             const QByteArray &v, int width, int height)
// {
//     m_w = width;
//     m_h = height;

//     m_y = y;
//     m_u = u;
//     m_v = v;

//     // Request a repaint
//     update();
// }

QSGNode *VideoItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    // if (m_y.isEmpty())
    //     return nullptr;

    // QSGGeometryNode *node = static_cast<QSGGeometryNode *>(oldNode);

    // if (!node) {
    //     node = new QSGGeometryNode();

    //     // Create geometry for a full rectangle
    //     QSGGeometry *geom = new QSGGeometry(
    //         QSGGeometry::defaultAttributes_TexturedPoint2D(), 4
    //         );
    //     geom->setDrawingMode(QSGGeometry::DrawTriangleStrip);

    //     auto *v = geom->vertexDataAsTexturedPoint2D();
    //     v[0].set(0, 0, 0, 0);
    //     v[1].set(width(), 0, 1, 0);
    //     v[2].set(0, height(), 0, 1);
    //     v[3].set(width(), height(), 1, 1);

    //     node->setGeometry(geom);
    //     node->setFlag(QSGNode::OwnsGeometry);

    //     auto *mat = new YuvMaterial();
    //     node->setMaterial(mat);
    //     node->setFlag(QSGNode::OwnsMaterial);
    // }

    // auto *mat = static_cast<YuvMaterial *>(node->material());

    // // Wrap Y plane into QImage (no RGB conversion)
    // mat->texY = window()->createTextureFromImage(
    //     QImage(
    //         reinterpret_cast<const uchar *>(m_y.constData()),
    //         m_w, m_h,
    //         QImage::Format_Grayscale8
    //         )
    //     );

    // // Wrap U plane
    // mat->texU = window()->createTextureFromImage(
    //     QImage(
    //         reinterpret_cast<const uchar *>(m_u.constData()),
    //         m_w / 2, m_h / 2,
    //         QImage::Format_Grayscale8
    //         )
    //     );

    // // Wrap V plane
    // mat->texV = window()->createTextureFromImage(
    //     QImage(
    //         reinterpret_cast<const uchar *>(m_v.constData()),
    //         m_w / 2, m_h / 2,
    //         QImage::Format_Grayscale8
    //         )
    //     );

    // node->markDirty(QSGNode::DirtyMaterial);

    // return node;

    QSGSimpleTextureNode *node =
        static_cast<QSGSimpleTextureNode *>(oldNode);

    if (!node)
        node = new QSGSimpleTextureNode();

    if (!window() || m_frame.isNull()) {
        delete node;
        return nullptr;
    }

    // Create texture from QImage
    QSGTexture *texture =
        window()->createTextureFromImage(m_frame);

    node->setTexture(texture);
    node->setOwnsTexture(true);   // Qt will delete old texture
    node->setRect(0, 0, width(), height());

    return node;
}
