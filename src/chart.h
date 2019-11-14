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

#ifndef CHART_H
#define CHART_H

#include <QQuickPaintedItem>

#include "track.h"

class Chart : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(qreal xMin READ xMin NOTIFY extremaChanged)
    Q_PROPERTY(qreal xMax READ xMax NOTIFY extremaChanged)
    Q_PROPERTY(qreal yMin READ yMin NOTIFY extremaChanged)
    Q_PROPERTY(qreal yMax READ yMax NOTIFY extremaChanged)
    Q_PROPERTY(qreal count READ count NOTIFY countChanged)

public:
    explicit Chart(QQuickItem *parent = nullptr);
    qreal xMin() const {return m_xMin;}
    qreal xMax() const {return m_xMax;}
    qreal yMin() const {return m_yMin;}
    qreal yMax() const {return m_yMax;}
    int count() const {return m_trackList.length();}

    void paint(QPainter *painter);
    Q_INVOKABLE void createSeries(Track *track);
    Q_INVOKABLE qreal mapToDistance(int x) const;
    Q_INVOKABLE QPoint mapToPosition(const QPointF &point) const;

Q_SIGNALS:
    void extremaChanged();
    void countChanged();
    void trackAdded(Track *track);
    void trackRemoved(QString trackName);

private:
    void updateExtrema();
    qreal m_xMin;
    qreal m_xMax;
    qreal m_yMin;
    qreal m_yMax;
    QVector<Track *> m_trackList;
};

#endif // CHART_H
