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

#include "maptrack.h"
#include "map.h"
#include "mapprojection.h"

#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGMaterial>
#include <QSGMaterialShader>

#include <cmath>
#include <cstring>
#include <functional>

namespace {

// From this zoom level on, every point of a line is drawn. Below, the points which
// would not make a visible difference are left out, and the geometry is rebuilt
// for each zoom level.
const double FullDetailZoom = 17.0;
const int FullDetail = 1000;
// How far from the full line a simplified one can be, in pixels.
const double SimplifyTolerance = 0.35;
// How far from a line or a point a position still hits it, in pixels.
const double HitTolerance = 3.0;
// The renderer works out the area covered by a node from the positions of its vertices,
// to tell if it can change the order in which nodes are drawn. It cannot know where these
// geometries end up, so two unused vertices make it assume they cover everything.
const float Everywhere = 1.0e7f;

struct LineVertex {
    float startX, startY;
    float endX, endY;
    float cornerX, cornerY;
};

struct PointVertex {
    float x, y;
    float cornerX, cornerY;
};

const QSGGeometry::AttributeSet &lineAttributes()
{
    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::createWithAttributeType(0, 2, QSGGeometry::FloatType, QSGGeometry::PositionAttribute),
        QSGGeometry::Attribute::createWithAttributeType(1, 2, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute),
        QSGGeometry::Attribute::createWithAttributeType(2, 2, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute)
    };
    static const QSGGeometry::AttributeSet set = {3, sizeof(LineVertex), attributes};
    return set;
}

const QSGGeometry::AttributeSet &pointAttributes()
{
    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::createWithAttributeType(0, 2, QSGGeometry::FloatType, QSGGeometry::PositionAttribute),
        QSGGeometry::Attribute::createWithAttributeType(1, 2, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute)
    };
    static const QSGGeometry::AttributeSet set = {2, sizeof(PointVertex), attributes};
    return set;
}

void writeColor(float *destination, const QColor &color, float opacity)
{
    const float alpha = color.alphaF()*opacity;
    destination[0] = color.redF()*alpha;
    destination[1] = color.greenF()*alpha;
    destination[2] = color.blueF()*alpha;
    destination[3] = alpha;
}

// Two triangles for each quad made of four consecutive vertices.
void writeQuadIndices(quint32 *indices, int quads)
{
    for (int i = 0; i < quads; ++i) {
        const quint32 first = 4*i;
        *indices++ = first;
        *indices++ = first+1;
        *indices++ = first+2;
        *indices++ = first+1;
        *indices++ = first+3;
        *indices++ = first+2;
    }
}

QSGGeometryNode *createNode(const QSGGeometry::AttributeSet &attributes, QSGMaterial *material)
{
    auto geometry = new QSGGeometry(attributes, 0, 0, QSGGeometry::UnsignedIntType);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    geometry->setVertexDataPattern(QSGGeometry::StaticPattern);
    geometry->setIndexDataPattern(QSGGeometry::StaticPattern);
    auto node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setMaterial(material);
    node->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);
    return node;
}

// The parameters of the shaders belong to a single node: the nodes are not batched,
// and the vertices are left as they are for the shaders to place them.
class TrackMaterial : public QSGMaterial
{
public:
    TrackMaterial()
    {
        setFlag(Blending);
        setFlag(RequiresFullMatrix);
        setFlag(NoBatching);
    }

    int compare(const QSGMaterial *other) const override
    {
        if (this == other)
            return 0;
        return std::less<const QSGMaterial *>()(this, other) ? -1 : 1;
    }

    QVector2D viewOrigin;
    float scale = 1.0f;
};

class LineMaterial : public TrackMaterial
{
public:
    QSGMaterialType *type() const override
    {
        static QSGMaterialType type;
        return &type;
    }
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override;

    QColor color;
    float halfWidth = 1.0f;
};

class LineShader : public QSGMaterialShader
{
public:
    LineShader()
    {
        setShaderFileName(VertexStage, QStringLiteral(":/shaders/line.vert.qsb"));
        setShaderFileName(FragmentStage, QStringLiteral(":/shaders/line.frag.qsb"));
    }

    bool updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *) override
    {
        // Follows the layout of the uniform block of the shaders.
        struct {
            float matrix[16];
            float color[4];
            float origin[2];
            float scale;
            float halfWidth;
        } uniforms;
        const auto material = static_cast<LineMaterial *>(newMaterial);
        const QMatrix4x4 matrix = state.combinedMatrix();
        std::memcpy(uniforms.matrix, matrix.constData(), sizeof(uniforms.matrix));
        writeColor(uniforms.color, material->color, state.opacity());
        uniforms.origin[0] = material->viewOrigin.x();
        uniforms.origin[1] = material->viewOrigin.y();
        uniforms.scale = material->scale;
        uniforms.halfWidth = material->halfWidth;

        QByteArray *buffer = state.uniformData();
        Q_ASSERT(size_t(buffer->size()) >= sizeof(uniforms));
        std::memcpy(buffer->data(), &uniforms, sizeof(uniforms));
        return true;
    }
};

QSGMaterialShader *LineMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new LineShader;
}

class PointsMaterial : public TrackMaterial
{
public:
    QSGMaterialType *type() const override
    {
        static QSGMaterialType type;
        return &type;
    }
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override;

    QColor color;
    QColor borderColor;
    QColor selectedColor;
    QColor selectedBorderColor;
    float selection[4] = {1.0f, 1.0f, -1.0f, -1.0f};
    float radius = 1.0f;
    float borderWidth = 0.0f;
};

class PointsShader : public QSGMaterialShader
{
public:
    PointsShader()
    {
        setShaderFileName(VertexStage, QStringLiteral(":/shaders/points.vert.qsb"));
        setShaderFileName(FragmentStage, QStringLiteral(":/shaders/points.frag.qsb"));
    }

    bool updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *) override
    {
        // Follows the layout of the uniform block of the shaders.
        struct {
            float matrix[16];
            float color[4];
            float borderColor[4];
            float selectedColor[4];
            float selectedBorderColor[4];
            float selection[4];
            float origin[2];
            float scale;
            float radius;
            float borderWidth;
        } uniforms;
        const auto material = static_cast<PointsMaterial *>(newMaterial);
        const QMatrix4x4 matrix = state.combinedMatrix();
        std::memcpy(uniforms.matrix, matrix.constData(), sizeof(uniforms.matrix));
        writeColor(uniforms.color, material->color, state.opacity());
        writeColor(uniforms.borderColor, material->borderColor, state.opacity());
        writeColor(uniforms.selectedColor, material->selectedColor, state.opacity());
        writeColor(uniforms.selectedBorderColor, material->selectedBorderColor, state.opacity());
        std::memcpy(uniforms.selection, material->selection, sizeof(uniforms.selection));
        uniforms.origin[0] = material->viewOrigin.x();
        uniforms.origin[1] = material->viewOrigin.y();
        uniforms.scale = material->scale;
        uniforms.radius = material->radius;
        uniforms.borderWidth = material->borderWidth;

        QByteArray *buffer = state.uniformData();
        Q_ASSERT(size_t(buffer->size()) >= sizeof(uniforms));
        std::memcpy(buffer->data(), &uniforms, sizeof(uniforms));
        return true;
    }
};

QSGMaterialShader *PointsMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new PointsShader;
}

// One quad per segment. With a tolerance (in units of the world), the points too close
// to the previous one are left out.
void buildLine(QSGGeometry *geometry, const std::vector<QPointF> &points, const QPointF &origin, double tolerance)
{
    std::vector<QVector2D> kept;
    kept.reserve(points.size());
    QPointF last;
    for (size_t i = 0; i < points.size(); ++i) {
        const QPointF &point = points[i];
        // The last point is kept no matter what, so that the line ends where the track does.
        if (!kept.empty() && i+1 < points.size()) {
            const QPointF delta = point-last;
            if (QPointF::dotProduct(delta, delta) < tolerance*tolerance)
                continue;
        }
        const QVector2D relative(float(point.x()-origin.x()), float(point.y()-origin.y()));
        if (!kept.empty() && relative == kept.back())
            continue;
        kept.push_back(relative);
        last = point;
    }

    const int segments = kept.empty() ? 0 : int(kept.size())-1;
    geometry->allocate(4*segments+2, 6*segments);
    auto vertex = static_cast<LineVertex *>(geometry->vertexData());
    for (int i = 0; i < segments; ++i) {
        const QVector2D start = kept[i];
        const QVector2D end = kept[i+1];
        for (int corner = 0; corner < 4; ++corner)
            *vertex++ = {start.x(), start.y(), end.x(), end.y(), float(corner%2), corner < 2 ? -1.0f : 1.0f};
    }
    *vertex++ = {-Everywhere, -Everywhere, 0.0f, 0.0f, 0.0f, 0.0f};
    *vertex++ = {Everywhere, Everywhere, 0.0f, 0.0f, 0.0f, 0.0f};
    writeQuadIndices(geometry->indexDataAsUInt(), segments);
}

// One quad per point.
void buildPoints(QSGGeometry *geometry, const std::vector<QPointF> &points, const QPointF &origin)
{
    const int count = int(points.size());
    geometry->allocate(4*count+2, 6*count);
    auto vertex = static_cast<PointVertex *>(geometry->vertexData());
    for (const QPointF &point : points) {
        const float x = float(point.x()-origin.x());
        const float y = float(point.y()-origin.y());
        for (int corner = 0; corner < 4; ++corner)
            *vertex++ = {x, y, corner%2 ? 1.0f : -1.0f, corner < 2 ? -1.0f : 1.0f};
    }
    *vertex++ = {-Everywhere, -Everywhere, 0.0f, 0.0f};
    *vertex++ = {Everywhere, Everywhere, 0.0f, 0.0f};
    writeQuadIndices(geometry->indexDataAsUInt(), count);
}

double squaredDistanceToSegment(const QPointF &point, const QPointF &start, const QPointF &end)
{
    const QPointF segment = end-start;
    const double lengthSquared = QPointF::dotProduct(segment, segment);
    double t = 0.0;
    if (lengthSquared > 0.0)
        t = qBound(0.0, QPointF::dotProduct(point-start, segment)/lengthSquared, 1.0);
    const QPointF delta = point-(start+t*segment);
    return QPointF::dotProduct(delta, delta);
}

}

MapTrackItem::MapTrackItem(QQuickItem *parent)
    : MapItem(parent)
    , m_revision(0)
{
    setFlag(ItemHasContents, true);
}

void MapTrackItem::setTrack(Track *track)
{
    if (track == m_track)
        return;
    if (m_track)
        disconnect(m_track, nullptr, this, nullptr);
    m_track = track;
    if (m_track) {
        connect(m_track, &Track::pathChanged, this, &MapTrackItem::project);
        connect(m_track, &QObject::destroyed, this, &MapTrackItem::project);
    }
    project();
    Q_EMIT(trackChanged());
}

bool MapTrackItem::mercatorBounds(QRectF *bounds) const
{
    if (m_points.empty())
        return false;
    *bounds = m_bounds;
    return true;
}

void MapTrackItem::updateView()
{
    if (map())
        setSize(map()->size());
    update();
}

QPointF MapTrackItem::mapToMercator(const QPointF &mapPoint) const
{
    QPointF point = map()->itemToMercator(mapPoint);
    point.rx() += std::round(origin().x()-point.x());
    return point;
}

bool MapTrackItem::viewTransform(qreal margin, QVector2D *viewOrigin, float *scale) const
{
    if (!map() || m_points.empty())
        return false;
    const double world = map()->worldSize();
    const QPointF center = map()->mercatorCenter();
    // The track is shown in the copy of the world closest to the center of the view.
    double dx = origin().x()-center.x();
    dx -= std::round(dx);
    const double dy = origin().y()-center.y();
    const double halfWidth = map()->width()/(2.0*world);
    const double halfHeight = map()->height()/(2.0*world);
    if (std::abs(dx) > halfWidth+m_bounds.width()/2.0+margin/world
            || std::abs(dy) > halfHeight+m_bounds.height()/2.0+margin/world) {
        return false;
    }
    *viewOrigin = QVector2D(float(-dx-halfWidth), float(-dy-halfHeight));
    *scale = float(world);
    return true;
}

void MapTrackItem::project()
{
    m_points.clear();
    m_bounds = QRectF();
    if (m_track) {
        const QVector<QGeoCoordinate> path = m_track->path();
        m_points.reserve(path.size());
        double left = qInf(), top = qInf(), right = -qInf(), bottom = -qInf();
        for (const QGeoCoordinate &coordinate : path) {
            const QPointF point = MapProjection::fromCoordinate(coordinate);
            m_points.push_back(point);
            left = std::fmin(left, point.x());
            right = std::fmax(right, point.x());
            top = std::fmin(top, point.y());
            bottom = std::fmax(bottom, point.y());
        }
        if (left <= right && top <= bottom)
            m_bounds.setCoords(left, top, right, bottom);
    }
    ++m_revision;
    update();
}


MapTrack::MapTrack(QQuickItem *parent)
    : MapTrackItem(parent)
    , m_color(Qt::black)
    , m_lineWidth(1.0)
    , m_builtRevision(0)
    , m_builtDetail(-1)
{
}

void MapTrack::setColor(const QColor &color)
{
    if (color == m_color)
        return;
    m_color = color;
    update();
    Q_EMIT(colorChanged());
}

void MapTrack::setLineWidth(qreal lineWidth)
{
    if (lineWidth == m_lineWidth)
        return;
    m_lineWidth = lineWidth;
    update();
    Q_EMIT(lineWidthChanged());
}

bool MapTrack::hitTest(const QPointF &mapPoint) const
{
    if (!map() || points().size() < 2)
        return false;
    const QPointF point = mapToMercator(mapPoint);
    const double reach = (m_lineWidth/2.0+HitTolerance)/map()->worldSize();
    QRectF bounds;
    mercatorBounds(&bounds);
    if (point.x() < bounds.left()-reach || point.x() > bounds.right()+reach
            || point.y() < bounds.top()-reach || point.y() > bounds.bottom()+reach) {
        return false;
    }
    for (size_t i = 1; i < points().size(); ++i) {
        if (squaredDistanceToSegment(point, points()[i-1], points()[i]) <= reach*reach)
            return true;
    }
    return false;
}

QSGNode *MapTrack::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto node = static_cast<QSGGeometryNode *>(oldNode);
    QVector2D viewOrigin;
    float scale;
    // A track out of the view does not need its geometry until it comes back.
    if (points().size() < 2 || !viewTransform(m_lineWidth/2.0+2.0, &viewOrigin, &scale)) {
        delete node;
        return nullptr;
    }

    const double zoom = map()->zoomLevel();
    const int detail = zoom >= FullDetailZoom ? FullDetail : int(std::ceil(zoom));
    if (!node) {
        node = createNode(lineAttributes(), new LineMaterial);
        m_builtDetail = -1;
    }
    if (detail != m_builtDetail || revision() != m_builtRevision) {
        const double tolerance = detail == FullDetail
                ? 0.0 : SimplifyTolerance/(MapProjection::TileSize*std::pow(2.0, detail));
        buildLine(node->geometry(), points(), origin(), tolerance);
        node->markDirty(QSGNode::DirtyGeometry);
        m_builtDetail = detail;
        m_builtRevision = revision();
    }

    auto material = static_cast<LineMaterial *>(node->material());
    material->viewOrigin = viewOrigin;
    material->scale = scale;
    material->color = m_color;
    material->halfWidth = float(m_lineWidth/2.0);
    node->markDirty(QSGNode::DirtyMaterial);
    return node;
}


MapTrackPoints::MapTrackPoints(QQuickItem *parent)
    : MapTrackItem(parent)
    , m_color(Qt::red)
    , m_borderColor(Qt::white)
    , m_selectedColor(Qt::black)
    , m_selectedBorderColor(Qt::black)
    , m_radius(3.0)
    , m_borderWidth(1.0)
    , m_builtRevision(0)
{
}

void MapTrackPoints::setColor(const QColor &color)
{
    if (color == m_color)
        return;
    m_color = color;
    update();
    Q_EMIT(colorChanged());
}

void MapTrackPoints::setBorderColor(const QColor &color)
{
    if (color == m_borderColor)
        return;
    m_borderColor = color;
    update();
    Q_EMIT(borderColorChanged());
}

void MapTrackPoints::setSelectedColor(const QColor &color)
{
    if (color == m_selectedColor)
        return;
    m_selectedColor = color;
    update();
    Q_EMIT(selectedColorChanged());
}

void MapTrackPoints::setSelectedBorderColor(const QColor &color)
{
    if (color == m_selectedBorderColor)
        return;
    m_selectedBorderColor = color;
    update();
    Q_EMIT(selectedBorderColorChanged());
}

void MapTrackPoints::setRadius(qreal radius)
{
    if (radius == m_radius)
        return;
    m_radius = radius;
    update();
    Q_EMIT(radiusChanged());
}

void MapTrackPoints::setBorderWidth(qreal borderWidth)
{
    if (borderWidth == m_borderWidth)
        return;
    m_borderWidth = borderWidth;
    update();
    Q_EMIT(borderWidthChanged());
}

void MapTrackPoints::setSelection(const QGeoRectangle &selection)
{
    if (selection == m_selection)
        return;
    m_selection = selection;
    update();
    Q_EMIT(selectionChanged());
}

int MapTrackPoints::pointAt(const QPointF &mapPoint) const
{
    if (!map() || points().empty())
        return -1;
    const QPointF point = mapToMercator(mapPoint);
    const double reach = (m_radius+HitTolerance)/map()->worldSize();
    int closest = -1;
    double closestDistance = reach*reach;
    for (size_t i = 0; i < points().size(); ++i) {
        const QPointF delta = points()[i]-point;
        const double distance = QPointF::dotProduct(delta, delta);
        if (distance <= closestDistance) {
            closest = int(i);
            closestDistance = distance;
        }
    }
    return closest;
}

QSGNode *MapTrackPoints::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto node = static_cast<QSGGeometryNode *>(oldNode);
    QVector2D viewOrigin;
    float scale;
    if (!viewTransform(m_radius+2.0, &viewOrigin, &scale)) {
        delete node;
        return nullptr;
    }

    if (!node || revision() != m_builtRevision) {
        if (!node)
            node = createNode(pointAttributes(), new PointsMaterial);
        buildPoints(node->geometry(), points(), origin());
        node->markDirty(QSGNode::DirtyGeometry);
        m_builtRevision = revision();
    }

    auto material = static_cast<PointsMaterial *>(node->material());
    material->viewOrigin = viewOrigin;
    material->scale = scale;
    material->color = m_color;
    material->borderColor = m_borderColor;
    material->selectedColor = m_selectedColor;
    material->selectedBorderColor = m_selectedBorderColor;
    material->radius = float(m_radius);
    material->borderWidth = float(m_borderWidth);
    if (m_selection.isValid()) {
        const QPointF topLeft = MapProjection::fromCoordinate(m_selection.topLeft())-origin();
        const QPointF bottomRight = MapProjection::fromCoordinate(m_selection.bottomRight())-origin();
        material->selection[0] = float(topLeft.x());
        material->selection[1] = float(topLeft.y());
        material->selection[2] = float(bottomRight.x());
        material->selection[3] = float(bottomRight.y());
    } else {
        // An empty area.
        material->selection[0] = material->selection[1] = 1.0f;
        material->selection[2] = material->selection[3] = -1.0f;
    }
    node->markDirty(QSGNode::DirtyMaterial);
    return node;
}
