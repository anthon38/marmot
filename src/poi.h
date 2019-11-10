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

#ifndef POI_H
#define POI_H

#include <QObject>
#include <QString>
#include <QGeoCoordinate>
//#include <QVariantMap>

class Poi : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate NOTIFY coordinateChanged)
//    Q_PROPERTY(QVariantMap attributes READ coordinate)

public:
    explicit Poi(QObject *parent = nullptr);
    QString name() const {return m_name;}
    void setName(const QString &name);
    QString description() const {return m_description;}
    void setDescription(const QString &description);
    QGeoCoordinate coordinate() const {return m_coordinate;}
    void setCoordinate(const QGeoCoordinate &coordinate);
//    QVariantMap attributes() const {return m_attributes;}

Q_SIGNALS:
    void nameChanged();
    void descriptionChanged();
    void coordinateChanged();

private:
    QString m_name;
    QString m_description;
    QGeoCoordinate m_coordinate;
//    QVariantMap m_attributes;
};

#endif // POI_H
