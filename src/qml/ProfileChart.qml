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

Item {
    id: profileChartItem
    property alias count: chart.count
    property alias background: background
    property var tooltipsMap: ({})
    property var positionMarkersMap: ({})
    property var model: null

    BackGround {
        id: background
        anchors.fill: parent
    }

    Marmot.Chart {
        id: chart
        anchors.top: elevationColumn.top
        anchors.topMargin: elevationColumn.itemHeight/2
        anchors.bottom: elevationColumn.bottom
        anchors.bottomMargin: elevationColumn.itemHeight/2
        anchors.left: verticalLine.right
        anchors.right: distanceRow.right
        anchors.rightMargin: distanceRow.lastItemWidth/2
        clip: true

        onExtremaChanged: distanceRow.updateValues()

        onTrackAdded: (track) => {
            var tooltip = Qt.createComponent("ChartTooltip.qml").createObject(profileChartItem)
            tooltipsMap[track.objectName] = tooltip

            var positionMarker = Qt.createComponent("WayPointItem.qml").createObject(mapView.map, {z: 3})
            positionMarker.color = track.color
            positionMarkersMap[track.objectName] = positionMarker
        }

        onTrackRemoved: (trackName) => {
            tooltipsMap[trackName].destroy()
            delete tooltipsMap[trackName]

            positionMarkersMap[trackName].destroy()
            delete positionMarkersMap[trackName]
        }

        MouseArea {
            id: plotMouseArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.AllButtons

            onPositionChanged: function(mouse) {
                if (!model || !containsMouse)
                    return;
                verticalBar.x = mouse.x-1 // for some reason there's a 1px shift
                verticalBar.visible = true
                var i, j
                var distance = chart.mapToDistance(mouse.x)
                for (i = 0; i < model.count; ++i) {
                    for (j = 0; j < model.get(i).tracks.length; ++j) {
                        var track = model.get(i).tracks[j]
                        if (track.length === 0)
                            break
                        var tooltip = tooltipsMap[track.objectName]
                        var positionMarker = positionMarkersMap[track.objectName]
                        if (distance > track.distance2D) {
                            tooltip.visible = false
                            mapView.map.removeMapItem(positionMarker)
                            break
                        }
                        var coordinate = track.coordinateFromDistance(distance)
                        tooltip.text = coordinate.altitude.toFixed(0)
                        var position = chart.mapToPosition(Qt.point(distance, coordinate.altitude))
                        tooltip.x = chart.x+position.x
                        tooltip.y = chart.y+position.y-tooltip.height
                        tooltip.visible = true
                        positionMarker.coordinate = coordinate
                        mapView.map.addMapItem(positionMarker)
                    }
                }
            }

            onExited: {
                if (!model)
                    return;
                var keys = Object.keys(tooltipsMap) // both maps have the same keys
                keys.forEach(function(key) {
                    tooltipsMap[key].visible = false
                    mapView.map.removeMapItem(positionMarkersMap[key])
                });
                verticalBar.visible = false
            }
        }

        Rectangle {
            id: verticalBar
            width: 1
            height: chart.height
            color: elevationLabel.color
            visible: false
        }
    }

    function createSeries(track) {
        return chart.createSeries(track)
    }

    Label {
        id: elevationLabel
        anchors.left: parent.left
        anchors.leftMargin: -width/2+height/2+Marmot.Units.smallSpacing
        anchors.verticalCenter: chart.verticalCenter
        rotation: 270
        horizontalAlignment: Text.AlignHCenter
        text: "<b>"+qsTr("Elevation [m]")+"</b>"
    }

    Column {
        id: elevationColumn
        property int itemHeight: 0
        anchors.top: parent.top
        anchors.topMargin: Marmot.Units.smallSpacing
        anchors.bottom: distanceRow.top
        anchors.bottomMargin: -itemHeight/2
        anchors.left: parent.left
        anchors.leftMargin: elevationLabel.height+2*Marmot.Units.smallSpacing
        spacing: (height-elevationRepeater.count*itemHeight)/(elevationRepeater.count-1)

        Repeater {
            id: elevationRepeater
            model: 5

            Row {
                anchors.right: parent.right
                spacing: 1

                Label {
                    text: Math.floor(chart.yMax-(chart.yMax-chart.yMin)*index/(elevationRepeater.count-1))
                }

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    height: 1
                    width: Marmot.Units.smallSpacing
                    color: elevationLabel.color
                }
            }
        }
        Component.onCompleted: itemHeight = elevationRepeater.itemAt(0).height
    }

    Rectangle {
        id: verticalLine
        anchors.top: chart.top
        anchors.left: elevationColumn.right
        anchors.bottom: chart.bottom
        width: 1
        color: elevationLabel.color
    }

    Rectangle {
        id: horizontalLine
        anchors.bottom: distanceRow.top
        anchors.left: chart.left
        anchors.right: chart.right
        height:1
        color: distanceLabel.color
    }

    Row {
        id: distanceRow
        property int itemWidth: 0
        property int totalItemWidth: 0
        property int firstItemWidth: 0
        property int lastItemWidth: 0

        anchors.bottom: distanceLabel.top
        anchors.left: verticalLine.right
        anchors.leftMargin: -firstItemWidth/2
        anchors.right: parent.right
        anchors.rightMargin: Marmot.Units.smallSpacing
        spacing: (width-totalItemWidth)/(distanceRepeater.count-1)

        Repeater {
            id: distanceRepeater
            model: 5

            Column {
                width: label.contentWidth
                spacing: 1

                Rectangle {
                    anchors.horizontalCenter: label.horizontalCenter
                    height: Marmot.Units.smallSpacing
                    width: 1
                    color: distanceLabel.color
                }

                Label {
                    id: label
                    text: ((chart.xMin+(chart.xMax-chart.xMin)*index/(distanceRepeater.count-1))/1000.0).toFixed(1)
                }
                onWidthChanged: if (index == (distanceRepeater.count-1)) distanceRow.updateValues()
            }
        }

        function updateValues() {
            if (distanceRepeater.count == 0)
                return
            firstItemWidth = distanceRepeater.itemAt(0).width
            lastItemWidth = distanceRepeater.itemAt(distanceRepeater.count-1).width
            totalItemWidth = 0
            for (var i = 0; i<distanceRepeater.count; ++i) {
                totalItemWidth += distanceRepeater.itemAt(i).width
            }
        }
    }

    Label {
        id: distanceLabel
        anchors.bottom: parent.bottom
        anchors.margins: Marmot.Units.smallSpacing
        anchors.horizontalCenter: chart.horizontalCenter
        horizontalAlignment: Text.AlignHCenter
        text: "<b>"+qsTr("Distance [km]")+"</b>"
    }
}
