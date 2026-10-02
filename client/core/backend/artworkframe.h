#ifndef ARTWORKFRAME_H
#define ARTWORKFRAME_H

#include <QByteArray>
#include <QImage>

extern "C" {
#include <libavutil/frame.h>
}

// Still picture put on screen while a file without a video track plays: the
// cover art the file carries, or a music glyph when it carries none.
class ArtworkFrame
{
public:
    // Falls back to the glyph when coverArt is empty or cannot be decoded. The
    // caller takes ownership of the frame; null means it could not be built.
    static AVFrame *create(const QByteArray &coverArt);

private:
    static QImage decodeCoverArt(const QByteArray &coverArt);
    static QImage glyph();
    static AVFrame *toYUVFrame(const QImage &image);
};

#endif // ARTWORKFRAME_H
