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

import QtQuick 2.0
import QtLocation 5.13
import org.kde.kirigami 2.9 as Kirigami

MapQuickItem {
    property alias color: circle.color
    sourceItem: Rectangle {
        id: circle
        width: 5*Kirigami.Units.devicePixelRatio+2*border.width
        height: width
        radius: width
        border.width: Kirigami.Units.devicePixelRatio
//        border.color: Qt.tint(color, "#e6ffffff") //white 90% transparency
        border.color: Qt.lighter(color, 2)
    }
    anchorPoint.x: width/2
    anchorPoint.y: height/2
}
