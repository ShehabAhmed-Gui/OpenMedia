#include "PlaylistModel.h"
#include <qcolor.h>

PlaylistModel::PlaylistModel(const QSharedPointer<SettingsController> settingsController,
                             const QSharedPointer<SettingsLoader> settingsLoader,
                             const QSharedPointer<FilesManager> filesManager,
                             const QSharedPointer<FolderMonitor> folderMonitor,
                             QObject *parent)
    : QAbstractListModel{parent}
    , m_settingsController(settingsController)
    , m_settingsLoader(settingsLoader)
    , m_filesManager(filesManager)
    , m_folderMonitor(folderMonitor)
{
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
    for (const QString &file : m_settingsLoader->loadPlaylistSettings()) {
        m_data.append(file);
        // Add file path to be monitored
        m_folderMonitor->addPath(file);
    }
    endInsertRows();

    connect(m_filesManager.get(), &FilesManager::fileChanged, this, &PlaylistModel::onMediaFileChanged);
}

PlaylistModel::~PlaylistModel()
{
    // Remove old playlist saved items before saving current items
    m_settingsController->removeGroup("Playlist");

    for (int i = 0; i < m_data.size(); i++) {
        const QString &file = m_data[i];
        const QString &item = QString("Item") + QString::number(i);
        m_settingsController->saveSetting("Playlist", item, file);
    }
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_data.count();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();

    if (row < 0 || row >= m_data.count()) {
        return QVariant();
    }

    // Return video name without exetenstion
    QString prettyVideoName = QFileInfo(m_data.at(row)).completeBaseName();

    switch(role) {
        case name:
            return prettyVideoName;
            break;
        case path:
            return m_data.at(row);
            break;

        default: return QVariant();
    }

    return QVariant();
}

QHash<int, QByteArray> PlaylistModel::roleNames() const
{
    return {
        {name, "name"},
        {path, "path"}
    };
}

bool PlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::UserRole) {
        m_data[index.row()] = value.toString();

        return true;
    }

    return false;
}

void PlaylistModel::loadVideos()
{
    QVector<QString> selected = m_filesManager->selectFiles();
    for (const auto& item : std::as_const(selected)) {
        if (!m_data.contains(item)) {
            beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
            m_data.append(item);
            endInsertRows();
            emit countChanged();
        }
    }
}

void PlaylistModel::deleteItem(const qsizetype &index)
{
    if (index < 0 || index >= m_data.size()) {
        qWarning() << "Playlist item" << index << "does not exist";
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_data.removeAt(index);
    endRemoveRows();

    // Keep the cursor pointing at the same entry it did before.
    if (m_currentIndex > index)
        --m_currentIndex;
    m_currentIndex = qBound<qsizetype>(0, m_currentIndex, qMax<qsizetype>(0, m_data.size() - 1));

    emit countChanged();
    emit currentIndexChanged();
}

void PlaylistModel::clearPlaylist()
{
    if (m_data.empty()) {
        return;
    }

    beginRemoveRows (QModelIndex(), 0, m_data.size() - 1);
    m_data.clear();
    endRemoveRows();

    m_currentIndex = 0;
    emit countChanged();
    emit currentIndexChanged();
}

QString PlaylistModel::getPrevious()
{
    if (m_data.isEmpty()) {
        return QString();
    }

    // The cursor can be stale after items were removed.
    m_currentIndex = qBound<qsizetype>(0, m_currentIndex, m_data.size() - 1);
    m_currentIndex = m_currentIndex == 0 ? m_data.size() - 1 : m_currentIndex - 1;

    emit currentIndexChanged();
    return m_data.at(m_currentIndex);
}

QString PlaylistModel::getNext()
{
    if (m_data.isEmpty()) {
        return QString();
    }

    m_currentIndex = qBound<qsizetype>(0, m_currentIndex, m_data.size() - 1);
    m_currentIndex = m_currentIndex == m_data.size() - 1 ? 0 : m_currentIndex + 1;

    emit currentIndexChanged();
    return m_data.at(m_currentIndex);
}

void PlaylistModel::setCurrentPath(const QString &path)
{
    const qsizetype index = m_data.indexOf(path);
    if (index < 0 || index == m_currentIndex)
        return;

    m_currentIndex = index;
    emit currentIndexChanged();
}

int PlaylistModel::count() const
{
    return static_cast<int>(m_data.size());
}

qsizetype PlaylistModel::currentIndex() const
{
    return m_currentIndex;
}

void PlaylistModel::setCurrentIndex(qsizetype newCurrentIndex)
{
    newCurrentIndex = qBound<qsizetype>(0, newCurrentIndex, qMax<qsizetype>(0, m_data.size() - 1));
    if (m_currentIndex == newCurrentIndex)
        return;
    m_currentIndex = newCurrentIndex;
    emit currentIndexChanged();
}

void PlaylistModel::onMediaFileChanged(const QString &path)
{
    // Walk backwards: removing while iterating forwards skips entries.
    for (qsizetype i = m_data.size() - 1; i >= 0; --i) {
        if (m_data.at(i) == path)
            deleteItem(i);
    }
}
