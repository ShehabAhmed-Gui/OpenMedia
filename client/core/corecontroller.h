#ifndef CORECONTROLLER_H
#define CORECONTROLLER_H

#include <QObject>

#include <QQmlApplicationEngine>

#include "settings.h"
#include "filesmanager.h"

// Models
#include "models/playlistmodel.h"
#include "models/metadatamodel.h"

// Proxies
#include "models/metadatafilterproxymodel.h"

// Controllers
#include "controllers/settingscontroller.h"
#include "controllers/filescontroller.h"

class CoreController : public QObject
{
    Q_OBJECT
public:
    explicit CoreController(QQmlApplicationEngine *engine,
                            const QSharedPointer<Settings> settings,
                            const QSharedPointer<FilesManager> filesManager,
                            QObject *parent = nullptr);

private:
    void loadFonts();

    QQmlApplicationEngine *m_engine {};

    QSharedPointer<PlaylistModel> m_listModel;
    QSharedPointer<MetaDataModel> m_metaDataModel;

    QSharedPointer<MetaDataFilterProxyModel> m_metaDataProxyModel;

    QSharedPointer<FilesManager> m_filesManager;
    QSharedPointer<FilesController> m_filesController;

    QSharedPointer<Settings> m_settings;
    QSharedPointer<SettingsController> m_settingsController;

    void initModels();
    void initControllers();
};

#endif // CORECONTROLLER_H
