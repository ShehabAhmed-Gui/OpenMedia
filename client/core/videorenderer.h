#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H

#include <QMatrix3x3>
#include <QVector3D>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QQuickFramebufferObject>
#include <QSize>

#include "videoitem.h"

class VideoRenderer : public QQuickFramebufferObject::Renderer,
                      protected QOpenGLFunctions_3_3_Core
{
public:
    VideoRenderer();
    ~VideoRenderer() override;

    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
    void synchronize(QQuickFramebufferObject *item) override;

private:
    void initialize();
    void uploadPlane(GLuint texture, const uint8_t *src, int linesize, int width, int height, QSize &cached);
    void updateColorConversion();

    bool m_initialized = false;

    QOpenGLShaderProgram m_program;

    GLuint m_texY = 0;
    GLuint m_texU = 0;
    GLuint m_texV = 0;

    QSize m_sizeY;
    QSize m_sizeU;
    QSize m_sizeV;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;

    QMatrix3x3 m_colorMatrix;
    QVector3D m_colorOffset;
    int m_colorspace = -1;
    int m_colorRange = -1;

    AVFrame *m_frame = nullptr;
    // Set for artwork: fit the picture inside the item instead of filling it.
    bool m_fitInside = false;
};

#endif // VIDEORENDERER_H
