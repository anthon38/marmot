#include "sortfilterproxymodel.h"

SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

int SortFilterProxyModel::sourceIndex(int proxyRow)
{
    QModelIndex sourceIndex = mapToSource(index(proxyRow, 0));
    return sourceIndex.row();
}
