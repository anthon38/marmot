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

#include "settings.h"

Settings::Settings(QObject *parent) : QObject(parent)
{
    m_settings = new QSettings(this);
}

QVariant Settings::booleanValue(const QString &key, const QVariant &defaultValue) const
{
    // This is a workaround. It seems impossible to force a conversion from
    // variant to bool in QML without explicitly calling toBool() on the C++ side.
    return m_settings->value(key, defaultValue).toBool();
}

QVariant Settings::value(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    m_settings->setValue(key, value);
}

bool Settings::contains(const QString &key) const
{
    return m_settings->contains(key);
}
