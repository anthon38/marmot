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
            onCurrentIndexChanged: bar.currentIndex = currentIndex

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
                    ToolButton {
                        display: Button.IconOnly
                        action: Kirigami.Action {
                            iconName: fileView.useCardView ? "view-list-text" : "view-list-details"
                            text: fileView.useCardView ? qsTr("List view") : qsTr("Card view")
                            onTriggered: fileView.useCardView = !fileView.useCardView
                        }
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        ToolTip.text: action.text
                    }
                }

                SortFilterProxyModel {
                    id: proxy

                    sourceModel: filesModel
                    filterRole: Qt.UserRole+1
                }

                Component {
                    id: cardViewComponent

                    ScrollView {
                        background: Rectangle {
                            color: Kirigami.Theme.backgroundColor
                        }
                        ListView {
                            spacing: Kirigami.Units.largeSpacing*2
                            topMargin: spacing
                            bottomMargin: spacing
                            boundsBehavior: Flickable.StopAtBounds
                            model: proxy
                            delegate: Kirigami.AbstractCard {
                                width: parent.width - Kirigami.Units.largeSpacing*4
                                x: Kirigami.Units.largeSpacing*2
                                showClickFeedback: true
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
                                            Kirigami.Heading {
                                                Layout.fillWidth: true
                                                text: name
                                                level: 2
                                                elide: Text.ElideRight
                                                wrapMode: Text.Wrap
                                            }
                                            ToolButton {
                                                Layout.alignment: Qt.AlignTop
                                                action: Action {
                                                    icon.name: "overflow-menu"
                                                }
                                                checked: menu.visible
                                                onClicked: menu.visible ? menu.close() : menu.popup(0, height)
                                                Menu {
                                                    id: menu

                                                    MenuItem {
                                                        action: Action {
                                                            icon.name: "document-edit"
                                                            text: qsTr("Edit file")
                                                            onTriggered: {
                                                                if (application.activeFile === filesModel.get(proxy.sourceIndex(index))) {
                                                                    application.activeFile = null
                                                                } else {
                                                                    application.activeFile = filesModel.get(proxy.sourceIndex(index))
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
                                        Kirigami.Separator {
                                            Layout.fillWidth: true
                                        }
                                        Label {
                                            Layout.fillWidth: true
                                            Component.onCompleted: {
                                                var distance = 0.0
                                                var climb = 0.0
                                                var file = filesModel.get(proxy.sourceIndex(index))
                                                for (var i = 0; i < file.tracks.length; ++i) {
                                                    distance += file.tracks[i].distance3D
                                                    climb += file.tracks[i].climb
                                                }
                                                text = qsTr("Distance: ")+(distance/1000.0).toFixed(2)+" km | "+qsTr("Climb: ")+climb.toFixed(0)+" m"
                                            }
                                        }
                                        Chart {
                                            Layout.fillWidth: true
                                            implicitHeight: 3*Kirigami.Units.gridUnit*Kirigami.Units.devicePixelRatio
                                            Component.onCompleted: {
                                                var file = filesModel.get(proxy.sourceIndex(index))
                                                for (var i = 0; i < file.tracks.length; ++i) {
                                                    createSeries(file.tracks[i])
                                                }
                                            }
                                        }
                                    }
                                }
                                highlighted: application.activeFile === filesModel.get(proxy.sourceIndex(index))
                                onClicked: application.fitToTrack(proxy.sourceIndex(index))
                            }
                        }
                    }
                }

                Component {
                    id: listViewComponent

                    ScrollView {
                        background: Rectangle {
                            color: Kirigami.Theme.backgroundColor
                        }
                        ListView {
                            boundsBehavior: Flickable.StopAtBounds
                            model: proxy
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
                                highlighted: application.activeFile === filesModel.get(proxy.sourceIndex(index))
                                onClicked: application.fitToTrack(proxy.sourceIndex(index))
                            }
                        }
                    }
                }

                StackView {
                    id: fileView

                    property bool useCardView: Settings.booleanValue("useCardView", false)
                    Layout.margins: Kirigami.Units.smallSpacing
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    initialItem: useCardView ? cardViewComponent : listViewComponent
                    onUseCardViewChanged: {
                        if (depth == 1) {
                            // we had one page so we push the other
                            push(useCardView ? cardViewComponent : listViewComponent, StackView.Immediate)
                        } else if (depth == 2){
                            // we pop the stack
                            pop(StackView.Immediate)
                        }
                    }
                    Component.onDestruction: Settings.setValue("useCardView", useCardView)
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
