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
import Marmot 1.0 as Marmot

Item {
    implicitWidth: mainLayout.width
    implicitHeight: mainLayout.height

    function saveSettings() {
        Marmot.Settings.setValue("providersUseEmbedded", tilesembeddedButton.checked)
        Marmot.Settings.setValue("providersUseStdPath", tilesstdpathButton.checked)
    }

    Component.onCompleted: {
        // load settings
        tilesembeddedButton.checked = Marmot.Settings.booleanValue("providersUseEmbedded", true)
        tilesstdpathButton.checked = Marmot.Settings.booleanValue("providersUseStdPath", false)
    }

    ColumnLayout {
        id: mainLayout

        RowLayout {
            Layout.fillWidth: true
            Label {
                Layout.alignment: Qt.AlignTop
                text: qsTr("Tile providers: ")
            }
            Column {
                id: tilesColumn
                ButtonGroup {
                    id: providersButtonGroup
                    onClicked: restartMessage.visible = true
                }
                RadioButton {
                    id: tilesembeddedButton
                    text: qsTr("Embedded in %1").arg(Qt.application.name)
                    ButtonGroup.group: providersButtonGroup
                }
                RadioButton {
                    id: tilesstdpathButton
                    text: qsTr("Configuration directory (%1)").arg(Marmot.Utils.prettyUrl(Marmot.Utils.location(Marmot.Utils.AppConfigLocation)+"/providers/"))
                    ButtonGroup.group: providersButtonGroup
                }
            }
        }

        Control {
            id: restartMessage
            Layout.fillWidth: true
            visible: false
            contentItem: RowLayout {
                    id: content
                    anchors.fill: parent
                    anchors.margins: Marmot.Units.smallSpacing
                    Marmot.IconImage {
                        name: "dialog-information"
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("%1 has to be restarted for these changes to take effect.").arg(Qt.application.name)
                    }
                    ToolButton {
                        icon.name: "dialog-close"
                        onClicked: restartMessage.visible = false
                    }
                }
            background: Rectangle {
                id: bgBorderRect
                color: Marmot.Theme.highlight
                radius: Marmot.Units.smallSpacing/2
                Rectangle {
                    id: bgFillRect
                    anchors.fill: bgBorderRect
                    anchors.margins: Marmot.Units.devicePixelRatio
                    color: Marmot.Theme.window
                    radius: bgBorderRect.radius * 0.60
                }

                Rectangle {
                    anchors.fill: bgFillRect
                    color: bgBorderRect.color
                    opacity: 0.20
                    radius: bgFillRect.radius
                }
            }
        }

    }
}
