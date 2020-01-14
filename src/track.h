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

#ifndef TRACK_H
#define TRACK_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QList>
#include <QGeoCoordinate>
#include <QGeoRectangle>
#include <QColor>

class Track : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString description READ description)
    Q_PROPERTY(int length READ length)
    Q_PROPERTY(QString statistics READ statistics NOTIFY statisticsChanged)
    Q_PROPERTY(QString duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(QVariantList path READ variantPath WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QGeoRectangle boundingBox READ boundingBox)
    Q_PROPERTY(qreal climb READ climb NOTIFY climbChanged)
    Q_PROPERTY(qreal altitudeMax READ altitudeMax NOTIFY altitudeMaxChanged)
    Q_PROPERTY(qreal altitudeMin READ altitudeMin NOTIFY altitudeMinChanged)
    Q_PROPERTY(qreal distance3D READ distance3D NOTIFY distance3DChanged)
    Q_PROPERTY(qreal distance2D READ distance2D NOTIFY distance2DChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit Track(QObject *parent = nullptr);
    QString name() const {return m_name;}
    void setName(const QString &name) {m_name = name;}
    QString description() const {return m_description;}
    void setDescription(const QString &description) {m_description = description;}
    int length() const {return m_path.length();}
    QString statistics() const;
    QString duration() const {return m_duration;}
    Q_INVOKABLE void setDuration(qint64 s);
    QVector<QGeoCoordinate> path() const {return m_path;}
    QVariantList variantPath() const;
    QGeoRectangle boundingBox() const {return m_bbox;}
    void updateBoundingBox();
    qreal climb() const {return m_climb;}
    qreal altitudeMax() const {return m_altitudeMax;}
    qreal altitudeMin() const {return m_altitudeMin;}
    qreal distance3D() const {return m_distance3D;}
    qreal distance2D() const {return m_distance2D;}
    QVector<qreal> distances2D() const {return m_distances2D;}
    QVector<QString> timeStamps() const {return m_timeStamps;}
    QColor color() const {return m_color;}
    void setColor(const QColor &color) {m_color = color; Q_EMIT(colorChanged());}
    Q_INVOKABLE int indexFromDistance(qreal distance) const;
    Q_INVOKABLE QGeoCoordinate coordinateFromDistance(qreal distance) const;
    void addPoint(const QGeoCoordinate &point, const QString &timeStamp = QString());
    Q_INVOKABLE void movePoint(int index, const QGeoCoordinate &point);
    Q_INVOKABLE void removePoint(int index);
    void setPath(const QVariantList &path);
    void computeStatistics(int nAverage = 3, qreal threshold = 3.0);

Q_SIGNALS:
    void statisticsChanged();
    void durationChanged();
    void climbChanged();
    void altitudeMaxChanged();
    void altitudeMinChanged();
    void distance3DChanged();
    void distance2DChanged();
    void pathChanged();
    void colorChanged();

private:
    void computeClimb(qreal threshold = 0);
    void averageAltitudes(int N = 3);
    QString secondsToFormattedString(qint64 s) const;

    QString m_name;
    QString m_description;
    QString m_duration;
    QVector<QGeoCoordinate> m_path;
    QGeoRectangle m_bbox;
    QVector<qreal> m_averagedAltitudes;
    qreal m_climb;
    qreal m_descent;
    qreal m_altitudeMax;
    qreal m_altitudeMin;
    qreal m_distance3D;
    qreal m_distance2D;
    QVector<qreal> m_distances2D;
    QVector<QString> m_timeStamps;
    QColor m_color;
};

#endif // TRACK_H
