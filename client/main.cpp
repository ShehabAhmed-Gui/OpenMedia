#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QApplication>

#include "settings/settingsManager.h"
#include "filesManager/filesManager.h"
#include "model/listmodel.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    FilesManager *filesManager = new FilesManager(&app);
#ifdef Q_OS_LINUX
    filesManager->setupDesktopFile();
#endif

    qmlRegisterSingletonInstance<SettingsManager>("com.qt.openmedia", 1, 0, "AppSettings", new SettingsManager(&app));
    qmlRegisterSingletonInstance<FilesManager>("com.qt.openmedia", 1, 0, "AppManager", filesManager);

    qmlRegisterType<ListModel>("com.qt.openmedia", 1, 0, "MVideos");

    app.setWindowIcon(QIcon(":/images/icon.png"));

    engine.load(QUrl(QStringLiteral("qrc:/ui/qml/Main.qml")));

    // Parse args
    if (QCoreApplication::arguments().size() > 1) {
        QString videoPath = QCoreApplication::arguments().last();
        filesManager->addFile(const_cast<QString &>(videoPath));
    }

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
