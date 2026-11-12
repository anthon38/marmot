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

#ifndef MAPITEM_H
#define MAPITEM_H

#include <QQuickItem>
#include <QGeoCoordinate>
#include <QPointer>

#include "map.h"

// Base class of the items which are positioned geographically. A map item
// belongs to the Map it is a direct child of.
class MapItem : public QQuickItem
{
    Q_OBJECT

public:
    explicit MapItem(QQuickItem *parent = nullptr);

    Map *map() const {return m_map;}
    // Extent of the item in normalised Web Mercator. Returns false if it has none.
    virtual bool mercatorBounds(QRectF *bounds) const;
    // Whether the item is drawn at this position of the map.
    virtual bool hitTest(const QPointF &mapPoint) const;

protected:
    void itemChange(ItemChange change, const ItemChangeData &value) override;
    void componentComplete() override;
    // Called when the map or its view changes.
    virtual void updateView() = 0;

private:
    void findMap();

    QPointer<Map> m_map;
};

// Keeps a point of the item (the anchor point) on a coordinate. The content
// is made of regular child items.
class MapMarker : public MapItem
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate WRITE setCoordinate NOTIFY coordinateChanged)
    Q_PROPERTY(QPointF anchorPoint READ anchorPoint WRITE setAnchorPoint NOTIFY anchorPointChanged)

public:
    explicit MapMarker(QQuickItem *parent = nullptr);

    QGeoCoordinate coordinate() const {return m_coordinate;}
    void setCoordinate(const QGeoCoordinate &coordinate);
    QPointF anchorPoint() const {return m_anchorPoint;}
    void setAnchorPoint(const QPointF &anchorPoint);
    bool mercatorBounds(QRectF *bounds) const override;

Q_SIGNALS:
    void coordinateChanged();
    void anchorPointChanged();

protected:
    void updateView() override;

private:
    QGeoCoordinate m_coordinate;
    QPointF m_anchorPoint;
};

// Covers the area between two coordinates. The content is made of regular
// child items.
class MapRectangle : public MapItem
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate topLeft READ topLeft WRITE setTopLeft NOTIFY topLeftChanged)
    Q_PROPERTY(QGeoCoordinate bottomRight READ bottomRight WRITE setBottomRight NOTIFY bottomRightChanged)

public:
    explicit MapRectangle(QQuickItem *parent = nullptr);

    QGeoCoordinate topLeft() const {return m_topLeft;}
    void setTopLeft(const QGeoCoordinate &topLeft);
    QGeoCoordinate bottomRight() const {return m_bottomRight;}
    void setBottomRight(const QGeoCoordinate &bottomRight);

Q_SIGNALS:
    void topLeftChanged();
    void bottomRightChanged();

protected:
    void updateView() override;

private:
    QGeoCoordinate m_topLeft;
    QGeoCoordinate m_bottomRight;
};

#endif // MAPITEM_H
