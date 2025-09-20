#ifndef METADATAFILTERPROXYMODEL_H
#define METADATAFILTERPROXYMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>

class MetaDataFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterMetaData READ filterMetaData WRITE setFilterMetaData NOTIFY filterMetaDataChanged FINAL)

public:
    MetaDataFilterProxyModel(QObject *parent = nullptr);

    // QSortFilterProxyModel interface
    QString filterMetaData() const;
    void setFilterMetaData(const QString &newFilterMetaData);

signals:
    void filterMetaDataChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QString m_filterMetaData;
};

#endif // METADATAFILTERPROXYMODEL_H
