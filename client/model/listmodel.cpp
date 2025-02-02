#include "listmodel.h"
#include <qcolor.h>

ListModel::ListModel(QObject *parent) : QAbstractListModel{parent}
{
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
        for (QString &item : settings.getKeys("Playlist")) {
            const QString &playlistItem = settings.getSetting("Playlist", item).toString();
            m_data.append(playlistItem);
        }
    endInsertRows();
}

ListModel::~ListModel()
{
    // Remove playlist saved items before saving current items
    settings.removeKey("Playlist", "");
    for (int i = 0; i < m_data.size(); i++) {
        const QString &file = m_data[i];
        const QString &item = QString("Item") + QString::number(i);
        settings.saveSettings("Playlist", item, file);
    }
}

qsizetype ListModel::currentIndex = 0;

int ListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_data.count();
}

QVariant ListModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();

    if (row < 0 || row >= m_data.count()) {
        return QVariant();
    }

    //Return video name without exetenstion
    QString videoName = QFileInfo(m_data.at(row)).completeBaseName();

    switch(role) {
        case name:
            return videoName;
            break;
        case path:
            return m_data.at(row);
            break;

        default: return QVariant();
    }

    return QVariant();
}

QHash<int, QByteArray> ListModel::roleNames() const
{
    return {
        {name, "name"},
        {path, "path"}
    };
}

bool ListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::UserRole) {
        m_data[index.row()] = value.toString();

        return true;
    }

    return false;
}

void ListModel::loadVideos()
{
    currentIndex = 0;
    QVector<QString> selected = filesManager.selectFiles();

      for (const auto& item : std::as_const(selected)) {
        if (!m_data.contains(item)) {
            beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
                m_data.append(item);
            endInsertRows();
        }
    }
}

void ListModel::deleteItem(const qsizetype &index)
{
    if (index != 0  || !(index > m_data.size())) {
        beginRemoveRows(QModelIndex(), index, index);
            m_data.removeAt(index);
        endRemoveRows();
    } else { qDebug() << "Item doesn't exist"; }
}

void ListModel::clearPlaylist()
{
    beginRemoveRows (QModelIndex(), 0, m_data.size() - 1);
        m_data.clear();
    endRemoveRows();
}

QString ListModel::getPrevious(const qsizetype &index) const
{
    if (m_data.isEmpty()) {
        return QString();
    }

    if (index <= 0) {
        currentIndex = m_data.size() - 1;
    } else if (index < m_data.size()) {
        currentIndex = index - 1;
    } else {
        currentIndex = m_data.size() - 1;
    }

    return m_data.at(currentIndex);
}

QString ListModel::getNext(const qsizetype &index) const
{
    if (m_data.isEmpty()) {
        return QString();
    }

    qsizetype nextIndex = (index + 1) % m_data.size();
    return m_data.at(nextIndex);
}
