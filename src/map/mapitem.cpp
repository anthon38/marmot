/*************************************************************************
 *  Copyright (c) 2026 Anthony Vital <anthony.vital@gmail.com>           *
 *                                                                       *
 *  This file is part of Marmot.                                         *
 *                                                                       *
 *  Marmot is free software: you can redistribute it and/or modify       *
 *  it under the terms of the GNU General Public License as published by *
 *  the Free Software Foundation, either version 3 of the License, or    *
 *  (at your option) any later version.                                  *
 *                                                                       *
 *  Marmot is distributed in the hope that it will be useful,            *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with Marmot. If not, see <http://www.gnu.org/licenses/>.       *
 *************************************************************************/

#include "mapitem.h"
#include "map.h"
#include "mapprojection.h"

MapItem::MapItem(QQuickItem *parent)
    : QQuickItem(parent)
{
}

bool MapItem::mercatorBounds(QRectF *bounds) const
{
    Q_UNUSED(bounds)
    return false;
}

bool MapItem::hitTest(const QPointF &mapPoint) const
{
    Q_UNUSED(mapPoint)
    return false;
}

void MapItem::itemChange(ItemChange change, const ItemChangeData &value)
{
    QQuickItem::itemChange(change, value);
    if (change == ItemParentHasChanged)
        findMap();
}

void MapItem::componentComplete()
{
    QQuickItem::componentComplete();
    findMap();
    updateView();
}

void MapItem::findMap()
{
    Map *map = qobject_cast<Map *>(parentItem());
    if (map == m_map)
        return;
    if (m_map)
        disconnect(m_map, nullptr, this, nullptr);
    m_map = map;
    if (m_map)
        connect(m_map, &Map::viewChanged, this, &MapItem::updateView);
    if (isComponentComplete())
        updateView();
}


MapMarker::MapMarker(QQuickItem *parent)
    : MapItem(parent)
{
}

void MapMarker::setCoordinate(const QGeoCoordinate &coordinate)
{
    if (coordinate == m_coordinate)
        return;
    m_coordinate = coordinate;
    updateView();
    Q_EMIT(coordinateChanged());
}

void MapMarker::setAnchorPoint(const QPointF &anchorPoint)
{
    if (anchorPoint == m_anchorPoint)
        return;
    m_anchorPoint = anchorPoint;
    updateView();
    Q_EMIT(anchorPointChanged());
}

bool MapMarker::mercatorBounds(QRectF *bounds) const
{
    if (!m_coordinate.isValid())
        return false;
    *bounds = QRectF(MapProjection::fromCoordinate(m_coordinate), QSizeF(0.0, 0.0));
    return true;
}

void MapMarker::updateView()
{
    if (!map() || !m_coordinate.isValid())
        return;
    const QPointF position = map()->fromCoordinate(m_coordinate)-m_anchorPoint;
    // Whole pixels keep the content sharp.
    setPosition(QPointF(qRound(position.x()), qRound(position.y())));
}


MapRectangle::MapRectangle(QQuickItem *parent)
    : MapItem(parent)
{
}

void MapRectangle::setTopLeft(const QGeoCoordinate &topLeft)
{
    if (topLeft == m_topLeft)
        return;
    m_topLeft = topLeft;
    updateView();
    Q_EMIT(topLeftChanged());
}

void MapRectangle::setBottomRight(const QGeoCoordinate &bottomRight)
{
    if (bottomRight == m_bottomRight)
        return;
    m_bottomRight = bottomRight;
    updateView();
    Q_EMIT(bottomRightChanged());
}

void MapRectangle::updateView()
{
    if (!map() || !m_topLeft.isValid() || !m_bottomRight.isValid()) {
        setSize(QSizeF(0.0, 0.0));
        return;
    }
    const QPointF topLeft = map()->fromCoordinate(m_topLeft);
    // Measured from the top left corner, so that the rectangle keeps its shape
    // even when the two corners are closest to the center in different copies of the world.
    const QPointF size = (MapProjection::fromCoordinate(m_bottomRight)-MapProjection::fromCoordinate(m_topLeft))*map()->worldSize();
    setPosition(topLeft);
    setSize(QSizeF(qMax(0.0, size.x()), qMax(0.0, size.y())));
}
