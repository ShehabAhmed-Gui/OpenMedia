#include "metadatafilterproxymodel.h"
#include "metadatamodel.h"

MetaDataFilterProxyModel::MetaDataFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool MetaDataFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    QString type = index.data(MetaDataModel::type).toString();

    return m_filterMetaData.isEmpty() || type == m_filterMetaData;
}

QHash<int, QByteArray> MetaDataFilterProxyModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    if (sourceModel()) {
        // Get roles from source model
        roles = sourceModel()->roleNames();
    } else {
        roles = QSortFilterProxyModel::roleNames();
    }

    return roles;
}

QString MetaDataFilterProxyModel::filterMetaData() const
{
    return m_filterMetaData;
}

void MetaDataFilterProxyModel::setFilterMetaData(const QString &newFilterMetaData)
{
    if (m_filterMetaData == newFilterMetaData)
        return;
    m_filterMetaData = newFilterMetaData;

    invalidateFilter();
    emit filterMetaDataChanged();
}
