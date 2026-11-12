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

#ifndef MAP_H
#define MAP_H

#include <QQuickItem>
#include <QGeoCoordinate>
#include <QTimer>
#include <QUrl>
#include <QVector>

#include "maptype.h"
#include "tileloader.h"

class MapItem;

// A slippy map: draws the raster tiles of the active map type and defines the
// view (center and zoom level) used by the MapItem children to position
// themselves. It does not handle input, see MapCanvas.qml for that.
class Map : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate center READ center WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(qreal minimumZoomLevel READ minimumZoomLevel NOTIFY minimumZoomLevelChanged)
    Q_PROPERTY(qreal maximumZoomLevel READ maximumZoomLevel NOTIFY maximumZoomLevelChanged)
    Q_PROPERTY(QUrl providersPath READ providersPath WRITE setProvidersPath NOTIFY providersPathChanged)
    Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY userAgentChanged)
    Q_PROPERTY(QList<QObject*> supportedMapTypes READ supportedMapTypes NOTIFY supportedMapTypesChanged)
    Q_PROPERTY(MapType *activeMapType READ activeMapType WRITE setActiveMapType NOTIFY activeMapTypeChanged)

public:
    // A tile texture (or a part of it) to draw in a rectangle of the item.
    struct TileDraw {
        TileKey key;
        QRectF target;
        QRectF source; // in the unit square
    };

    explicit Map(QQuickItem *parent = nullptr);

    QGeoCoordinate center() const;
    void setCenter(const QGeoCoordinate &center);
    qreal zoomLevel() const {return m_zoom;}
    void setZoomLevel(qreal zoomLevel);
    qreal minimumZoomLevel() const;
    qreal maximumZoomLevel() const;
    QUrl providersPath() const {return m_providersPath;}
    void setProvidersPath(const QUrl &path);
    QString userAgent() const {return m_userAgent;}
    void setUserAgent(const QString &userAgent);
    QList<QObject*> supportedMapTypes() const;
    MapType *activeMapType() const {return m_activeType;}
    void setActiveMapType(MapType *type);

    // The view, in normalised Web Mercator (see mapprojection.h).
    QPointF mercatorCenter() const {return QPointF(m_centerX, m_centerY);}
    // Size in pixels of the whole world at the current zoom level.
    double worldSize() const;
    // Position in the item of a Mercator point, using the copy of the world closest to the center.
    QPointF mercatorToItem(const QPointF &point) const;
    QPointF itemToMercator(const QPointF &point) const;

    Q_INVOKABLE QGeoCoordinate toCoordinate(const QPointF &point) const;
    Q_INVOKABLE QPointF fromCoordinate(const QGeoCoordinate &coordinate) const;
    Q_INVOKABLE void pan(qreal dx, qreal dy);
    // Moves the view so that the coordinate is displayed at the given position of the item.
    Q_INVOKABLE void alignCoordinateToPoint(const QGeoCoordinate &coordinate, const QPointF &point);
    // shape is a geoshape (only its bounding rectangle is used), margins are in pixels.
    Q_INVOKABLE void fitViewportToGeoShape(const QVariant &shape, qreal margins = 10);
    // Returns false if there is nothing to fit the view to.
    Q_INVOKABLE bool fitViewportToVisibleMapItems(qreal margins = 10);
    Q_INVOKABLE MapType *mapType(const QString &key) const;
    // Topmost visible map item drawn under a position of the item, if any.
    Q_INVOKABLE MapItem *mapItemAt(const QPointF &point) const;

Q_SIGNALS:
    void centerChanged();
    void zoomLevelChanged();
    void minimumZoomLevelChanged();
    void maximumZoomLevelChanged();
    void providersPathChanged();
    void userAgentChanged();
    void supportedMapTypesChanged();
    void activeMapTypeChanged();
    // The center, the zoom level or the size of the map changed.
    void viewChanged();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void updatePolish() override;
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    // Returns whether the center or the zoom level changed, once brought within their limits.
    bool setView(double centerX, double centerY, double zoom);
    void fitToMercator(const QRectF &bounds, qreal margins);
    void loadMapTypes();
    void updateTiles();

    double m_centerX;
    double m_centerY;
    double m_zoom;
    QUrl m_providersPath;
    QString m_userAgent;
    QList<MapType *> m_mapTypes;
    MapType *m_activeType;
    TileLoader *m_loader;
    QTimer m_retryTimer;
    QVector<TileDraw> m_tiles;
};

#endif // MAP_H
