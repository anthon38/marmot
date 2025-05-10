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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import Marmot 1.0 as Marmot


ToolBar {
    id: root

    property alias fileName: fileNameLabel.text
    property alias deletingPoint: deleteButton.checked
    property alias deletingZone: deleteZoneButton.checked
    property alias movingPoint: moveButton.checked

    states: [
        State {
            name: "visible"
            AnchorChanges {
                target: root
                anchors.bottom: undefined
                anchors.top: parent.top
            }
        }
    ]
    transitions: [
        Transition {
            AnchorAnimation { duration: Marmot.Units.longDuration }
        }
    ]

    background: BackGround {}

    RowLayout {
        anchors.fill: parent
        CustomToolButton {
            id: navigateButton
            tooltipText: qsTr("Navigate the map")
            icon.name: "transform-move"
            checked: true
            checkable: true
            autoExclusive: true
        }
        CustomToolButton {
            id: deleteButton
            tooltipText: qsTr("Delete a point")
            icon.name: "node-delete"
            checkable: true
            autoExclusive: true
        }
        CustomToolButton {
            id: deleteZoneButton
            tooltipText: qsTr("Select a zone")
            icon.name: "tool_rect_selection"
            checkable: true
            autoExclusive: true
        }
        CustomToolButton {
            id: moveButton
            tooltipText: qsTr("Move a point")
            icon.name: "edit-node"
            checkable: true
            autoExclusive: true
        }
        Label {
            id: fileNameLabel
            elide: Label.ElideRight
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: true
        }
        CustomToolButton {
            tooltipText: qsTr("Export to GPX")
            icon.name: "document-save-as"
            onClicked: {
                var dialog = Qt.createComponent("SaveAsDialog.qml").createObject(parent)
                dialog.file = application.activeFile
            }
        }
        CustomToolButton {
            tooltipText: qsTr("Exit edit mode")
            icon.name: "window-close-symbolic"
            onClicked: {
                navigateButton.checked = true
                deletingPoint = false
                deletingZone = false
                movingPoint = false
                application.activeFile = null
            }
        }
    }
}
