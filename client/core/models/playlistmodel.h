#ifndef LISTMODEL_H
#define LISTMODEL_H

#include <QAbstractListModel>
#include <QObject>

#include "../filesmanager.h"

class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(qsizetype currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged FINAL)

public:
    PlaylistModel(const QSharedPointer<Settings> settings,
              const QSharedPointer<FilesManager> filesManager,
              QObject *parent = 0);

    ~PlaylistModel();

enum data {
    name = Qt::UserRole,
    path = Qt::UserRole + 1
};

public:
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

public:
    Q_INVOKABLE void loadVideos();
    Q_INVOKABLE void deleteItem(const qsizetype &index);
    Q_INVOKABLE void clearPlaylist();
    Q_INVOKABLE QString getPrevious();
    Q_INVOKABLE QString getNext();

    qsizetype currentIndex() const;
    void setCurrentIndex(qsizetype newCurrentIndex);

public slots:
    void onMediaFileChanged(const QString &path);

signals:
    void currentIndexChanged();

private:
    QVector<QString> m_data;
    QSharedPointer<FilesManager> m_filesManager;
    QSharedPointer<Settings> m_settings;
    qsizetype m_currentIndex = 0;
};

#endif // LISTMODEL_H
