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
    Q_PROPERTY(bool loop READ loop WRITE setLoop NOTIFY loopChanged FINAL)

public:
    explicit VideoManager(QSharedPointer<Settings> settings,
                          QObject *parent = nullptr);

    void openMediaFile(QString path);

    bool loop() const;
    void setLoop(bool newLoop);
    void setSourceVideo(const QString &path);
    QImage getThumbnailAtTimestamp(qint64 timestamp);

public slots:
    void extractVideoThumbnails(const QString &path);

signals:
    void loopChanged();
    void frameUpdated(int timestampMs);
    void extractingInProgress();
    void extractedVideoThumbnails();
    void playMediaFile(QString path);

private:
    bool m_loop = false;
    QSharedPointer<Settings> m_settings;
    ThumbnailsExtractor *m_thumbnailsExtractor;
};

#endif // VIDEOMANAGER_H
