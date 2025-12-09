#ifndef CORECONTROLLER_H
#define CORECONTROLLER_H

#include <QObject>

#include <QQmlApplicationEngine>

#include "settings.h"
#include "settingsloader.h"
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
                            const QSharedPointer<SettingsController> settingsController,
                            const QSharedPointer<SettingsLoader> settingsLoader,
                            const QSharedPointer<FilesManager> filesManager,
                            const QSharedPointer<VideoManager> videoManager,
                            const QSharedPointer<FolderMonitor> folderMonitor,
                            QObject *parent = nullptr);

private:
    void loadFonts();

    QQmlApplicationEngine *m_engine {};

    QSharedPointer<PlaylistModel> m_listModel;
    QSharedPointer<MetaDataModel> m_metaDataModel;

    QSharedPointer<MetaDataFilterProxyModel> m_metaDataProxyModel;

    QSharedPointer<FilesManager> m_filesManager;
    QSharedPointer<FilesController> m_filesController;

    QSharedPointer<SettingsLoader> m_settingsLoader;
    QSharedPointer<SettingsController> m_settingsController;

    QSharedPointer<FrameProvider> m_frameProvider;
    QSharedPointer<FolderMonitor> m_folderMonitor;

    QSharedPointer<VideoManager> m_videoManager;
    QSharedPointer<VideoController> m_videoController;

    void initModels();
    void initControllers();
};

#endif // CORECONTROLLER_H
