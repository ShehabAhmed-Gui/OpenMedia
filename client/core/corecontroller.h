#ifndef CORECONTROLLER_H
#define CORECONTROLLER_H

#include <QObject>

#include <QQmlApplicationEngine>

#include "settings.h"
#include "filesmanager.h"
#include "videomanager.h"
#include "frameprovider.h"

// Models
#include "models/playlistmodel.h"
#include "models/metadatamodel.h"

// Proxies
#include "models/metadatafilterproxymodel.h"

// Controllers
#include "controllers/settingscontroller.h"
#include "controllers/filescontroller.h"
#include "controllers/videocontroller.h"

class CoreController : public QObject
{
    Q_OBJECT
public:
    explicit CoreController(QQmlApplicationEngine *engine,
                            const QSharedPointer<Settings> settings,
                            const QSharedPointer<FilesManager> filesManager, const QSharedPointer<VideoManager> videoManager,
                            QObject *parent = nullptr);

signals:
    void videoPassedAsArg(const QString &path);

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

    QSharedPointer<FrameProvider> m_frameProvider;

    QSharedPointer<VideoManager> m_videoManager;
    QSharedPointer<VideoController> m_videoController;

    void initModels();
    void initControllers();
};

#endif // CORECONTROLLER_H
