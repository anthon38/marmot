/*************************************************************************
 *  Copyright (c) 2026 Anthony Vital <anthony.vital@gmail.com>           *
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
import Marmot 1.0 as Marmot

// An interactive map: a Marmot.Map which can be dragged, flicked and zoomed,
// along with the credits of its tiles. The items declared in a MapCanvas are
// added to the map.
Item {
    id: root

    property alias map: map
    // Whether the pointer moves and zooms the map.
    property bool gestureEnabled: true
    default property alias mapData: map.data

    Marmot.Map {
        id: map
        anchors.fill: parent
        clip: true

        DragHandler {
            target: null
            enabled: root.gestureEnabled
            onTranslationChanged: (delta) => map.pan(-delta.x, -delta.y)
            onActiveChanged: active ? flick.stop() : flick.launch(centroid.velocity)
        }

        PinchHandler {
            id: pinch
            property var startCentroid
            target: null
            enabled: root.gestureEnabled
            grabPermissions: PointerHandler.TakeOverForbidden
            onActiveChanged: {
                if (active) {
                    flick.stop()
                    startCentroid = map.toCoordinate(centroid.position)
                } else {
                    flick.launch(centroid.velocity)
                }
            }
            onScaleChanged: (delta) => {
                map.zoomLevel += Math.log2(delta)
                map.alignCoordinateToPoint(startCentroid, centroid.position)
            }
        }

        WheelHandler {
            enabled: root.gestureEnabled
            // Same workaround as the MapView of QtLocation: touchpads are not told apart from mice everywhere.
            acceptedDevices: Qt.platform.pluginName === "cocoa" || Qt.platform.pluginName === "wayland"
                             ? PointerDevice.Mouse | PointerDevice.TouchPad
                             : PointerDevice.Mouse
            onWheel: (event) => {
                // Zoom around the pointer
                var coordinate = map.toCoordinate(point.position)
                map.zoomLevel += event.angleDelta.y/120
                map.alignCoordinateToPoint(coordinate, point.position)
            }
        }

        // Keeps the map moving for a moment once it is released
        NumberAnimation {
            id: flick

            property vector2d distance
            property real progress: 0
            property real applied: 0

            target: flick
            property: "progress"
            from: 0
            to: 1
            duration: 500
            easing.type: Easing.OutQuad

            onProgressChanged: {
                map.pan(-distance.x*(progress-applied), -distance.y*(progress-applied))
                applied = progress
            }

            function launch(velocity) {
                stop()
                distance = Qt.vector2d(0, 0)
                progress = 0
                applied = 0
                distance = Qt.vector2d(velocity.x*0.2, velocity.y*0.2)
                if (distance.length() >= 1)
                    start()
            }
        }
    }

    Label {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        visible: text !== ""
        padding: Math.floor(Marmot.Units.smallSpacing/2)
        font.pointSize: application.font.pointSize*0.90
        textFormat: Text.StyledText
        text: map.activeMapType ? map.activeMapType.copyright : ""
        onLinkActivated: (link) => Qt.openUrlExternally(link)
        background: Rectangle {
            color: Qt.rgba(Marmot.Theme.window.r, Marmot.Theme.window.g, Marmot.Theme.window.b, 0.7)
        }
    }
}
