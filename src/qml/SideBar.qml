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
import QtQuick.Layouts 1.15
import QtLocation 6.9
// import QtGraphicalEffects 1.13
import Marmot 1.0 as Marmot

Drawer {
    id: sidebar

    closePolicy: Popup.CloseOnEscape | Popup.NoAutoClose
    modal: false

    // LinearGradient {
    //     anchors {
    //         top: parent.top
    //         bottom: parent.bottom
    //         left: parent.right
    //     }
    //     width: 1.5*Marmot.Units.smallSpacing
    //     start: Qt.point(0, 0)
    //     end: Qt.point(width, 0)
    //     gradient: Gradient {
    //         GradientStop {
    //             position: 0.0
    //             color: Qt.rgba(0, 0, 0, 0.5)
    //         }
    //         GradientStop {
    //             position: 0.3
    //             color: Qt.rgba(0, 0, 0, 0.3)
    //         }
    //         GradientStop {
    //             position: 1.0
    //             color:  "transparent"
    //         }
    //     }
    // }

    ColumnLayout {
        anchors.fill: parent
        clip: true
        spacing: 0

        RowLayout {

            Layout.fillWidth: true
            Layout.margins: Marmot.Units.largeSpacing

            Label {
                Layout.fillWidth: true
                text: Qt.application.name
                font.pointSize: Math.round(application.font.pointSize*1.80)
            }

            CustomToolButton {
                icon.name: "settings-configure"
                tooltipText: qsTr("Configure...")
                onClicked: popup.open()

                Dialog {
                    id: popup

                    parent: application.contentItem
                    closePolicy: Popup.CloseOnEscape | Popup.NoAutoClose
                    modal: true
                    title: qsTr("Settings")
                    standardButtons: Dialog.Ok | Dialog.Cancel

                    x: Math.round((parent.width - width) / 2)
                    y: Math.round((parent.height - height) / 2)

                    SettingsPanel { id: settingsPanel }

                    onAccepted: settingsPanel.saveSettings()
                }
            }
        }

        TabBar {
            id: bar

            Layout.margins: Marmot.Units.smallSpacing
            Layout.fillWidth: true

            TabButton {
                text: qsTr("Files")
            }
            TabButton {
                text: qsTr("Search")
            }
        }

        SwipeView {

            Layout.fillWidth: true
            Layout.fillHeight: true

            currentIndex: bar.currentIndex
            onCurrentIndexChanged: (index) => bar.currentIndex = index

            ColumnLayout {
                spacing: 0

                RowLayout {

                    Layout.leftMargin: Marmot.Units.smallSpacing
                    Layout.rightMargin: Marmot.Units.smallSpacing

                    CustomToolButton {
                        icon.name: "document-open"
                        tooltipText: qsTr("Open...")
                        onClicked: Qt.createComponent("OpenDialog.qml").createObject(sidebar)
                    }
                    TextField {
                        id: modelFilterField
                        onTextChanged: (text) => proxy.setFilterFixedString(text)
                        IconButton {
                            id: clearFilterButton
                            onClicked: modelFilterField.clear()
                            Connections {
                                target: filesModel
                                function onCountChanged() {
                                    if (filesModel.count === 0) modelFilterField.clear()
                                }
                            }
                        }
                    }
                    CustomToolButton {
                        icon.name: "document-close"
                        tooltipText: qsTr("Close all")
                        onClicked: application.closeAllFiles()
                    }
                    CustomToolButton {
                        icon.name: fileView.useCardView ? "view-list-text" : "view-list-details"
                        tooltipText: fileView.useCardView ? qsTr("List view") : qsTr("Card view")
                        onClicked: fileView.useCardView = !fileView.useCardView
                    }
                }

                Marmot.SortFilterProxyModel {
                    id: proxy

                    sourceModel: filesModel
                    filterRole: Qt.UserRole+1
                }

                Component {
                    id: cardViewComponent

                    ScrollView {
                        background: Rectangle {
                            color: Marmot.Theme.base
                        }
                        ListView {
                            id: listView
                            spacing: Marmot.Units.largeSpacing*2
                            topMargin: spacing
                            bottomMargin: spacing
                            clip: true
                            boundsBehavior: Flickable.StopAtBounds
                            model: proxy
                            delegate: Rectangle {
                                readonly property var file: filesModel.get(proxy.sourceIndex(index))
                                width: listView.width - Marmot.Units.largeSpacing*4
                                implicitHeight: itemDelegate.implicitHeight
                                x: Marmot.Units.largeSpacing*2
                                color: Marmot.Theme.base
                                ItemDelegate {
                                    id: itemDelegate
                                    anchors.fill: parent
                                    contentItem: Item {
                                        implicitWidth: layout.implicitWidth
                                        implicitHeight: layout.implicitHeight
                                        ColumnLayout {
                                            id: layout
                                            anchors {
                                                left: parent.left
                                                right: parent.right
                                            }
                                            RowLayout {
                                                Layout.fillWidth: true
                                                Label {
                                                    Layout.fillWidth: true
                                                    text: name
                                                    elide: Text.ElideRight
                                                    wrapMode: Text.Wrap
                                                    font.pointSize: Math.round(application.font.pointSize*1.30)
                                                }
                                                CustomToolButton {
                                                    Layout.alignment: Qt.AlignTop
                                                    icon.name: "overflow-menu"
                                                    checked: menu.visible
                                                    onClicked: () => menu.visible ? menu.close() : menu.popup(0, height)
                                                    Menu {
                                                        id: menu

                                                        MenuItem {
                                                            action: Action {
                                                                icon.name: "document-edit"
                                                                text: qsTr("Edit file")
                                                                onTriggered: {
                                                                    if (application.activeFile === file) {
                                                                        application.activeFile = null
                                                                    } else {
                                                                        application.activeFile = file
                                                                    }
                                                                }
                                                            }
                                                        }
                                                        MenuItem {
                                                            action: Action {
                                                                icon.name: "document-close"
                                                                text: qsTr("Close file")
                                                                onTriggered: application.removeFile(proxy.sourceIndex(index))
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            Separator {
                                                Layout.fillWidth: true
                                            }
                                            Label {
                                                Layout.fillWidth: true
                                                text: file ? qsTr("Distance: ")+(file.distance3D/1000.0).toFixed(2)+" km | "+qsTr("Climb: ")+file.climb.toFixed(0)+" m" : ""
                                            }
                                            Marmot.Chart {
                                                Layout.fillWidth: true
                                                implicitHeight: 3*Marmot.Units.gridUnit*Marmot.Units.devicePixelRatio
                                                Component.onCompleted: {
                                                    for (var i = 0; i < file.tracks.length; ++i) {
                                                        createSeries(file.tracks[i])
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    highlighted: application.activeFile === file
                                    onClicked: application.fitToTrack(proxy.sourceIndex(index))
                                }
                                // layer.enabled: true
                                // layer.effect: DropShadow {
                                //     cached: true
                                //     verticalOffset: 1
                                //     radius: 12
                                //     samples: 25
                                //     color: Qt.rgba(0, 0, 0, 0.5)
                                // }
                            }
                        }
                    }
                }

                Component {
                    id: listViewComponent

                    ScrollView {
                        background: Rectangle {
                            color: Marmot.Theme.base
                        }
                        ListView {
                            clip: true
                            boundsBehavior: Flickable.StopAtBounds
                            model: proxy
                            delegate: ColumnLayout {
                                readonly property var file: filesModel.get(proxy.sourceIndex(index))
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                }
                                spacing: -separator.height
                                ItemDelegate {
                                    id: itemDelegate
                                    Layout.fillWidth: true
                                    implicitHeight: buttonsRow.implicitHeight + 2*padding

                                    contentItem: Label {
                                        text: name
                                        Row {
                                            id: buttonsRow
                                            anchors.right: parent.right
                                            visible: itemDelegate.hovered
                                            CustomToolButton {
                                                icon.name: "document-edit"
                                                tooltipText: qsTr("Edit file")
                                                onClicked: {
                                                    if (application.activeFile === file) {
                                                        application.activeFile = null
                                                    } else {
                                                        application.activeFile = file
                                                    }
                                                }
                                            }
                                            CustomToolButton {
                                                icon.name: "document-close"
                                                tooltipText: qsTr("Close file")
                                                onClicked: application.removeFile(proxy.sourceIndex(index))
                                            }
                                        }
                                    }
                                    highlighted: application.activeFile === file
                                    onClicked: application.fitToTrack(proxy.sourceIndex(index))
                                }
                                Separator {
                                    id: separator
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }

                StackView {
                    id: fileView

                    property bool useCardView: Marmot.Settings.booleanValue("useCardView", false)
                    Layout.margins: Marmot.Units.smallSpacing
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    initialItem: useCardView ? cardViewComponent : listViewComponent
                    onUseCardViewChanged: {
                        if (depth === 1) {
                            // we had one page so we push the other
                            push(useCardView ? cardViewComponent : listViewComponent, StackView.Immediate)
                        } else if (depth === 2){
                            // we pop the stack
                            pop(StackView.Immediate)
                        }
                    }
                    Component.onDestruction: Marmot.Settings.setValue("useCardView", useCardView)
                }
            }

            ColumnLayout {
                spacing: 0

                RowLayout {

                    Layout.leftMargin: Marmot.Units.smallSpacing
                    Layout.rightMargin: Marmot.Units.smallSpacing

                   TextField {
                        id: searchInput

                        Layout.fillWidth: true
                        leftPadding: searchButton.visible ? searchButton.width : padding
                        rightPadding: clearSearchButton.visible ? clearSearchButton.width : padding

                        font.italic: text.length === 0
                        placeholderText: qsTr("Search...")
                        enabled: searchGeocodeModel.status != GeocodeModel.Loading
                        onAccepted: {
                            var coordinateStrings = text.split(',')
                            if (coordinateStrings.length === 2) {
                                var first = parseFloat(coordinateStrings[0])
                                var second = parseFloat(coordinateStrings[1])
                                var coordinate = QtPositioning.coordinate(first, second)
                                if (!coordinate.isValid)
                                    coordinate = QtPositioning.coordinate(second, first)
                                if (coordinate.isValid) {
                                    searchGeocodeModel.query = coordinate
                                    return
                                }
                            }
                            searchGeocodeModel.query = text
                        }
                        IconButton {
                            id: searchButton
                            height: parent.height
                            anchors {
                                left: parent.left
                                verticalCenter: parent.verticalCenter
                            }
                            visible: searchInput.text !== ""
                            icon.name: "search"

                            onClicked: searchInput.accepted()
                        }
                        IconButton {
                            id: clearSearchButton
                            height: parent.height
                            anchors {
                                right: parent.right
                                verticalCenter: parent.verticalCenter
                            }
                            visible: searchInput.text !== ""
                            icon.name: "edit-clear"

                            onClicked: {
                                searchInput.clear()
                                searchInput.accepted()
                            }
                        }
                    }
                    CustomToolButton {
                        icon.name: "internet-services"
                        tooltipText: qsTr("Provider...")
                        checked: menu.visible
                        onClicked: () => menu.visible ? menu.close() : menu.popup(0, height)
                        Menu {
                            id: menu
                            padding: Marmot.Units.smallSpacing

                            RadioButton {
                                checked: searchGeocodeModel.plugin == osmPlugin
                                action: Action {
                                    text: "Nominatim"
                                    onTriggered: searchGeocodeModel.plugin = osmPlugin
                                }
                            }
                            RadioButton {
                                checked: searchGeocodeModel.plugin == orsPlugin
                                action: Action {
                                    text: "OpenRouteService"
                                    onTriggered: searchGeocodeModel.plugin = orsPlugin
                                }
                            }
                        }
                    }
                }

                ScrollView {
                    background: Rectangle {
                        color: Marmot.Theme.base
                    }

                    Layout.margins: Marmot.Units.smallSpacing
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ListView {
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: searchModel
                        delegate: ColumnLayout {
                            anchors {
                                left: parent.left
                                right: parent.right
                            }
                            spacing: -separator.height
                            ItemDelegate {
                                Layout.fillWidth: true
                                text: address
                                onClicked: {
                                    if (searchGeocodeModel.get(index).boundingBox.isValid) {
                                        mapView.map.fitViewportToGeoShape(searchGeocodeModel.get(index).boundingBox, 200)
                                    } else if (searchGeocodeModel.get(index).coordinate.isValid) {
                                        mapView.map.center = searchGeocodeModel.get(index).coordinate
                                        mapView.map.zoomLevel = 16
                                    }
                                }
                            }
                            Separator {
                                id: separator
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }

        }

    }
}
