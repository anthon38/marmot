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

#include "map.h"
#include "mapitem.h"
#include "mapprojection.h"

#include <QGeoRectangle>
#include <QGeoShape>
#include <QQuickWindow>
#include <QSGImageNode>
#include <QSGTexture>

#include <algorithm>
#include <cmath>

namespace {

// How many zoom levels up a loaded tile is looked for to stand in for a missing one.
const int MaxAncestorLevels = 5;
// Zoom level used to show something which has no extent.
const double PointZoomLevel = 16.0;

int floorMod(int value, int modulus)
{
    const int result = value%modulus;
    return result < 0 ? result+modulus : result;
}

// Scene graph side of the tiles: one image node per drawn tile, and the textures.
class TileLayerNode : public QSGNode
{
public:
    ~TileLayerNode() override
    {
        for (const Texture &texture : std::as_const(m_textures))
            delete texture.texture;
    }

    void sync(QQuickWindow *window, const QVector<Map::TileDraw> &tiles, const TileLoader *loader)
    {
        ++m_frame;
        QSGNode *child = firstChild();
        for (const Map::TileDraw &tile : tiles) {
            auto it = m_textures.find(tile.key);
            if (it == m_textures.end()) {
                const QImage image = loader->image(tile.key);
                if (image.isNull())
                    continue;
                QSGTexture *texture = window->createTextureFromImage(
                            image, image.hasAlphaChannel() ? QQuickWindow::CreateTextureOptions()
                                                           : QQuickWindow::TextureIsOpaque);
                it = m_textures.insert(tile.key, {texture, m_frame});
            }
            it->lastUse = m_frame;

            QSGImageNode *node;
            if (child) {
                node = static_cast<QSGImageNode *>(child);
                child = child->nextSibling();
            } else {
                node = window->createImageNode();
                node->setFlag(QSGNode::OwnedByParent);
                node->setFiltering(QSGTexture::Linear);
                appendChildNode(node);
            }
            const QSizeF size = it->texture->textureSize();
            node->setTexture(it->texture);
            node->setRect(tile.target);
            node->setSourceRect(QRectF(tile.source.x()*size.width(), tile.source.y()*size.height(),
                                       tile.source.width()*size.width(), tile.source.height()*size.height()));
        }
        while (child) {
            QSGNode *next = child->nextSibling();
            removeChildNode(child);
            delete child;
            child = next;
        }

        // Keep some of the textures which are not drawn anymore, the most recently used ones.
        const qsizetype capacity = qMax<qsizetype>(128, 2*tiles.size());
        if (m_textures.size() > capacity) {
            QVector<quint64> uses;
            uses.reserve(m_textures.size());
            for (const Texture &texture : std::as_const(m_textures))
                uses << texture.lastUse;
            std::sort(uses.begin(), uses.end());
            const quint64 oldestKept = qMin(uses.at(uses.size()-capacity), m_frame);
            for (auto it = m_textures.begin(); it != m_textures.end();) {
                if (it->lastUse < oldestKept) {
                    delete it->texture;
                    it = m_textures.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

private:
    struct Texture {
        QSGTexture *texture;
        quint64 lastUse;
    };

    QHash<TileKey, Texture> m_textures;
    quint64 m_frame = 0;
};

}

Map::Map(QQuickItem *parent)
    : QQuickItem(parent)
    , m_centerX(0.5)
    , m_centerY(0.5)
    , m_zoom(2.0)
    , m_activeType(nullptr)
    , m_loader(new TileLoader(this))
{
    setFlag(ItemHasContents, true);

    connect(m_loader, &TileLoader::tileLoaded, this, [this](const TileKey &key) {
        if (m_activeType && key.type == m_activeType->id())
            polish();
    });
    // Have another go at the tiles which could not be loaded, if they are still needed by then.
    m_retryTimer.setSingleShot(true);
    m_retryTimer.setInterval(TileLoader::RetryDelay+100);
    connect(&m_retryTimer, &QTimer::timeout, this, &QQuickItem::polish);
    connect(m_loader, &TileLoader::tileFailed, this, [this]() {
        if (!m_retryTimer.isActive())
            m_retryTimer.start();
    });

    loadMapTypes();
}

QGeoCoordinate Map::center() const
{
    return MapProjection::toCoordinate(mercatorCenter());
}

void Map::setCenter(const QGeoCoordinate &center)
{
    if (!center.isValid())
        return;
    const QPointF point = MapProjection::fromCoordinate(center);
    setView(point.x(), point.y(), m_zoom);
}

void Map::setZoomLevel(qreal zoomLevel)
{
    if (std::isnan(zoomLevel))
        return;
    setView(m_centerX, m_centerY, zoomLevel);
}

qreal Map::minimumZoomLevel() const
{
    // The world is at least as high as the map.
    return height() > MapProjection::TileSize ? std::log2(height()/MapProjection::TileSize) : 0.0;
}

qreal Map::maximumZoomLevel() const
{
    return m_activeType ? m_activeType->maximumZoomLevel() : 19.0;
}

void Map::setProvidersPath(const QUrl &path)
{
    if (path == m_providersPath)
        return;
    m_providersPath = path;
    loadMapTypes();
    Q_EMIT(providersPathChanged());
}

void Map::setUserAgent(const QString &userAgent)
{
    if (userAgent == m_userAgent)
        return;
    m_userAgent = userAgent;
    m_loader->setUserAgent(userAgent);
    Q_EMIT(userAgentChanged());
}

QList<QObject *> Map::supportedMapTypes() const
{
    QList<QObject *> types;
    for (MapType *type : m_mapTypes)
        types << type;
    return types;
}

void Map::setActiveMapType(MapType *type)
{
    if (type == m_activeType || !m_mapTypes.contains(type))
        return;
    m_activeType = type;
    m_loader->cancelAllExcept({});
    Q_EMIT(activeMapTypeChanged());
    Q_EMIT(maximumZoomLevelChanged());
    setView(m_centerX, m_centerY, m_zoom);
    polish();
}

double Map::worldSize() const
{
    return MapProjection::TileSize*std::pow(2.0, m_zoom);
}

QPointF Map::mercatorToItem(const QPointF &point) const
{
    double dx = point.x()-m_centerX;
    dx -= std::round(dx);
    const double world = worldSize();
    return QPointF(width()/2.0+dx*world, height()/2.0+(point.y()-m_centerY)*world);
}

QPointF Map::itemToMercator(const QPointF &point) const
{
    const double world = worldSize();
    return QPointF(m_centerX+(point.x()-width()/2.0)/world, m_centerY+(point.y()-height()/2.0)/world);
}

QGeoCoordinate Map::toCoordinate(const QPointF &point) const
{
    const QPointF mercator = itemToMercator(point);
    return MapProjection::toCoordinate(QPointF(mercator.x()-std::floor(mercator.x()),
                                               qBound(0.0, mercator.y(), 1.0)));
}

QPointF Map::fromCoordinate(const QGeoCoordinate &coordinate) const
{
    if (!coordinate.isValid())
        return QPointF(qQNaN(), qQNaN());
    return mercatorToItem(MapProjection::fromCoordinate(coordinate));
}

void Map::pan(qreal dx, qreal dy)
{
    const double world = worldSize();
    setView(m_centerX+dx/world, m_centerY+dy/world, m_zoom);
}

void Map::alignCoordinateToPoint(const QGeoCoordinate &coordinate, const QPointF &point)
{
    if (!coordinate.isValid())
        return;
    const QPointF mercator = MapProjection::fromCoordinate(coordinate);
    const double world = worldSize();
    setView(mercator.x()-(point.x()-width()/2.0)/world, mercator.y()-(point.y()-height()/2.0)/world, m_zoom);
}

void Map::fitViewportToGeoShape(const QVariant &shape, qreal margins)
{
    QGeoRectangle rectangle;
    if (shape.userType() == qMetaTypeId<QGeoRectangle>())
        rectangle = shape.value<QGeoRectangle>();
    else if (shape.canConvert<QGeoShape>())
        rectangle = shape.value<QGeoShape>().boundingGeoRectangle();
    if (!rectangle.isValid())
        return;

    const QPointF topLeft = MapProjection::fromCoordinate(rectangle.topLeft());
    QPointF bottomRight = MapProjection::fromCoordinate(rectangle.bottomRight());
    if (bottomRight.x() < topLeft.x()) // across the antimeridian
        bottomRight.rx() += 1.0;
    fitToMercator(QRectF(topLeft, bottomRight), margins);
}

bool Map::fitViewportToVisibleMapItems(qreal margins)
{
    QRectF bounds;
    bool found = false;
    const QList<QQuickItem *> children = childItems();
    for (QQuickItem *child : children) {
        const MapItem *item = qobject_cast<MapItem *>(child);
        QRectF itemBounds;
        if (!item || !item->isVisible() || !item->mercatorBounds(&itemBounds))
            continue;
        if (found) {
            bounds.setCoords(qMin(bounds.left(), itemBounds.left()), qMin(bounds.top(), itemBounds.top()),
                             qMax(bounds.right(), itemBounds.right()), qMax(bounds.bottom(), itemBounds.bottom()));
        } else {
            bounds = itemBounds;
            found = true;
        }
    }
    if (found)
        fitToMercator(bounds, margins);
    return found;
}

MapType *Map::mapType(const QString &key) const
{
    for (MapType *type : m_mapTypes) {
        if (type->key() == key)
            return type;
    }
    return nullptr;
}

MapItem *Map::mapItemAt(const QPointF &point) const
{
    MapItem *found = nullptr;
    const QList<QQuickItem *> children = childItems();
    for (QQuickItem *child : children) {
        MapItem *item = qobject_cast<MapItem *>(child);
        // Among siblings with the same z, the last one is drawn on top.
        if (item && item->isVisible() && (!found || item->z() >= found->z()) && item->hitTest(point))
            found = item;
    }
    return found;
}

void Map::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    if (newGeometry.size() == oldGeometry.size())
        return;
    if (newGeometry.height() != oldGeometry.height())
        Q_EMIT(minimumZoomLevelChanged());
    // The limits of the view depend on the size. Even if it stays the same, the items have to follow.
    if (!setView(m_centerX, m_centerY, m_zoom)) {
        Q_EMIT(viewChanged());
        polish();
    }
}

void Map::updatePolish()
{
    updateTiles();
}

QSGNode *Map::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto node = static_cast<TileLayerNode *>(oldNode);
    if (!node)
        node = new TileLayerNode;
    node->sync(window(), m_tiles, m_loader);
    return node;
}

bool Map::setView(double centerX, double centerY, double zoom)
{
    zoom = qBound(minimumZoomLevel(), zoom, qMax(minimumZoomLevel(), maximumZoomLevel()));

    // The world repeats itself horizontally, but the view stays between the poles.
    centerX -= std::floor(centerX);
    const double halfHeight = height()/(2.0*MapProjection::TileSize*std::pow(2.0, zoom));
    centerY = halfHeight < 0.5 ? qBound(halfHeight, centerY, 1.0-halfHeight) : 0.5;

    const bool zoomed = zoom != m_zoom;
    const bool moved = centerX != m_centerX || centerY != m_centerY;
    if (!zoomed && !moved)
        return false;
    m_centerX = centerX;
    m_centerY = centerY;
    m_zoom = zoom;

    // The items are up to date by the time the handlers of the property signals run.
    Q_EMIT(viewChanged());
    if (zoomed)
        Q_EMIT(zoomLevelChanged());
    if (moved)
        Q_EMIT(centerChanged());
    polish();
    return true;
}

void Map::fitToMercator(const QRectF &bounds, qreal margins)
{
    if (width() <= 0.0 || height() <= 0.0)
        return;
    // Margins which do not leave half of the map free are ignored.
    const double availableWidth = 2.0*margins < width()/2.0 ? width()-2.0*margins : width();
    const double availableHeight = 2.0*margins < height()/2.0 ? height()-2.0*margins : height();

    double zoom = PointZoomLevel;
    if (bounds.width() > 0.0 || bounds.height() > 0.0) {
        const double scale = qMin(bounds.width() > 0.0 ? availableWidth/bounds.width() : qInf(),
                                  bounds.height() > 0.0 ? availableHeight/bounds.height() : qInf());
        // Whole zoom levels show the tiles at their native size.
        zoom = std::floor(std::log2(scale/MapProjection::TileSize));
    }
    setView(bounds.center().x(), bounds.center().y(), zoom);
}

void Map::loadMapTypes()
{
    const QString activeKey = m_activeType ? m_activeType->key() : QString();
    const QList<MapType *> oldTypes = m_mapTypes;

    QString directory;
    if (m_providersPath.scheme() == QLatin1String("qrc"))
        directory = QLatin1Char(':')+m_providersPath.path();
    else if (m_providersPath.isLocalFile())
        directory = m_providersPath.toLocalFile();
    else
        directory = m_providersPath.toString();
    m_mapTypes = MapType::load(directory, this);

    m_activeType = mapType(activeKey);
    if (!m_activeType)
        m_activeType = m_mapTypes.first();
    for (MapType *type : oldTypes)
        type->deleteLater();
    m_loader->cancelAllExcept({});

    Q_EMIT(supportedMapTypesChanged());
    Q_EMIT(activeMapTypeChanged());
    Q_EMIT(maximumZoomLevelChanged());
    setView(m_centerX, m_centerY, m_zoom);
    polish();
}

void Map::updateTiles()
{
    m_tiles.clear();
    update();
    if (!m_activeType || width() <= 0.0 || height() <= 0.0)
        return;

    const int type = m_activeType->id();
    const int maximumZoom = m_activeType->maximumZoomLevel();
    const int zoom = qBound(0, qRound(m_zoom), maximumZoom);
    const int count = 1 << zoom;
    const double world = worldSize();
    const double tileSize = world/count;

    // Position of the top left corner of the item, in pixels of the world.
    double left = m_centerX*world-width()/2.0;
    double top = m_centerY*world-height()/2.0;
    if (qFuzzyCompare(tileSize, MapProjection::TileSize)) {
        // Tiles shown at their native size are sharper when they sit on whole pixels.
        const double ratio = window() ? window()->effectiveDevicePixelRatio() : 1.0;
        left = std::round(left*ratio)/ratio;
        top = std::round(top*ratio)/ratio;
    }

    const int firstColumn = int(std::floor(left/tileSize));
    const int lastColumn = int(std::floor((left+width())/tileSize));
    const int firstRow = qMax(0, int(std::floor(top/tileSize)));
    const int lastRow = qMin(count-1, int(std::floor((top+height())/tileSize)));

    struct Missing {
        TileKey key;
        double distance;
    };
    QVector<Missing> missing;
    QVector<TileDraw> loaded;
    QSet<TileKey> wanted;

    for (int row = firstRow; row <= lastRow; ++row) {
        for (int column = firstColumn; column <= lastColumn; ++column) {
            const int x = floorMod(column, count);
            const TileKey key{type, zoom, x, row};
            const QRectF target(column*tileSize-left, row*tileSize-top, tileSize, tileSize);
            wanted.insert(key);
            if (m_loader->contains(key)) {
                loaded << TileDraw{key, target, QRectF(0.0, 0.0, 1.0, 1.0)};
                continue;
            }
            const QPointF offset = target.center()-QPointF(width()/2.0, height()/2.0);
            missing << Missing{key, QPointF::dotProduct(offset, offset)};

            // Meanwhile, show what is already there for this area: a part of a tile
            // of a lower zoom level, or else the tiles of the next one.
            bool replaced = false;
            for (int level = 1; level <= MaxAncestorLevels && level <= zoom && !replaced; ++level) {
                const TileKey ancestor{type, zoom-level, x >> level, row >> level};
                if (!m_loader->contains(ancestor))
                    continue;
                const double fraction = 1.0/(1 << level);
                m_tiles << TileDraw{ancestor, target,
                                    QRectF((x-(ancestor.x << level))*fraction, (row-(ancestor.y << level))*fraction,
                                           fraction, fraction)};
                replaced = true;
            }
            if (replaced || zoom >= maximumZoom)
                continue;
            for (int i = 0; i < 4; ++i) {
                const TileKey child{type, zoom+1, 2*x+i%2, 2*row+i/2};
                if (m_loader->contains(child)) {
                    m_tiles << TileDraw{child,
                                        QRectF(target.x()+(i%2)*tileSize/2.0, target.y()+(i/2)*tileSize/2.0,
                                               tileSize/2.0, tileSize/2.0),
                                        QRectF(0.0, 0.0, 1.0, 1.0)};
                }
            }
        }
    }
    m_tiles += loaded;

    // Enough room for the tiles in view and the ones around, in and out.
    m_loader->setCapacity(qMax(256, 3*int(wanted.size())));
    m_loader->cancelAllExcept(wanted);
    std::sort(missing.begin(), missing.end(), [](const Missing &a, const Missing &b) {
        return a.distance < b.distance;
    });
    for (const Missing &tile : std::as_const(missing))
        m_loader->request(tile.key, m_activeType->tileUrl(tile.key.zoom, tile.key.x, tile.key.y));
}
