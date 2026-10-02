#include "videorenderer.h"

#include <QOpenGLFramebufferObject>

#define VERTEX_SRC ":/ui/shaders/yuv_core.vert"
#define FRAGMENT_SRC ":/ui/shaders/yuv_core.frag"

namespace {
// Theme.base: what a fitted picture is surrounded with, so the artwork canvas
// and the area around it read as one background.
constexpr float BackgroundR = 7.0f / 255.0f;
constexpr float BackgroundG = 9.0f / 255.0f;
constexpr float BackgroundB = 12.0f / 255.0f;
}

VideoRenderer::VideoRenderer()
{
}

VideoRenderer::~VideoRenderer()
{
    av_frame_free(&m_frame);

    if (!m_initialized)
        return;

    glDeleteTextures(1, &m_texY);
    glDeleteTextures(1, &m_texU);
    glDeleteTextures(1, &m_texV);
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void VideoRenderer::initialize()
{
    initializeOpenGLFunctions();

    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, VERTEX_SRC);
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, FRAGMENT_SRC);

    if (!m_program.link())
        qFatal("Shader link failed: %s", m_program.log().toUtf8().constData());

    // Qt composites the framebuffer with its first row at the top, so the
    // texture is sampled in the same order the frame planes are uploaded.
    const float vertices[] = {
        // position   // texcoord
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          reinterpret_cast<void *>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glGenTextures(1, &m_texY);
    glGenTextures(1, &m_texU);
    glGenTextures(1, &m_texV);

    for (GLuint texture : { m_texY, m_texU, m_texV }) {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    m_initialized = true;
}

void VideoRenderer::render()
{
    if (!m_initialized)
        initialize();

    QOpenGLFramebufferObject *fbo = framebufferObject();
    const int fboWidth = fbo->width();
    const int fboHeight = fbo->height();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glViewport(0, 0, fboWidth, fboHeight);

    if (m_frame && m_fitInside)
        glClearColor(BackgroundR, BackgroundG, BackgroundB, 1.0f);
    else
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    if (!m_frame || m_frame->width <= 0 || m_frame->height <= 0)
        return;

    const int width = m_frame->width;
    const int height = m_frame->height;
    const int chromaWidth = (width + 1) / 2;
    const int chromaHeight = (height + 1) / 2;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glActiveTexture(GL_TEXTURE0);
    uploadPlane(m_texY, m_frame->data[0], m_frame->linesize[0], width, height, m_sizeY);
    glActiveTexture(GL_TEXTURE1);
    uploadPlane(m_texU, m_frame->data[1], m_frame->linesize[1], chromaWidth, chromaHeight, m_sizeU);
    glActiveTexture(GL_TEXTURE2);
    uploadPlane(m_texV, m_frame->data[2], m_frame->linesize[2], chromaWidth, chromaHeight, m_sizeV);

    double aspect = double(width) / double(height);
    if (m_frame->sample_aspect_ratio.num > 0 && m_frame->sample_aspect_ratio.den > 0)
        aspect *= av_q2d(m_frame->sample_aspect_ratio);

    // Video is scaled until it covers the whole item rather than fitting inside
    // it: the overflowing edge lands outside the framebuffer and is clipped, so
    // the aspect ratio is kept without leaving bars. Artwork is fitted the
    // other way round, so none of the picture is cut off.
    int viewWidth = fboWidth;
    int viewHeight = qRound(fboWidth / aspect);
    if (m_fitInside ? viewHeight > fboHeight : viewHeight < fboHeight) {
        viewHeight = fboHeight;
        viewWidth = qRound(fboHeight * aspect);
    }

    glViewport((fboWidth - viewWidth) / 2, (fboHeight - viewHeight) / 2, viewWidth, viewHeight);

    updateColorConversion();

    m_program.bind();
    m_program.setUniformValue("y_tex", 0);
    m_program.setUniformValue("u_tex", 1);
    m_program.setUniformValue("v_tex", 2);
    m_program.setUniformValue("colorMatrix", m_colorMatrix);
    m_program.setUniformValue("colorOffset", m_colorOffset);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    m_program.release();
}

void VideoRenderer::uploadPlane(GLuint texture, const uint8_t *src, int linesize,
                                int width, int height, QSize &cached)
{
    if (!src || linesize <= 0)
        return;

    glBindTexture(GL_TEXTURE_2D, texture);
    // Planes are padded for alignment, so the row stride is not the width.
    glPixelStorei(GL_UNPACK_ROW_LENGTH, linesize);

    if (cached != QSize(width, height)) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, src);
        cached = QSize(width, height);
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, src);
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

void VideoRenderer::updateColorConversion()
{
    const int colorspace = m_frame->colorspace;
    const int range = m_frame->color_range;

    if (colorspace == m_colorspace && range == m_colorRange)
        return;

    m_colorspace = colorspace;
    m_colorRange = range;

    double kr = 0.299;  // BT.601
    double kb = 0.114;

    switch (colorspace) {
    case AVCOL_SPC_BT709:
        kr = 0.2126;
        kb = 0.0722;
        break;
    case AVCOL_SPC_BT2020_NCL:
    case AVCOL_SPC_BT2020_CL:
        kr = 0.2627;
        kb = 0.0593;
        break;
    default:
        // Rec.709 is the safe assumption for anything HD that did not tag
        // itself, BT.601 for the rest.
        if (colorspace == AVCOL_SPC_UNSPECIFIED && m_frame->height > 576) {
            kr = 0.2126;
            kb = 0.0722;
        }
        break;
    }

    const double kg = 1.0 - kr - kb;
    const bool fullRange = range == AVCOL_RANGE_JPEG
        || m_frame->format == AV_PIX_FMT_YUVJ420P;

    const double yScale = fullRange ? 1.0 : 255.0 / 219.0;
    const double cScale = fullRange ? 1.0 : 255.0 / 224.0;

    const float values[9] = {
        float(yScale), 0.0f,                                float(2.0 * (1.0 - kr) * cScale),
        float(yScale), float(-2.0 * kb * (1.0 - kb) / kg * cScale), float(-2.0 * kr * (1.0 - kr) / kg * cScale),
        float(yScale), float(2.0 * (1.0 - kb) * cScale),    0.0f
    };

    m_colorMatrix = QMatrix3x3(values);
    m_colorOffset = QVector3D(fullRange ? 0.0f : -16.0f / 255.0f, -0.5f, -0.5f);
}

QOpenGLFramebufferObject *VideoRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    format.setSamples(0);

    return new QOpenGLFramebufferObject(size, format);
}

void VideoRenderer::synchronize(QQuickFramebufferObject *item)
{
    auto *videoItem = static_cast<VideoItem *>(item);

    if (videoItem->takeClearRequest())
        av_frame_free(&m_frame);

    if (AVFrame *frame = videoItem->takePendingFrame()) {
        av_frame_free(&m_frame);
        m_frame = frame;
        m_fitInside = videoItem->isArtwork();
    }
}
