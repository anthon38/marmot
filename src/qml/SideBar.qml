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
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.3
import QtLocation 5.13
import QtGraphicalEffects 1.13
import org.kde.kirigami 2.9 as Kirigami
import Marmot 1.0

Drawer {
    id: sidebar

    closePolicy: Popup.CloseOnEscape | Popup.NoAutoClose
    modal: false

    LinearGradient {
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.right
        }
        width: 1.5*Kirigami.Units.smallSpacing
        start: Qt.point(0, 0)
        end: Qt.point(width, 0)
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Qt.rgba(0, 0, 0, 0.5)
            }
            GradientStop {
                position: 0.3
                color: Qt.rgba(0, 0, 0, 0.3)
            }
            GradientStop {
                position: 1.0
                color:  "transparent"
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        clip: true
        spacing: 0

        RowLayout {

            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing

            Kirigami.Heading {
                Layout.fillWidth: true
                text: Qt.application.name
            }

            CustomToolButton {
                action: Kirigami.Action {
                    iconName: "settings-configure"
                    text: qsTr("Configure...")
                    onTriggered: popup.open()
                }
                tooltipText: action.text

                Dialog {
                    id: popup

                    parent: Overlay.overlay
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

            Layout.margins: Kirigami.Units.smallSpacing
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

            ColumnLayout {
                spacing: 0

                RowLayout {

                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing

                    ToolButton {
                        display: Button.IconOnly
                        action: Kirigami.Action {
                            iconName: "document-open"
                            text: qsTr("Open...")
                            onTriggered: Qt.createComponent("OpenDialog.qml").createObject(sidebar)
                        }
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        ToolTip.text: action.text
                    }
                    Kirigami.ActionTextField {
                        id: modelFilterField

                        Layout.fillWidth: true

                        font.italic: text.length === 0
                        placeholderText: qsTr("Filter...")
                        enabled: filesModel.count > 0
                        onTextChanged: proxy.setFilterFixedString(text)
                        rightActions: [
                            Kirigami.Action {
                                id: clearAction
                                iconName: "edit-clear"
                                visible: modelFilterField.text !== ""
                                onTriggered: modelFilterField.clear()
                            }
                        ]
                        Connections {
                            target: filesModel
                            onCountChanged: if (filesModel.count == 0) clearAction.trigger()
                        }
                    }
                    ToolButton {
                        display: Button.IconOnly
                        action: Kirigami.Action {
                            iconName: "document-close"
                            text: qsTr("Close all")
                            onTriggered: application.closeAllFiles()
                        }
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        ToolTip.text: action.text
                    }
                }

                Kirigami.ScrollablePage {

                    Layout.margins: Kirigami.Units.smallSpacing
                    Layout.fillHeight: true

                    ListView {
                        id: filesList

                        boundsBehavior: Flickable.StopAtBounds
                        model: SortFilterProxyModel {
                            id: proxy
                            sourceModel: filesModel
                            filterRole: Qt.UserRole+1
                        }
                        delegate: Kirigami.SwipeListItem {
                            contentItem: Label {
                                text: name
                            }
                            actions: [
                                Kirigami.Action {
                                    iconName: "document-edit"
                                    text: qsTr("Edit file")
                                    onTriggered: {
                                        if (application.activeFile === filesModel.get(proxy.sourceIndex(index))) {
                                            application.activeFile = null
                                        } else {
                                            application.activeFile = filesModel.get(proxy.sourceIndex(index))
                                        }
                                    }
                                },
                                Kirigami.Action {
                                    iconName: "document-close"
                                    text: qsTr("Close file")
                                    onTriggered: application.removeFile(proxy.sourceIndex(index))
                                }]
                            checked: application.activeFile === filesModel.get(proxy.sourceIndex(index))
                            onClicked: application.fitToTrack(proxy.sourceIndex(index))
                        }
                    }
                }
            }

            ColumnLayout {
                spacing: 0

                RowLayout {

                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing

                    Kirigami.ActionTextField {
                        id: searchInput

                        Layout.fillWidth: true

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
                        leftActions: [
                            Kirigami.Action {
                                iconName: "search"
                                visible: searchInput.text !== ""
                                onTriggered: searchInput.accepted()
                            }
                        ]
                        rightActions: [
                            Kirigami.Action {
                                iconName: "edit-clear"
                                visible: searchInput.text !== ""
                                onTriggered: {
                                    searchInput.clear()
                                    searchInput.accepted()
                                }
                            }
                        ]
                    }
                    ToolButton {
                        action: Action {
                            icon.name: "internet-services"
                            text: qsTr("Provider...")
                        }
                        display: Button.IconOnly
                        checked: menu.visible
                        onClicked: menu.visible ? menu.close() : menu.popup(width, 0)
                        Menu {
                            id: menu
                            padding: Kirigami.Units.smallSpacing

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
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        ToolTip.text: action.text
                    }
                }

                Kirigami.ScrollablePage {

                    Layout.margins: Kirigami.Units.smallSpacing
                    Layout.fillHeight: true

                    ListView {
                        id: searchResultsList

                        boundsBehavior: Flickable.StopAtBounds
                        model: searchModel
                        delegate: Kirigami.BasicListItem {
                            reserveSpaceForIcon: false
                            label: address
                            onClicked: {
                                if (searchGeocodeModel.get(index).boundingBox.isValid) {
                                    map.fitViewportToGeoShape(searchGeocodeModel.get(index).boundingBox, 200)
                                } else if (searchGeocodeModel.get(index).coordinate.isValid) {
                                    map.center = searchGeocodeModel.get(index).coordinate
                                    map.zoomLevel = 16
                                }
                            }
                        }
                    }
                }
            }

        }

    }
}
