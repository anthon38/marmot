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

#ifndef MAPPROJECTION_H
#define MAPPROJECTION_H

#include <QGeoCoordinate>
#include <QPointF>
#include <QtMath>

#include <cmath>

// Web Mercator, normalised so that the whole world is the unit square:
// x grows eastwards from 0 (180°W) to 1 (180°E), y grows southwards from
// 0 (~85.05°N) to 1 (~85.05°S).
namespace MapProjection {

constexpr double MaxLatitude = 85.05112878;
constexpr double TileSize = 256.0;

inline double lonToX(double longitude)
{
    return (longitude + 180.0) / 360.0;
}

inline double latToY(double latitude)
{
    const double s = std::sin(qDegreesToRadians(qBound(-MaxLatitude, latitude, MaxLatitude)));
    return 0.5 - std::log((1.0 + s) / (1.0 - s)) / (4.0 * M_PI);
}

inline double xToLon(double x)
{
    return x * 360.0 - 180.0;
}

inline double yToLat(double y)
{
    return qRadiansToDegrees(std::atan(std::sinh(M_PI * (1.0 - 2.0 * y))));
}

inline QPointF fromCoordinate(const QGeoCoordinate &coordinate)
{
    return QPointF(lonToX(coordinate.longitude()), latToY(coordinate.latitude()));
}

inline QGeoCoordinate toCoordinate(const QPointF &point)
{
    return QGeoCoordinate(yToLat(point.y()), xToLon(point.x()));
}

}

#endif // MAPPROJECTION_H
