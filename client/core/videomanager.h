#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include "settings.h"
#include <QObject>
#include <QImage>
#include <QFile>

#include "thumbnailsextractor.h"

class VideoManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Loop loopState READ loopState WRITE setLoopState NOTIFY loopStateChanged FINAL)

public:
    explicit VideoManager(QSharedPointer<Settings> settings,
                          QObject *parent = nullptr);

    enum Loop {
        Disabled = 0,
        Enabled = 1
    };

    void openMediaFile(QString path);

    Loop loopState() const;
    void setLoopState(Loop newLoopState);
    void setSourceVideo(const QString &path);
    QImage getVideoFrame(qint64 timestamp);

public slots:
    void extractVideoThumbnails(const QString &path);

signals:
    void loopStateChanged();
    void frameUpdated(int timestampMs);
    void extractingInProgress();
    void extractedVideoThumbnails();
    void playMediaFile(QString path);

private:
    Loop m_loopState = Disabled;
    QSharedPointer<Settings> m_settings;
    ThumbnailsExtractor *m_thumbnailsExtractor;
};

#endif // VIDEOMANAGER_H
