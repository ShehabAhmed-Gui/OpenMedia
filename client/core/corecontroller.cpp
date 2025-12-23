#include "corecontroller.h"
#include <qqmlcontext.h>

#include <QFontDatabase>

CoreController::CoreController(QQmlApplicationEngine *engine,
                               const QSharedPointer<SettingsController> settingsController,
                               const QSharedPointer<SettingsLoader> settingsLoader,
                               const QSharedPointer<FilesManager> filesManager,
                               const QSharedPointer<VideoManager> videoManager,
                               const QSharedPointer<FolderMonitor> folderMonitor,
                               QObject *parent)
    : QObject{parent}
    , m_settingsLoader(settingsLoader)
    , m_settingsController(settingsController)
    , m_filesManager(filesManager)
    , m_videoManager(videoManager)
    , m_folderMonitor(folderMonitor)
    , m_engine(engine)
{
    loadFonts();

    initQmlElements();
    initModels();
    initControllers();

    m_frameProvider.reset(new FrameProvider(m_videoManager));
    m_engine->addImageProvider("framesprovider", m_frameProvider.get());
}

void CoreController::loadFonts()
{
    QVector<QString> fontPaths = {
      ":/ui/fonts/Poppins-Light.ttf",
      ":/ui/fonts/Poppins-Regular.ttf",
      ":/ui/fonts/Poppins-Medium.ttf",
      ":/ui/fonts/Poppins-SemiBold.ttf",
      ":/ui/fonts/Poppins-Bold.ttf",
      ":/ui/fonts/Poppins-ExtraBold.ttf"
    };

    for (const QString &font : fontPaths) {
        int id = QFontDatabase::addApplicationFont(font);

        if (id == -1) {
            qCritical() << "Failed to add font:" << font;
        }
    }
}

void CoreController::initModels()
{
    m_listModel.reset(new PlaylistModel(m_settingsController, m_settingsLoader, m_filesManager, m_folderMonitor, this));
    m_engine->rootContext()->setContextProperty("PlaylistModel", m_listModel.get());

    m_metaDataModel.reset(new MetaDataModel(this));
    m_engine->rootContext()->setContextProperty("MetaDataModel", m_metaDataModel.get());

    qmlRegisterType<MetaDataFilterProxyModel>("com.qt.openmedia", 1, 0, "MetaDataFilterProxyModel");
}

void CoreController::initControllers()
{
    m_engine->rootContext()->setContextProperty("SettingsController", m_settingsController.get());

    m_filesController.reset(new FilesController(m_filesManager, this));
    m_engine->rootContext()->setContextProperty("FilesController", m_filesController.get());

    m_videoController.reset(new VideoController(m_videoManager, this));
    m_engine->rootContext()->setContextProperty("VideoController", m_videoController.get());

    m_mediaPlayer.reset(new MediaPlayer(m_videoManager, this));
    m_mediaPlayerController.reset(new MediaPlayerController(m_mediaPlayer, this));

    // Register Playback namespace to QML
    qmlRegisterUncreatableType<Playback>(
        "com.qt.openmedia",
        1,
        0,
        "Playback",
        "Playback is a namespace"
        );

    m_engine->rootContext()->setContextProperty("MediaPlayerController", m_mediaPlayerController.get());
}

void CoreController::initQmlElements()
{
    qmlRegisterType<VideoItem>("com.qt.openmedia", 1, 0, "VideoItem");
}
