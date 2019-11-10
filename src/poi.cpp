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

#include "poi.h"

Poi::Poi(QObject *parent)
    : QObject(parent)
{
}

void Poi::setName(const QString &name)
{
    m_name = name;
    Q_EMIT(nameChanged());
}

void Poi::setDescription(const QString &description)
{
    m_description = description;
    Q_EMIT(descriptionChanged());
}

void Poi::setCoordinate(const QGeoCoordinate &coordinate)
{
    m_coordinate = coordinate;
    Q_EMIT(coordinateChanged());
}
