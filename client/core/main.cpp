#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QApplication>

#include "settings.h"
#include "filesmanager.h"

#include "corecontroller.h"

#include <QLoggingCategory>

using namespace std;

int main(int argc, char *argv[])
{
    // Disable all multimedia logs
    QLoggingCategory::setFilterRules("qt.multimedia.*=false");

    QApplication app(argc, argv);

    QQmlApplicationEngine *engine = new QQmlApplicationEngine();

    const QUrl url(QStringLiteral("qrc:/ui/qml/Main.qml"));

    app.setOrganizationName("OpenMedia");
    app.setApplicationName("OpenMedia");
    app.setWindowIcon(QIcon(":/ui/icons/icon.png"));

    QSharedPointer<Settings> settings;
    settings.reset(new Settings(&app));

    QSharedPointer<FilesManager> filesManager;
    filesManager.reset(new FilesManager(settings, &app));

    // Don't give videoManager a parent
    // so it can be moved to a worker thread
    QSharedPointer<VideoManager> videoManager;
    videoManager.reset(new VideoManager(settings));

#ifdef Q_OS_LINUX
    filesManager->setupDesktopFile();
#endif

    QScopedPointer<CoreController> coreController;
    coreController.reset(new CoreController(engine, settings, filesManager, videoManager));

    qmlRegisterSingletonInstance("com.qt.openmedia", 1, 0, "CoreController", coreController.get());

    engine->load(url);

    // Parse args
    if (QCoreApplication::arguments().size() > 1) {
        QString videoPath = QCoreApplication::arguments().constLast();
        videoManager->openMediaFile(const_cast<QString &>(videoPath));
    }

    if (engine->rootObjects().isEmpty())
        return -1;

    return app.exec();
}
