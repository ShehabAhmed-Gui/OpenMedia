#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QApplication>

#include "settings.h"
#include "filesmanager.h"

#include "corecontroller.h"

#include <QThread>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine *engine = new QQmlApplicationEngine(&app);

    const QUrl url(QStringLiteral("qrc:/ui/qml/Main.qml"));

    app.setOrganizationName("OpenMedia");
    app.setApplicationName("OpenMedia");
    app.setWindowIcon(QIcon(":/images/icon.png"));

    QSharedPointer<Settings> settings;
    settings.reset(new Settings(&app));

    QSharedPointer<FilesManager> filesManager;
    filesManager.reset(new FilesManager(settings, &app));

#ifdef Q_OS_LINUX
    filesManager->setupDesktopFile();
#endif

    QScopedPointer<CoreController> coreController;
    coreController.reset(new CoreController(engine, settings, filesManager));

    engine->load(url);

    // Parse args
    if (QCoreApplication::arguments().size() > 1) {
        QString videoPath = QCoreApplication::arguments().last();
        filesManager->playFile(const_cast<QString &>(videoPath));
    }

    if (engine->rootObjects().isEmpty())
        return -1;

    return app.exec();
}
