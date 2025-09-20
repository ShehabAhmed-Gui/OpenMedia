#include "metadatamodel.h"

MetaDataModel::MetaDataModel(QObject *parent)
    : QAbstractListModel{parent}
{
}

void MetaDataModel::setSubtitles(QList<QMediaMetaData> subtitles)
{
    beginResetModel();
    m_tracks.clear();
    endResetModel();

    Track subtitleOff("Off", "subtitle");
    addTrack(subtitleOff);

    for (int i = 0; i < subtitles.size(); i++) {
        QString language = subtitles[i].stringValue(QMediaMetaData::Language);

        if (language.isEmpty() || language.isNull()) {
            language = QString("Subtitle %1").arg(i + 1);
        }

        Track track(language, "subtitle");

        addTrack(track);
    }
}

void MetaDataModel::setAudioTracks(QList<QMediaMetaData> tracks)
{
    for (int i = 0; i < tracks.size(); i++) {
        QString language = tracks[i].stringValue(QMediaMetaData::Language);

        if (language.isEmpty() || language.isNull()) {
            language = QString("AudioTrack %1").arg(i + 1);
        }

        Track track(language, "audio");

        addTrack(track);
    }
}

void MetaDataModel::addTrack(const Track &track)
{
    for (const auto &metadata : std::as_const(m_tracks)) {
        if (metadata.m_language == track.m_language) {
            return;
        }
    }

    beginInsertRows(QModelIndex(), m_tracks.size(), m_tracks.size());
    m_tracks.append(track);
    endInsertRows();
}

int MetaDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_tracks.count();
}

QVariant MetaDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tracks.size())
        return QVariant();

    int row = index.row();

    const Track track = m_tracks.at(row);

    switch (role) {
    case name:
        return track.m_language;
    case type:
        return track.m_type;
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> MetaDataModel::roleNames() const
{
    return {
        { name, "name" },
        { type, "type" }
    };
}
