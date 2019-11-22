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
import Marmot 1.0 as Marmot

MapPolyline {
    id: polyLineItem
    property var track: null

    path: track.path
    line.color: track.color
    line.width: 5*Marmot.Units.devicePixelRatio

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            if (mouse.button == Qt.RightButton) {
                map.addMapItem(trackInfoMapItem)
                trackInfoItem.text = "<center><i>"+track.name+"</i></center>"+track.statistics
                trackInfoMapItem.coordinate = map.toCoordinate(Qt.point(mouse.x+polyLineItem.x, mouse.y+polyLineItem.y))
                trackInfoMapItem.anchorPoint.x = 0
                trackInfoMapItem.anchorPoint.y = trackInfoItem.height
            }
        }
        onDoubleClicked: {
            if (mouse.button == Qt.LeftButton) {
                map.fitViewportToGeoShape(track.boundingBox, 200)
            }
        }
    }
}
