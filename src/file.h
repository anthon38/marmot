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

#ifndef FILE_H
#define FILE_H

#include "track.h"

#include <QObject>
#include <QVariantList>
#include <QXmlStreamReader>
#include <QGeoRectangle>

class File : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY opened)
    Q_PROPERTY(QList<QObject*> pois READ pois NOTIFY poiChanged)
    Q_PROPERTY(QList<QObject*> tracks READ tracks NOTIFY tracksChanged)
    Q_PROPERTY(QGeoRectangle boundingBox READ boundingBox)
    Q_PROPERTY(qreal climb READ climb NOTIFY climbChanged)
    Q_PROPERTY(qreal altitudeMax READ altitudeMax NOTIFY altitudeMaxChanged)
    Q_PROPERTY(qreal altitudeMin READ altitudeMin NOTIFY altitudeMinChanged)
    Q_PROPERTY(qreal distance3D READ distance3D NOTIFY distance3DChanged)
    Q_PROPERTY(qreal distance2D READ distance2D NOTIFY distance2DChanged)
public:
    explicit File(QObject *parent = nullptr);
    Q_INVOKABLE bool open(const QString &fileName);
    void parseKml(QXmlStreamReader *reader);
    void parseGpx(QXmlStreamReader *reader);
    QString name() const {return m_name;}
    QObjectList pois() const {return m_pois;}
    QObjectList tracks() const {return m_tracks;}
    QGeoRectangle boundingBox() const {return m_bbox;}
    qreal climb() const {return m_climb;}
    qreal altitudeMax() const {return m_altitudeMax;}
    qreal altitudeMin() const {return m_altitudeMin;}
    qreal distance3D() const {return m_distance3D;}
    qreal distance2D() const {return m_distance2D;}
    Q_INVOKABLE void addPoi(const QGeoCoordinate &coordinate, const QString &name, const QString &description = QString());
    Q_INVOKABLE void removePoi(int index);
    Q_INVOKABLE void addTrack();
    Q_INVOKABLE void exportToGpx(const QString &fileName) const;

Q_SIGNALS:
    void opened();
    void tracksChanged();
    void poiChanged();
    void climbChanged();
    void altitudeMaxChanged();
    void altitudeMinChanged();
    void distance3DChanged();
    void distance2DChanged();

private :
    void appendTrack(Track * track);
    void updateBoundingBox();
    void updateMinMax();

    QString m_name;
    QObjectList m_pois;
    QObjectList m_tracks;
    QGeoRectangle m_bbox;
    qreal m_climb;
    qreal m_altitudeMax;
    qreal m_altitudeMin;
    qreal m_distance3D;
    qreal m_distance2D;
};

#endif // FILE_H
