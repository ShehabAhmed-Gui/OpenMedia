#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QApplication>
#include <QQuickWindow>
#include <QQuickStyle>

#include "settings.h"
#include "filesmanager.h"
#include "settingsloader.h"
#include "corecontroller.h"

#include "mediaplayer.h"
#include "videoitem.h"

#include <QLoggingCategory>

using namespace std;

int main(int argc, char *argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    // Disable all multimedia logs
    QLoggingCategory::setFilterRules("qt.multimedia.*=false");

    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);

    // The native Windows style refuses control customization.
    QQuickStyle::setStyle("Basic");

    QApplication app(argc, argv);

    // Decoded frames are handed over through queued connections and through QML.
    qRegisterMetaType<AVFrame *>("AVFrame*");

    QQmlApplicationEngine *engine = new QQmlApplicationEngine();

    const QUrl url(QStringLiteral("qrc:/ui/qml/Main.qml"));

    app.setOrganizationName("OpenMedia");
    app.setApplicationName("OpenMedia");
    app.setWindowIcon(QIcon(":/ui/icons/logo_tile.svg"));

    QSharedPointer<Settings> settings;
    settings.reset(new Settings(&app));

    QSharedPointer<SettingsLoader> settingsLoader;
    settingsLoader.reset(new SettingsLoader(settings, &app));

    QSharedPointer<SettingsController> settingsController;
    settingsController.reset(new SettingsController(settings, settingsLoader, &app));

    QSharedPointer<FolderMonitor> folderMonitor;
    folderMonitor.reset(new FolderMonitor(&app));

    QSharedPointer<FilesManager> filesManager;
    filesManager.reset(new FilesManager(settingsController, folderMonitor, &app));

    // Don't give videoManager a parent
    // so it can be moved to a worker thread
    QSharedPointer<VideoManager> videoManager;
    videoManager.reset(new VideoManager(settings));

#ifdef Q_OS_LINUX
    filesManager->setupDesktopFile();
#endif

    QScopedPointer<CoreController> coreController;
    coreController.reset(new CoreController(engine, settingsController, settingsLoader, filesManager, videoManager, folderMonitor));

    engine->load(url);

    // Parse args
    if (QCoreApplication::arguments().size() > 1) {
        QString videoPath = QCoreApplication::arguments().constLast();
        videoManager->openMediaFile(const_cast<QString &>(videoPath));
    }

    if (engine->rootObjects().isEmpty())
        return -1;

    const int result = app.exec();

    // The QML bindings reference the controllers, so the engine has to go first.
    delete engine;

    return result;
}
