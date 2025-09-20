#ifndef METADATAMODEL_H
#define METADATAMODEL_H

#include <QAbstractListModel>
#include <QObject>

#include <QMediaMetaData>

class MetaDataModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit MetaDataModel(QObject *parent = nullptr);

    Q_INVOKABLE void setSubtitles(QList<QMediaMetaData> subtitles);
    Q_INVOKABLE void setAudioTracks(QList<QMediaMetaData> tracks);

    enum data {
        name = Qt::UserRole,
        type = Qt::UserRole + 1
    };

    struct Track
    {
        Track(QString language, QString type) {
            m_language = language;
            m_type = type;
        }

        QString m_language;
        QString m_type;
    };

private:
    void addTrack(const Track &track);

    QList<Track> m_tracks;

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;
};

#endif // METADATAMODEL_H
