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
    Q_PROPERTY(qreal xMin READ xMin WRITE setXmin NOTIFY xMinChanged)
    Q_PROPERTY(qreal xMax READ xMax WRITE setXmax NOTIFY xMaxChanged)
    Q_PROPERTY(qreal yMin READ yMin WRITE setYmin NOTIFY yMinChanged)
    Q_PROPERTY(qreal yMax READ yMax WRITE setYmax NOTIFY yMaxChanged)
    Q_PROPERTY(qreal count READ count NOTIFY countChanged)

public:
    explicit Chart(QQuickItem *parent = nullptr);
    qreal xMin() const {return m_xMin;}
    void setXmin(qreal xMin);
    qreal xMax() const {return m_xMax;}
    void setXmax(qreal xMax);
    qreal yMin() const {return m_yMin;}
    void setYmin(qreal yMin);
    qreal yMax() const {return m_yMax;}
    void setYmax(qreal yMax);
    int count() const {return m_trackList.length();}

    void paint(QPainter *painter);
    Q_INVOKABLE void createSeries(Track *track);
    Q_INVOKABLE qreal mapToDistance(int x) const;
    Q_INVOKABLE QPoint mapToPosition(const QPointF &point) const;

Q_SIGNALS:
    void xMinChanged();
    void xMaxChanged();
    void yMinChanged();
    void yMaxChanged();
    void countChanged();
    void trackAdded(Track *track);
    void trackRemoved(QString trackName);

private:
    qreal m_xMin;
    qreal m_xMax;
    qreal m_yMin;
    qreal m_yMax;
    QVector<Track *> m_trackList;
};

#endif // CHART_H
