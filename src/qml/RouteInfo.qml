import QtQuick 2.0
import QtQuick.Controls 2.5
import org.kde.kirigami 2.9 as Kirigami

import HikeManager 1.0

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
            AnchorAnimation { duration: 125 }
        }
    ]

    FilesModel {
        id: fileModel
    }

    Component.onCompleted: {
        file = Qt.createQmlObject('import HikeManager 1.0; File {}', routeInfoItem)
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
        anchors.margins: Kirigami.Units.smallSpacing
        iconSize: Kirigami.Units.gridUnit
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
        anchors.margins: Kirigami.Units.smallSpacing
        iconSize: Kirigami.Units.gridUnit
        icon.name: "flag"
        tooltipText: qsTr("Add points of interest")
        checkable: true
    }

    CustomButton {
        id: exportButton
        anchors.top: parent.top
        anchors.right: textInfo.right
        anchors.margins: Kirigami.Units.smallSpacing
        iconSize: Kirigami.Units.gridUnit
        icon.name: "document-save-as-symbolic"
        tooltipText: qsTr("Export to GPX")
        onClicked: {
            var dialog = Qt.createComponent("SaveAsDialog.qml").createObject(parent)
            dialog.file = plotInfo.file
        }
    }

    TextArea {
        id: textInfo

        anchors.right: altitudeProfile.left
        anchors.left: parent.left
        anchors.top: clearButton.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 0
        wrapMode: TextEdit.NoWrap
        textFormat: Text.RichText
        readOnly: true
        background: Rectangle {
            opacity: 0.0
        }
        text: file.tracks[0].statistics
    }

    ProfileChart {
        id: altitudeProfile

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 0
        width: 200*Kirigami.Units.devicePixelRatio
        height: 100*Kirigami.Units.devicePixelRatio
        antialiasing: true
        background.color: "transparent"
        background.border.color: "transparent"

        model: fileModel
    }
}
