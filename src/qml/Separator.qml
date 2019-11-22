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

import QtQuick 2.13
import QtQuick.Layouts 1.3
import Marmot 1.0 as Marmot

Rectangle {
    height: Math.floor(Marmot.Units.devicePixelRatio)
    width: Math.floor(Marmot.Units.devicePixelRatio)
    Layout.preferredWidth: Math.floor(Marmot.Units.devicePixelRatio)
    Layout.preferredHeight: Math.floor(Marmot.Units.devicePixelRatio)
    color: Qt.tint(Marmot.Theme.text, Qt.rgba(Marmot.Theme.window.r, Marmot.Theme.window.g, Marmot.Theme.window.b, 0.8))
}
