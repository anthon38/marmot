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
import Qt.labs.platform 1.1
import Qt.labs.settings 1.0

Item {
    implicitWidth: mainLayout.width
    implicitHeight: mainLayout.height

    Settings {
        id: settings
        property bool providersUseEmbedded: true
        property bool providersUseStdPath: false
    }

    function saveSettings() {
        settings.providersUseEmbedded = tilesembeddedButton.checked
        settings.providersUseStdPath = tilesstdpathButton.checked
    }

    Component.onCompleted: {
        // load settings
        tilesembeddedButton.checked = settings.providersUseEmbedded
        tilesstdpathButton.checked = settings.providersUseStdPath
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
                RadioButton {
                    id: tilesembeddedButton
                    text: qsTr("Embedded in %1").arg(Qt.application.name)
                }
                RadioButton {
                    id: tilesstdpathButton
                    text: qsTr("Configuration folder (%1)").arg(StandardPaths.writableLocation(StandardPaths.AppConfigLocation)+"/providers/")
                }
            }
        }

    }
}
