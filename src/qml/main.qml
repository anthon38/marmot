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

import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.3
import QtPositioning 5.5
import QtLocation 5.14
import QtQuick.Layouts 1.3
import QtQml 2.13
import Marmot 1.0 as Marmot

ApplicationWindow {
    id: application

    property var currentLocation: QtPositioning.coordinate(45.187778, 5.726945) //G-Town
    property var activeFile: null

    visible: true

    Component.onCompleted: {
        // load settings
        visibility = Number.fromLocaleString(Marmot.Settings.value("visibility", Window.Windowed))
        width = Marmot.Settings.value("width", 450*Marmot.Units.devicePixelRatio)
        height = Marmot.Settings.value("height", 300*Marmot.Units.devicePixelRatio)
        delayedLoading.start()
    }

    onClosing: {
        // save settings
        Marmot.Settings.setValue("visibility", visibility)
        Marmot.Settings.setValue("width", width)
        Marmot.Settings.setValue("height", height)
    }

    Timer {
        id: delayedLoading
        interval: 1
        onTriggered: {
            scaleIndicator.update()
            // load files if any
            if (Qt.application.arguments.length > 1) {
                var files = Qt.application.arguments.slice()
                files.shift()
                open(files)
            }
        }
    }

    PositionSource {
        id: positionSource
        onSourceErrorChanged: {
            if (sourceError == PositionSource.NoError)
                return
            console.log("Source error: " + sourceError)
            stop()
        }
        onPositionChanged: {
            currentLocation = position.coordinate
            centerMap()
        }
    }

    function centerMap() {
        map.center = currentLocation
        map.zoomLevel = 10
    }

    Plugin {
        id: osmPlugin
        name: "osm"
        PluginParameter {
            name: "osm.mapping.providersrepository.address"
            value: Marmot.Settings.booleanValue("providersUseEmbedded", true) ? "qrc:/providers/" : Marmot.Utils.location(Marmot.Utils.AppConfigLocation)+"/providers/"
        }
        PluginParameter { name: "osm.useragent"; value: Qt.application.name }
    }

    Plugin {
        id: orsPlugin
        name: "ors"
        PluginParameter { name: "ors.api_key"; value: "5b3ce3597851110001cf624889cac8de7d414405b9394e02dd629d89" }
        PluginParameter { name: "ors.search.sources"; value: "osm" }
    }

    RouteQuery {
        id: aQuery
        travelModes: RouteQuery.PedestrianTravel
        routeOptimizations: RouteQuery.ShortestRoute
        maneuverDetail: RouteQuery.NoManeuvers
        onWaypointsChanged: {
            if (waypoints.length < 2) // not enough waypoints to make a query, the model is not updated so we have to do it manually
                routeModel.reset()
        }
    }

    RouteModel {
        id: routeModel
        plugin: orsPlugin
        query: aQuery
        autoUpdate: true
        onModelReset: plotInfo.updateRoute()
        onStatusChanged: {
            if (status == RouteModel.Error) {
                console.log(error+": "+errorString)
            }
        }
    }

    ListModel {
        id: poiModel
        onRowsInserted: {
            for (var i = first; i < last+1; ++i)
                plotInfo.file.addPoi(QtPositioning.coordinate(get(i).latitude, get(i).longitude), get(i).name)
        }
        onRowsRemoved: {
            for (var i = last; i > first-1; --i)
                plotInfo.file.removePoi(i)
        }
    }

    GeocodeModel {
        id: poiGeocodeModel
        plugin: routeModel.plugin
        autoUpdate: true
        onStatusChanged: {
            if (status == GeocodeModel.Error) {
                console.log(error+": "+errorString)
            }
        }
        onLocationsChanged: {
            if (count == 1) {
                var name = get(0).address.text
                // Use short name if available
                if (get(0).extendedAttributes.name)
                    name = get(0).extendedAttributes.name
                poiModel.append({"name": name, "latitude": query.latitude, "longitude": query.longitude})
            }
        }
    }

    ListModel {
        id: searchModel
    }

    GeocodeModel {
        id: searchGeocodeModel
        plugin: osmPlugin
        limit: 50
        autoUpdate: true
        onStatusChanged: {
            if (status == GeocodeModel.Error) {
                console.log(error+": "+errorString)
            }
        }
        onLocationsChanged: {
            searchModel.clear()
            for (var i = 0; i<count; ++i) {
                searchModel.append({"address": get(i).address.text})
            }
        }
    }

    Map {
        id: map
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            left: menuButton.left
            leftMargin: -menuButton.anchors.leftMargin
        }

        plugin: osmPlugin
        zoomLevel: 10
        onCopyrightLinkActivated: Qt.openUrlExternally(link)
        activeMapType: supportedMapTypes[6]

        Component.onCompleted: centerMap()

        onZoomLevelChanged: scaleIndicator.update()
        onCenterChanged: scaleIndicator.update()

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            cursorShape: (routeModel.status == RouteModel.Loading)
                      || (poiGeocodeModel.status == GeocodeModel.Loading)
                         ? Qt.BusyCursor : Qt.ArrowCursor
            enabled: (routeModel.status != RouteModel.Loading) && (poiGeocodeModel.status != GeocodeModel.Loading)
            onClicked: {
                map.removeMapItem(trackInfoMapItem)
                if (plotRouteButton.checked) {
                    if (mouse.button == Qt.LeftButton) {
                        if (plotInfo.isAddingPoi) {
                            poiGeocodeModel.query = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                        } else {
                            aQuery.addWaypoint(map.toCoordinate(Qt.point(mouse.x, mouse.y)))
                        }
                    } else {
                        if (plotInfo.isAddingPoi) {
                            if (poiModel.count > 0) poiModel.remove(poiModel.count-1)
                        } else {
                            if (aQuery.waypoints.length > 0) aQuery.removeWaypoint(aQuery.waypoints[aQuery.waypoints.length-1])
                        }
                    }
                }
            }
            onPressed: {
                if (editToolBar.deletingZone) {
                    map.gesture.enabled = false
                    zoneSelection.origin = Qt.point(mouse.x, mouse.y)
                    map.addMapItem(zoneSelection)
                }
            }
            onPositionChanged: {
                if (editToolBar.deletingZone) {
                    var xMin = Math.min(zoneSelection.origin.x, mouse.x)
                    var xMax = Math.max(zoneSelection.origin.x, mouse.x)
                    var yMin = Math.min(zoneSelection.origin.y, mouse.y)
                    var yMax = Math.max(zoneSelection.origin.y, mouse.y)
                    zoneSelection.topLeft = map.toCoordinate(Qt.point(xMin, yMin))
                    zoneSelection.bottomRight = map.toCoordinate(Qt.point(xMax, yMax))
                }
            }
            onReleased: {
                if (editToolBar.deletingZone) {
                    map.gesture.enabled = true
                }
            }
        }

        MapItemView {
            z: 1
            model: routeModel
            delegate: MapRoute {
                route: routeData
                line.color: "blue"
                line.width: 5
                smooth: true
                opacity: 0.9
                onRouteChanged: plotInfo.updateRoute()
            }
        }

        MapItemView {
            z: 1
            model:aQuery.waypoints
            delegate: WayPointItem {
                coordinate: modelData
                color: "blue"
            }
        }

        MapItemView {
            z: 11
            model: poiModel
            delegate: PoiMapItem {
                coordinate: QtPositioning.coordinate(latitude, longitude)
                text: name
                imageSource: "qrc:/images/pin_blue.svg"
            }
        }

        Instantiator {
            model: activeFile ? activeFile.tracks : null
            delegate: MapItemView {
                z: 2
                model: ListModel {
                    id: wpModel
                }
                Component.onCompleted: {
                    var path = modelData.path
                    for (var i = 0; i < modelData.length; ++i) {
                        model.append({"latitude": path[i].latitude, "longitude": path[i].longitude})
                    }
                }

                delegate: WayPointItem {
                    readonly property var isSelected: zoneSelection.region.contains(coordinate)
                    coordinate: QtPositioning.coordinate(latitude, longitude)
                    color: isSelected ? "black" : "red"
                    MouseArea {
                        anchors.fill: parent
                        drag.target: editToolBar.movingPoint ? parent : null
                        onReleased: if (editToolBar.movingPoint) modelData.movePoint(index, coordinate)
                        onDoubleClicked:  if (editToolBar.deletingPoint) remove()
                    }
                    Connections {
                        target: deleteShortcut
                        function onActivated() {if (isSelected) remove()}
                    }
                    function remove() {
                        modelData.removePoint(index)
                        wpModel.remove(index)
                    }
                }
            }
            onObjectAdded: map.addMapItemView(object)
            onObjectRemoved: map.removeMapItemView(object)
        }
    }

    MapQuickItem {
        id: trackInfoMapItem
        z: 12
        sourceItem: Label {
            id: trackInfoItem
            padding: Marmot.Units.smallSpacing
            wrapMode: TextEdit.NoWrap
            textFormat: Text.RichText
            background: BackGround {}
        }
    }

    MapRectangle {
        id: zoneSelection
        z: 9
        property var region: QtPositioning.rectangle(topLeft, bottomRight)
        property point origin: Qt.point(-1, -1)
    }

    EditToolBar {
        id: editToolBar
        anchors {
            bottom: parent.top
            margins: Marmot.Units.largeSpacing
            horizontalCenter: map.horizontalCenter
        }
        state: activeFile ? "visible" : ""
        Shortcut {
            id: deleteShortcut
            sequence: StandardKey.Delete
            context: Qt.ApplicationShortcut
            onActivated: map.removeMapItem(zoneSelection)
        }
        onDeletingZoneChanged: if (!deletingZone) map.removeMapItem(zoneSelection)
    }

    onActiveFileChanged: {
        if (activeFile) {
            editToolBar.fileName = activeFile.name
        }
    }

    Marmot.FilesModel {
        id: filesModel

        onFileAppened: {
            map.removeMapItem(trackInfoMapItem)
            // PolyLines
            for (var i = 0; i < file.tracks.length; ++i) {
                var polyLine = Qt.createComponent("PolyLine.qml").createObject(map, {track: file.tracks[i]})
                polyLine.objectName = file.tracks[i].objectName+"_polyline"
                map.addMapItem(polyLine)
                profileChart.createSeries(file.tracks[i])
            }
            // Points of interest
            for (var j = 0; j < file.pois.length; ++j) {
                var poi = Qt.createComponent("PoiMapItem.qml").createObject(map, {coordinate: file.pois[j].coordinate, text: file.pois[j].name})
                poi.objectName = file.pois[j].objectName+"_marker"
                map.addMapItemGroup(poi)
            }
        }

        onFileRemoved: {
            map.removeMapItem(trackInfoMapItem)
            var itemsToRemove = []
            for (var i = 0; i < file.tracks.length; ++i) {
                itemsToRemove.push(file.tracks[i].objectName+"_polyline")
            }
            for (var j = 0; j < file.pois.length; ++j) {
                itemsToRemove.push(file.pois[j].objectName+"_marker")
            }
            var items = map.children
            for (var k = 0; k < items.length; ++k) {
                if (itemsToRemove.includes(items[k].objectName)) {
                    items[k].destroy()
                }
            }
            file.destroy()
        }
    }

    function open(files) {
        if (files.length === 0)
            return
        for (var i = 0; i < files.length; ++i) {
            var f = Qt.createQmlObject('import Marmot 1.0; File {}', filesModel)
            if (f.open(files[i].toString())) {
                filesModel.append(f)
            } else {
                f.destroy()
                console.log("Couldn't open "+files[i].toString())
            }
        }
        aggregateStats.update()
        map.fitViewportToVisibleMapItems()
    }

    function removeFile(index) {
        filesModel.remove(index)
        aggregateStats.update()
    }

    function closeAllFiles() {
        while (filesModel.count > 0)
            filesModel.remove(filesModel.count-1)
        aggregateStats.update()
    }

    function fitToTrack(index) {
        map.fitViewportToGeoShape(filesModel.get(index).boundingBox, 200)
    }

    SideBar {
        id: sideBar
        width: Math.round(200*Marmot.Units.devicePixelRatio)
        height: application.height
    }

    CustomButton {
        id: menuButton
        anchors.top: parent.top
        x: sideBar.position*sideBar.width+Marmot.Units.largeSpacing
        anchors.margins: Marmot.Units.largeSpacing
        icon.name: "application-menu"
        tooltipText: qsTr("Menu")

        onClicked: sideBar.opened ? sideBar.close() : sideBar.open()
    }

    CustomButton {
        id: osmButton
        anchors.right: parent.right
        anchors.bottom: plotRouteButton.top
        anchors.margins: Marmot.Units.largeSpacing
        icon.name: "internet-services"
        tooltipText: qsTr("Open in OpenStreetMap")

        onClicked: Qt.openUrlExternally("https://www.openstreetmap.org/#map="+map.zoomLevel+"/"+map.center.latitude+"/"+map.center.longitude)
    }

    CustomButton {
        id: plotRouteButton
        anchors.right: parent.right
        anchors.bottom: mapTypeButton.top
        anchors.margins: Marmot.Units.largeSpacing
        icon.name: "routeplanning"
        tooltipText: qsTr("Plot a route")
        checkable: true

        onClicked: plotInfo.state = plotInfo.state === "" ? "visible" : ""
    }

    RouteInfo {
        id: plotInfo
        anchors.right: plotRouteButton.left
        anchors.top: parent.bottom
        anchors.margins: Marmot.Units.largeSpacing
    }

    CustomButton {
        id: mapTypeButton
        anchors.right: parent.right
        anchors.bottom: fitToViewButton.top
        anchors.margins: Marmot.Units.largeSpacing
        icon.name: "layer-visible-on"
        tooltipText: qsTr("Layers")
        checked: menu.visible

        onClicked: if (!menu.visible) menu.popup(-menu.width, 0)

        Menu {
            id: menu
            padding: Marmot.Units.smallSpacing

            Repeater {
                id: mapTypeRepeater
                delegate: RadioButton {
                    text: modelData.name
                    checked: map.activeMapType.name === modelData.name
                    onClicked: map.activeMapType = modelData
                }
            }
            Connections {
                target: map
                function onSupportedMapTypesChanged() {
                    var supportedStyles = [MapType.StreetMap, MapType.SatelliteMapDay, MapType.TerrainMap, MapType.PedestrianMap]
                    var availableMaps = []
                    for (var i = 0; i < map.supportedMapTypes.length; ++i) {
                        var type = map.supportedMapTypes[i]
                        if (supportedStyles.includes(type.style)) {
                            availableMaps.push(type)
                        }
                    }
                    mapTypeRepeater.model = availableMaps
                }
            }
        }
    }

    CustomButton {
        id: fitToViewButton
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Marmot.Units.largeSpacing
        icon.name: "zoom-fit-selection"
        tooltipText: qsTr("Fit to view")

        onClicked: {
            if (map.mapItems.length > 0) {
                   map.fitViewportToVisibleMapItems()
            } else {
               centerMap()
            }
        }
    }

    Label {
        id: scaleIndicator
        padding: Marmot.Units.smallSpacing
        anchors.right: fitToViewButton.left
        anchors.bottom: parent.bottom
        anchors.margins: Marmot.Units.largeSpacing

        width: Marmot.Units.gridUnit*15

        background: BackGround {}

        horizontalAlignment: Text.AlignHCenter

        function update() {
            var leftPoint = map.toCoordinate(Qt.point(map.x, map.y))
            var rightPoint = map.toCoordinate(Qt.point(map.x+(rightMark.x-leftMark.x), map.y))
            var distance = leftPoint.distanceTo(rightPoint)
            text = distance > 1000 ? (distance/1000.0).toFixed(1)+" km" : distance.toFixed(0)+" m"
        }

        Rectangle {
            id: leftMark
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: Marmot.Units.smallSpacing

            height: parent.height
            width: Marmot.Units.smallSpacing
            color: Marmot.Theme.text
        }

        Rectangle {
            id: rightMark
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: Marmot.Units.smallSpacing

            height: parent.height
            width: Marmot.Units.smallSpacing
            color: Marmot.Theme.text
        }

        MouseArea {
            anchors.fill: parent
        }
    }

    ProfileChart {
        id: profileChart
        anchors.top: map.top
        anchors.right: map.right
        anchors.margins: Marmot.Units.largeSpacing
        width: 20*Marmot.Units.gridUnit
        height: 10*Marmot.Units.gridUnit
        visible: count > 0

        model: filesModel
    }

    AggregateStats {
        id: aggregateStats
        anchors.top: profileChart.bottom
        anchors.topMargin: Marmot.Units.largeSpacing
        anchors.right: profileChart.right
        visible: profileChart.visible
    }

    DropArea {
        property var supportedExt: ["gpx", "kml"]
        anchors.fill: parent
        onEntered: {
            for (var i = 0; i < drag.urls.length; ++i) {
                var extension = drag.urls[i].split('.').pop();
                if (supportedExt.includes(extension)) {
                    drag.accepted = true
                    return
                }
            }
            drag.accepted = false
        }

        onDropped: {
            var files = []
            for (var i = 0; i < drop.urls.length; ++i) {
                var extension = drop.urls[i].split('.').pop();
                if (supportedExt.includes(extension)) {
                    files.push(drop.urls[i])
                }
            }
            open(files)
        }
    }
}
