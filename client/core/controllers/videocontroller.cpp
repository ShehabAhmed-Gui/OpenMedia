#include "videocontroller.h"

VideoController::VideoController(QSharedPointer<VideoManager> videoManager,
                                 QObject *parent)
    : QObject{parent}
    , m_videoManager(videoManager)
{
    connect(m_videoManager.get(), &VideoManager::frameUpdated, this, &VideoController::frameUpdated);
    connect(m_videoManager.get(), &VideoManager::extractingInProgress, this, &VideoController::extractingInProgress);
    connect(m_videoManager.get(), &VideoManager::extractedVideoThumbnails, this, &VideoController::extractedVideoThumbnails);

    workerThread = new QThread(this);
    m_videoManager->moveToThread(workerThread);
    workerThread->start();
}

VideoController::~VideoController()
{
    workerThread->quit();
    workerThread->wait();
}

VideoManager::Loop VideoController::loopState() const
{
    return m_videoManager->loopState();
}

void VideoController::setLoopState(VideoManager::Loop newLoopState)
{
    if (loopState() == newLoopState)
        return;

    m_videoManager->setLoopState(newLoopState);
    emit loopStateChanged();
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

QImage VideoController::getVideoFrame(quint64 timestamp)
{
    return m_videoManager->getVideoFrame(timestamp);
}
