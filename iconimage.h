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

#ifndef ICONIMAGE_H
#define ICONIMAGE_H

#include <QQuickPaintedItem>
#include <QIcon>

class IconImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
public:
    explicit IconImage(QQuickItem *parent = nullptr);
    void paint(QPainter *painter);
    QString name() const { return m_icon.name();}
    void setName(const QString& name);

Q_SIGNALS:
    void nameChanged();

private:
    QIcon m_icon;
};

#endif // ICONIMAGE_H
