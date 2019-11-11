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

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QQmlEngine>

class Utils : public QObject
{
    Q_OBJECT
public:
    static QObject *qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
    {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)

        return new Utils();
    }
    explicit Utils(QObject *parent = nullptr);
    Q_INVOKABLE QString prettyUrl(QUrl url) const;
};

#endif // UTILS_H
