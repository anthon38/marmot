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

#ifndef MAPTYPE_H
#define MAPTYPE_H

#include <QObject>
#include <QString>
#include <QUrl>

// A raster tile source, described by a provider file using the same JSON
// format as the QtLocation OSM plugin (UrlTemplate, MaximumZoomLevel,
// MapCopyRight, DataCopyRight).
class MapType : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString key READ key CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString copyright READ copyright CONSTANT)
    Q_PROPERTY(int maximumZoomLevel READ maximumZoomLevel CONSTANT)

public:
    MapType(const QString &key, const QString &urlTemplate, int maximumZoomLevel,
            const QString &copyright, QObject *parent = nullptr);

    // Reads every provider file of a directory (a local path or a ":/" resource path).
    // A default OpenStreetMap "street" type is added if the directory does not provide one.
    static QList<MapType *> load(const QString &directory, QObject *parent);

    int id() const {return m_id;}
    QString key() const {return m_key;}
    QString name() const {return m_name;}
    QString copyright() const {return m_copyright;}
    int maximumZoomLevel() const {return m_maximumZoomLevel;}
    QUrl tileUrl(int zoom, int x, int y) const;

private:
    int m_id;
    QString m_key;
    QString m_name;
    QString m_urlTemplate;
    QString m_copyright;
    int m_maximumZoomLevel;
};

#endif // MAPTYPE_H
