import QtQuick 2.0
import QtQuick.Controls 2.5

import HikeManager 1.0

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

    Chart {
        id: chart
        anchors.top: elevationColumn.top
        anchors.topMargin: elevationColumn.itemHeight/2
        anchors.bottom: elevationColumn.bottom
        anchors.bottomMargin: elevationColumn.itemHeight/2
        anchors.left: verticalLine.right
        anchors.right: distanceRow.right
        anchors.rightMargin: distanceRow.lastItemWidth/2
        clip: true
        xMax: (model.xMax < 1000.0) ? 1000.0 : model.xMax
        yMin: model.yMin*0.95
        yMax: (model.yMax < 10.0) ? 10.0 : model.yMax*1.05

        onXMinChanged: distanceRow.updateValues()
        onXMaxChanged: distanceRow.updateValues()
        onYMinChanged: distanceRow.updateValues()
        onYMaxChanged: distanceRow.updateValues()

        onTrackAdded: {
            var tooltip = Qt.createComponent("ChartTooltip.qml").createObject(profileChartItem)
            tooltipsMap[track.objectName] = tooltip

            var positionMarker = Qt.createComponent("WayPointItem.qml").createObject(map, {z: 3})
            positionMarker.color = track.color
            positionMarkersMap[track.objectName] = positionMarker
        }

        onTrackRemoved: {
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

            onPositionChanged: {
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
                            map.removeMapItem(positionMarker)
                            break
                        }
                        var coordinate = track.coordinateFromDistance(distance)
                        tooltip.text = coordinate.altitude.toFixed(0)
                        var position = chart.mapToPosition(Qt.point(distance, coordinate.altitude))
                        tooltip.x = chart.x+position.x
                        tooltip.y = chart.y+position.y-tooltip.height
                        tooltip.visible = true
                        positionMarker.coordinate = coordinate
                        map.addMapItem(positionMarker)
                    }
                }
            }

            onExited: {
                if (!model)
                    return;
                var keys = Object.keys(tooltipsMap) // both maps have the same keys
                keys.forEach(function(key) {
                    tooltipsMap[key].visible = false
                    map.removeMapItem(positionMarkersMap[key])
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
        anchors.leftMargin: -width/2+height/2+4
        anchors.verticalCenter: chart.verticalCenter
        rotation: 270
        horizontalAlignment: Text.AlignHCenter
        text: "<b>"+qsTr("Elevation [m]")+"</b>"
    }

    Column {
        id: elevationColumn
        property int itemHeight: 0
        anchors.top: parent.top
        anchors.topMargin: 4
        anchors.bottom: distanceRow.top
        anchors.bottomMargin: -itemHeight/2
        anchors.left: parent.left
        anchors.leftMargin: elevationLabel.height+4+4
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
                    width: 4
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
        anchors.rightMargin: 4
        spacing: (width-totalItemWidth)/(distanceRepeater.count-1)

        Repeater {
            id: distanceRepeater
            model: 5

            Column {
                width: label.contentWidth
                spacing: 1

                Rectangle {
                    anchors.horizontalCenter: label.horizontalCenter
                    height: 5
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
        anchors.margins: 4
        anchors.horizontalCenter: chart.horizontalCenter
        horizontalAlignment: Text.AlignHCenter
        text: "<b>"+qsTr("Distance [km]")+"</b>"
    }
}
