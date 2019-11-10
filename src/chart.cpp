/*************************************************************************
 *  Copyright (c) 2019 Anthony Vital <anthony.vital@gmail.com>           *
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

#include "chart.h"

#include <QPainterPath>
#include <QPainter>

Chart::Chart(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_xMin(0.0)
    , m_xMax(1000.0)
    , m_yMin(0.0)
    , m_yMax(1.0)
{
    setFlag(ItemHasContents, true);

    connect(this, &Chart::countChanged, this, &QQuickItem::update);
    connect(this, &Chart::xMinChanged, this, &QQuickItem::update);
    connect(this, &Chart::xMaxChanged, this, &QQuickItem::update);
    connect(this, &Chart::yMinChanged, this, &QQuickItem::update);
    connect(this, &Chart::yMaxChanged, this, &QQuickItem::update);
}

void Chart::setXmin(qreal xMin)
{
    m_xMin = xMin;
    Q_EMIT(xMinChanged());
}

void Chart::setXmax(qreal xMax)
{
    m_xMax = xMax;
    Q_EMIT(xMaxChanged());
}

void Chart::setYmin(qreal yMin)
{
    m_yMin = yMin;
    Q_EMIT(yMinChanged());
}

void Chart::setYmax(qreal yMax)
{
    m_yMax = yMax;
    Q_EMIT(yMaxChanged());
}

void Chart::paint(QPainter *painter)
{
    painter->setRenderHint(QPainter::Antialiasing);
    qreal xRatio = width()/((m_xMax-m_xMin));
    qreal yRatio = -height()/(m_yMax-m_yMin);

    for (const auto &track : m_trackList) {
        if (track->length() == 0)
            break;

        painter->setPen(QPen(track->color(), 2));
        QPainterPath path;
        path.moveTo((track->distances2D().at(0)-m_xMin)*xRatio,
                    (track->path().at(0).altitude()-m_yMax)*yRatio);
        for (int i = 0; i < track->length(); ++i) {
            path.lineTo((track->distances2D().at(i)-m_xMin)*xRatio,
                        (track->path().at(i).altitude()-m_yMax)*yRatio);
        }
        painter->drawPath(path);
    }
}

void Chart::createSeries(Track *track)
{
    connect(track, &Track::pathChanged, this, &QQuickItem::update);
    connect(track, &Track::destroyed, this, [=](){
        m_trackList.removeOne(track);
        Q_EMIT(countChanged());
        Q_EMIT(trackRemoved(track->objectName()));
    });
    m_trackList.append(track);
    Q_EMIT(countChanged());
    Q_EMIT(trackAdded(track));
}

qreal Chart::mapToDistance(int x) const
{
    return m_xMin+(m_xMax-m_xMin)*x/width();
}

QPoint Chart::mapToPosition(const QPointF &point) const
{
    return QPoint(int(width()*(point.x()-m_xMin)/(m_xMax-m_xMin)), int(height()-height()*(point.y()-m_yMin)/(m_yMax-m_yMin)));
}
