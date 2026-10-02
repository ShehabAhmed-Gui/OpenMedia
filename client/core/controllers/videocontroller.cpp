#include "videocontroller.h"

VideoController::VideoController(QSharedPointer<VideoManager> videoManager,
                                 QObject *parent)
    : QObject{parent}
    , m_videoManager(videoManager)
{
    connect(m_videoManager.get(), &VideoManager::frameUpdated, this, &VideoController::frameUpdated);
    connect(m_videoManager.get(), &VideoManager::extractingInProgress, this, &VideoController::extractingInProgress);
    connect(m_videoManager.get(), &VideoManager::extractedVideoThumbnails, this, &VideoController::extractedVideoThumbnails);
    connect(m_videoManager.get(), &VideoManager::playMediaFile, this, &VideoController::playMediaFile);
    connect(m_videoManager.get(), &VideoManager::loopChanged, this, &VideoController::loopChanged);

    workerThread = new QThread(this);
    m_videoManager->moveToThread(workerThread);
    workerThread->start();
}

VideoController::~VideoController()
{
    workerThread->quit();
    workerThread->wait();
}

bool VideoController::loop() const
{
    return m_videoManager->loop();
}

void VideoController::setLoop(bool newLoop)
{
    m_videoManager->setLoop(newLoop);
}

void VideoController::extractVideoThumbnails(const QString &path)
{
    // Skip music files
    if (path.endsWith(".mp3")) {
        return;
    }

    QMetaObject::invokeMethod(
        m_videoManager.get(),
        "extractVideoThumbnails",
        Q_ARG(QString, path)
    );
}

QImage VideoController::getThumbnailAtTimestamp(quint64 timestamp)
{
    return m_videoManager->getThumbnailAtTimestamp(timestamp);
}
