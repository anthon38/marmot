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

#include "iconimage.h"

IconImage::IconImage(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setFlag(ItemHasContents, true);
    setImplicitSize(32, 32);
    connect(this, &IconImage::nameChanged, this, &QQuickItem::update);
    connect(this, &QQuickItem::enabledChanged, this, &QQuickItem::update);
}

void IconImage::paint(QPainter *painter)
{
    if (m_icon.isNull())
        return;
    m_icon.paint(painter, 0, 0, static_cast<int>(width()), static_cast<int>(height()),
                 Qt::AlignCenter,
                 isEnabled() ? QIcon::Normal : QIcon::Disabled
                 );
}

void IconImage::setName(const QString &name)
{
    m_icon = QIcon(QIcon::fromTheme(name));
    Q_EMIT(nameChanged());
}
