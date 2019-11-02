#include "sortfilterproxymodel.h"

SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void SortFilterProxyModel::setFilter(QString filter)
{
    m_filter = filter;
    setFilterFixedString(filter);
    Q_EMIT(filterChanged());
}

int SortFilterProxyModel::sourceIndex(int proxyRow)
{
    QModelIndex sourceIndex = mapToSource(index(proxyRow, 0));
    return sourceIndex.row();
}
