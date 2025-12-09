#include "PlaylistModel.h"
#include <qcolor.h>

PlaylistModel::PlaylistModel(const QSharedPointer<Settings> settings,
                     const QSharedPointer<FilesManager> filesManager,
                     QObject *parent)
    : QAbstractListModel{parent}
    , m_settings(settings)
    , m_filesManager(filesManager)
{

    // TODO: implement SettingsLoader
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
    for (QString &item : m_settings->getKeys("Playlist")) {
        const QString &playlistItem = m_settings->getSetting("Playlist", item).toString();
        m_data.append(playlistItem);
    }
    endInsertRows();

    connect(m_filesManager.get(), &FilesManager::fileChanged, this, &PlaylistModel::onMediaFileChanged);
}

PlaylistModel::~PlaylistModel()
{
    // Remove old playlist saved items before saving current items
    m_settings->removeGroup("Playlist");

    for (int i = 0; i < m_data.size(); i++) {
        const QString &file = m_data[i];
        const QString &item = QString("Item") + QString::number(i);
        m_settings->saveSetting("Playlist", item, file);
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
        }
    }
}

void PlaylistModel::deleteItem(const qsizetype &index)
{
    if (index != 0  || !(index > m_data.size())) {
        beginRemoveRows(QModelIndex(), index, index);
        m_data.removeAt(index);
        endRemoveRows();
    } else { qWarning() << "Item doesn't exist"; }
}

void PlaylistModel::clearPlaylist()
{
    if (m_data.empty()) {
        return;
    }

    beginRemoveRows (QModelIndex(), 0, m_data.size() - 1);
    m_data.clear();
    endRemoveRows();

    m_currentIndex = -1;
}

QString PlaylistModel::getPrevious()
{
    if (m_data.isEmpty()) {
        return QString();
    }

    m_currentIndex = m_currentIndex == 0? m_data.size() - 1 : m_currentIndex - 1;

    return m_data.at(m_currentIndex);
}

QString PlaylistModel::getNext()
{
    if (m_data.isEmpty()) {
        return QString();
    }

    m_currentIndex = m_currentIndex == m_data.size() - 1? 0 : m_currentIndex + 1;

    return m_data.at(m_currentIndex);
}

qsizetype PlaylistModel::currentIndex() const
{
    return m_currentIndex;
}

void PlaylistModel::setCurrentIndex(qsizetype newCurrentIndex)
{
    if (m_currentIndex == newCurrentIndex)
        return;
    m_currentIndex = newCurrentIndex;
    emit currentIndexChanged();
}

void PlaylistModel::onMediaFileChanged(const QString &path)
{
    for (int i = 0; i < m_data.size(); ++i) {
        if (m_data[i] == path) {
            beginRemoveRows(QModelIndex(), i, i);
            m_data.remove(i);
            endRemoveRows();
        }
    }
}
