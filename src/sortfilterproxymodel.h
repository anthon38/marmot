#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class SortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit SortFilterProxyModel(QObject *parent = nullptr);
    Q_INVOKABLE int sourceIndex(int proxyRow);
};

#endif // SORTFILTERPROXYMODEL_H
