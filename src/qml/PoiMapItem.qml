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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 6.9
import Marmot 1.0 as Marmot

MapItemGroup {
    property alias text: nameLabel.text
    property alias coordinate: poimapItem.coordinate
    property alias imageSource: image.source
    z: 10

    MapQuickItem {
        id: poimapItem

        anchorPoint.x: image.width/2
        anchorPoint.y: image.height
        sourceItem: Image {
            id: image
            source: "qrc:/images/pin_red.svg"
        }
    }

    MapQuickItem {
        id: labelMapItem

        coordinate: poimapItem.coordinate
        anchorPoint.x: width/2
        anchorPoint.y: height+image.height+Marmot.Units.smallSpacing
        sourceItem: Label {
            id: nameLabel
            font.pointSize: application.font.pointSize*0.90
            font.italic: true
            padding: Marmot.Units.smallSpacing
            background: BackGround {}
        }
    }
}
