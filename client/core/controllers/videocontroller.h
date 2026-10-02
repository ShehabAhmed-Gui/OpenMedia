#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>
#include <QSharedPointer>
#include <QThread>

#include "../videomanager.h"

class VideoController : public QObject
{
    Q_OBJECT
public:
    explicit VideoController(QSharedPointer<VideoManager> videoManager,
                             QObject *parent = nullptr);

    ~VideoController();

    Q_PROPERTY(bool loop READ loop WRITE setLoop NOTIFY loopChanged FINAL)

    Q_INVOKABLE bool loop() const;
    Q_INVOKABLE void setLoop(bool newLoop);
    Q_INVOKABLE QImage getThumbnailAtTimestamp(quint64 timestamp);

public slots:
    Q_INVOKABLE void extractVideoThumbnails(const QString &path);

signals:
    void loopChanged();
    void frameUpdated(int timestamp);
    void extractingInProgress();
    void extractedVideoThumbnails();
    void playMediaFile(QString path);

private:
    QSharedPointer<VideoManager> m_videoManager;

    QThread *workerThread;
};

#endif // VIDEOCONTROLLER_H
