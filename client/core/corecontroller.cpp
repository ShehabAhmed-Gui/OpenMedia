#include "corecontroller.h"
#include <qqmlcontext.h>

#include <QFontDatabase>

CoreController::CoreController(QQmlApplicationEngine *engine,
                               const QSharedPointer<Settings> settings,
                               const QSharedPointer<FilesManager> filesManager,
                               QObject *parent)
    : QObject{parent}
    , m_settings(settings)
    , m_filesManager(filesManager)
    , m_engine(engine)
{
    loadFonts();

    initModels();
    initControllers();
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
            qWarning() << "Failed to add font:" << font;
        }
    }
}

void CoreController::initModels()
{
    m_listModel.reset(new PlaylistModel(m_settings, m_filesManager, this));
    m_engine->rootContext()->setContextProperty("PlaylistModel", m_listModel.get());

    m_metaDataModel.reset(new MetaDataModel(this));
    m_engine->rootContext()->setContextProperty("MetaDataModel", m_metaDataModel.get());

    qmlRegisterType<MetaDataFilterProxyModel>("com.qt.openmedia", 1, 0, "MetaDataFilterProxyModel");
}

void CoreController::initControllers()
{
    m_settingsController.reset(new SettingsController(m_settings, this));
    m_engine->rootContext()->setContextProperty("SettingsController", m_settingsController.get());

    m_filesController.reset(new FilesController(m_filesManager, this));
    m_engine->rootContext()->setContextProperty("FilesController", m_filesController.get());
}
