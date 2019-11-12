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
import org.kde.kirigami 2.9 as Kirigami

import Marmot 1.0

Item {
    implicitWidth: mainLayout.width
    implicitHeight: mainLayout.height

    function saveSettings() {
        Settings.setValue("providersUseEmbedded", tilesembeddedButton.checked)
        Settings.setValue("providersUseStdPath", tilesstdpathButton.checked)
    }

    Component.onCompleted: {
        // load settings
        tilesembeddedButton.checked = Settings.booleanValue("providersUseEmbedded", true)
        tilesstdpathButton.checked = Settings.booleanValue("providersUseStdPath", false)
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
                    text: qsTr("Configuration directory (%1)").arg(Utils.prettyUrl(Utils.location(Utils.AppConfigLocation)+"/providers/"))
                    ButtonGroup.group: providersButtonGroup
                }
            }
        }

        Kirigami.InlineMessage {
            id: restartMessage
            Layout.fillWidth: true
            visible: false
            text: qsTr("%1 has to be restarted for these changes to take effect.").arg(Qt.application.name)
        }

    }
}
