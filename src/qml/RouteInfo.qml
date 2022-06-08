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
import QtQuick.Controls 2.5
import Marmot 1.0 as Marmot

BackGround {
    id: routeInfoItem

    property alias isAddingPoi: poiButton.checked
    property var file: null

    implicitWidth: textInfo.implicitWidth+altitudeProfile.width
    implicitHeight: altitudeProfile.height

    states: [
        State {
            name: "visible"
            AnchorChanges {
                target: routeInfoItem
                anchors.bottom: parent.bottom
                anchors.top: undefined
            }
        }
    ]
    transitions: [
        Transition {
            AnchorAnimation { duration: Marmot.Units.longDuration }
        }
    ]

    Marmot.FilesModel {
        id: fileModel
    }

    Component.onCompleted: {
        file = Qt.createQmlObject('import Marmot 1.0 as Marmot; Marmot.File {}', routeInfoItem)
        file.addTrack()
//        file.tracks[0].color = "blue"
        fileModel.append(file)
        altitudeProfile.createSeries(file.tracks[0])
    }

    function updateRoute() {
        if (routeModel.count > 0) {
            file.tracks[0].path = routeModel.get(0).path
            file.tracks[0].setDuration(routeModel.get(0).travelTime)
        } else if (file) {
            file.tracks[0].path = []
        }
    }

    CustomButton {
        id: clearButton
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: Marmot.Units.smallSpacing
        iconSize: Marmot.Units.gridUnit
        icon.name: "edit-clear-all-symbolic"
        tooltipText: qsTr("Clear")
        onClicked: {
            aQuery.clearWaypoints()
            poiModel.clear()
        }
    }

    CustomButton {
        id: poiButton
        anchors.top: parent.top
        anchors.horizontalCenter: textInfo.horizontalCenter
        anchors.margins: Marmot.Units.smallSpacing
        iconSize: Marmot.Units.gridUnit
        icon.name: "mark-location-symbolic"
        tooltipText: qsTr("Add points of interest")
        checkable: true
    }

    CustomButton {
        id: exportButton
        anchors.top: parent.top
        anchors.right: textInfo.right
        anchors.margins: Marmot.Units.smallSpacing
        iconSize: Marmot.Units.gridUnit
        icon.name: "document-save-as-symbolic"
        tooltipText: qsTr("Export to GPX")
        onClicked: {
            var dialog = Qt.createComponent("SaveAsDialog.qml").createObject(parent)
            dialog.file = plotInfo.file
        }
    }

    Label {
        id: textInfo

        anchors.right: altitudeProfile.left
        anchors.left: parent.left
        anchors.top: clearButton.bottom
        anchors.bottom: parent.bottom
        anchors.margins: Marmot.Units.smallSpacing
        wrapMode: TextEdit.NoWrap
        textFormat: Text.RichText
        verticalAlignment: Text.AlignTop
        text: file.tracks[0].statistics
    }

    ProfileChart {
        id: altitudeProfile

        width: 20*Marmot.Units.gridUnit
        height: 10*Marmot.Units.gridUnit
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 0
        antialiasing: true
        background.color: "transparent"
        background.border.color: "transparent"

        model: fileModel
    }
}
