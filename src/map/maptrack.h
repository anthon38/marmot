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

#ifndef MAPTRACK_H
#define MAPTRACK_H

#include <QColor>
#include <QGeoRectangle>
#include <QPointer>
#include <QUrl>
#include <QVector2D>
#include <QVector4D>

#include <vector>

#include "mapitem.h"
#include "track.h"

// Base class of the items showing a track. The track is projected once, when
// its path changes, and the geometry sent to the GPU is expressed in units of
// the world: moving the view only comes down to updating the parameters of a
// shader.
class MapTrackItem : public MapItem
{
    Q_OBJECT
    Q_PROPERTY(Track *track READ track WRITE setTrack NOTIFY trackChanged)

public:
    explicit MapTrackItem(QQuickItem *parent = nullptr);

    Track *track() const {return m_track;}
    void setTrack(Track *track);
    bool mercatorBounds(QRectF *bounds) const override;

Q_SIGNALS:
    void trackChanged();

protected:
    void updateView() override;

    // The path of the track, in normalised Web Mercator.
    const std::vector<QPointF> &points() const {return m_points;}
    // The geometry is expressed relatively to this point, for the sake of precision.
    QPointF origin() const {return m_bounds.center();}
    // Changes every time the points do.
    quint64 revision() const {return m_revision;}
    // A position of the map in normalised Web Mercator, in the copy of the world where the track is.
    QPointF mapToMercator(const QPointF &mapPoint) const;
    // Gives the parameters of the shaders: the top left corner of the view relatively to the
    // origin, and the number of pixels per unit of the world. Returns false if the track is
    // out of the view, given a margin in pixels.
    bool viewTransform(qreal margin, QVector2D *viewOrigin, float *scale) const;

private:
    void project();

    QPointer<Track> m_track;
    std::vector<QPointF> m_points;
    QRectF m_bounds;
    quint64 m_revision;
};

// The line of a track.
//
// It is a line of a single color, unless it is given a fragment shader of its
// own, which decides what the line looks like: it is told where each fragment
// is relatively to the line, and what the track is like there (distance from
// the start, altitude, time). shaders/line_altitude.frag describes what such a
// shader can rely on; the other shaders/line_*.frag are more examples.
class MapTrack : public MapTrackItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    // A compiled fragment shader (.qsb file, made with the qsb tool of Qt) which draws
    // the line instead of the default one. The file is read again when it is set anew.
    Q_PROPERTY(QUrl fragmentShader READ fragmentShader WRITE setFragmentShader NOTIFY fragmentShaderChanged)
    // How many times the fragment shader draws the track, one time above the other.
    Q_PROPERTY(int shaderPasses READ shaderPasses WRITE setShaderPasses NOTIFY shaderPassesChanged)
    // Handed over to the fragment shader.
    Q_PROPERTY(QColor secondaryColor READ secondaryColor WRITE setSecondaryColor NOTIFY secondaryColorChanged)
    Q_PROPERTY(QVector4D shaderParameters READ shaderParameters WRITE setShaderParameters NOTIFY shaderParametersChanged)

public:
    explicit MapTrack(QQuickItem *parent = nullptr);

    QColor color() const {return m_color;}
    void setColor(const QColor &color);
    qreal lineWidth() const {return m_lineWidth;}
    void setLineWidth(qreal lineWidth);
    QUrl fragmentShader() const {return m_fragmentShader;}
    void setFragmentShader(const QUrl &fragmentShader);
    int shaderPasses() const {return m_shaderPasses;}
    void setShaderPasses(int passes);
    QColor secondaryColor() const {return m_secondaryColor;}
    void setSecondaryColor(const QColor &color);
    QVector4D shaderParameters() const {return m_shaderParameters;}
    void setShaderParameters(const QVector4D &parameters);
    bool hitTest(const QPointF &mapPoint) const override;

Q_SIGNALS:
    void colorChanged();
    void lineWidthChanged();
    void fragmentShaderChanged();
    void shaderPassesChanged();
    void secondaryColorChanged();
    void shaderParametersChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    void updateData();

    QColor m_color;
    qreal m_lineWidth;
    QUrl m_fragmentShader;
    // The file of the fragment shader, or nothing for the default one, and
    // what tells it apart from the previous contents of that file.
    QString m_shaderFile;
    QString m_shaderKey;
    int m_shaderPasses;
    QColor m_secondaryColor;
    QVector4D m_shaderParameters;
    // The data of each point for the fragment shader, and their range.
    std::vector<QVector4D> m_data;
    QVector4D m_dataMin;
    QVector4D m_dataMax;
    quint64 m_dataRevision;
    // What the geometry of the node was built from.
    quint64 m_builtRevision;
    int m_builtDetail;
};

// The points of a track, as discs. The ones in the selected area are drawn
// with their own colors.
class MapTrackPoints : public MapTrackItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(QColor selectedColor READ selectedColor WRITE setSelectedColor NOTIFY selectedColorChanged)
    Q_PROPERTY(QColor selectedBorderColor READ selectedBorderColor WRITE setSelectedBorderColor NOTIFY selectedBorderColorChanged)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(qreal borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(QGeoRectangle selection READ selection WRITE setSelection NOTIFY selectionChanged)

public:
    explicit MapTrackPoints(QQuickItem *parent = nullptr);

    QColor color() const {return m_color;}
    void setColor(const QColor &color);
    QColor borderColor() const {return m_borderColor;}
    void setBorderColor(const QColor &color);
    QColor selectedColor() const {return m_selectedColor;}
    void setSelectedColor(const QColor &color);
    QColor selectedBorderColor() const {return m_selectedBorderColor;}
    void setSelectedBorderColor(const QColor &color);
    qreal radius() const {return m_radius;}
    void setRadius(qreal radius);
    qreal borderWidth() const {return m_borderWidth;}
    void setBorderWidth(qreal borderWidth);
    QGeoRectangle selection() const {return m_selection;}
    void setSelection(const QGeoRectangle &selection);

    // Index of the point of the track drawn at a position of the map, or -1.
    Q_INVOKABLE int pointAt(const QPointF &mapPoint) const;

Q_SIGNALS:
    void colorChanged();
    void borderColorChanged();
    void selectedColorChanged();
    void selectedBorderColorChanged();
    void radiusChanged();
    void borderWidthChanged();
    void selectionChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    QColor m_color;
    QColor m_borderColor;
    QColor m_selectedColor;
    QColor m_selectedBorderColor;
    qreal m_radius;
    qreal m_borderWidth;
    QGeoRectangle m_selection;
    quint64 m_builtRevision;
};

#endif // MAPTRACK_H
