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
import QtQuick.Controls 2.15
import Marmot 1.0 as Marmot

Label {
    padding: Marmot.Units.smallSpacing

    background: BackGround {}

    function update() {
        var distance = 0
        var climb = 0
        for (var i = 0; i < filesModel.count; ++i) {
            for (var j = 0; j < filesModel.get(i).tracks.length; ++j) {
                distance += filesModel.get(i).tracks[j].distance3D
                climb += filesModel.get(i).tracks[j].climb
            }
        }
        text = "<i><b>"+qsTr("Aggregate: ")+"</b></i>"+qsTr("Distance: ")+(distance/1000.0).toFixed(2)+" km | "+qsTr("Climb: ")+climb.toFixed(0)+" m"
    }

    MouseArea {
        anchors.fill: parent
    }
}
