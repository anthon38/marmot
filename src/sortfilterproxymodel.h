#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class SortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)

public:
    explicit SortFilterProxyModel(QObject *parent = nullptr);
    QString filter() const {return m_filter;}
    void setFilter(QString filter);
    Q_INVOKABLE int sourceIndex(int proxyRow);

Q_SIGNALS:
    void filterChanged();

private:
    QString m_filter;
};

#endif // SORTFILTERPROXYMODEL_H
