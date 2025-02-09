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

    FilesManager *filesManager = new FilesManager();

    qmlRegisterSingletonInstance<SettingsManager>("com.qt.openmedia", 1, 0, "AppSettings", new SettingsManager(&app));
    qmlRegisterSingletonInstance<FilesManager>("com.qt.openmedia", 1, 0, "AppManager", filesManager);

    qmlRegisterType<ListModel>("com.qt.openmedia", 1, 0, "MVideos");

    app.setWindowIcon(QIcon(":/images/icon.ico"));

    engine.load(QUrl(QStringLiteral("qrc:/ui/qml/Main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
