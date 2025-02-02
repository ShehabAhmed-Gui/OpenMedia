#ifndef LISTMODEL_H
#define LISTMODEL_H

#include <QAbstractListModel>
#include <QObject>

#include "../filesManager/filesManager.h"


class ListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(qsizetype currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged FINAL)

public:
    ListModel(QObject *parent = 0);

    ~ListModel();

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

signals:
    void currentIndexChanged();

private:
    QVector<QString> m_data;
    FilesManager filesManager;
    SettingsManager settings;
    qsizetype m_currentIndex;
};

#endif // LISTMODEL_H
