import QtQuick 2.0
import QtQuick.Controls 2.5
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

    CustomToolButton {
        id: clearButton
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 6
        text: "\uD83E\uDDF9"
        tooltipText: qsTr("Clear")
        onClicked: {
            aQuery.clearWaypoints()
            poiModel.clear()
        }
    }

    CustomToolButton {
        id: poiButton
        anchors.top: parent.top
        anchors.horizontalCenter: textInfo.horizontalCenter
        anchors.margins: 6
        text: "\uD83D\uDEA9"
        tooltipText: qsTr("Add points of interest")
        checkable: true
    }

    CustomToolButton {
        id: exportButton
        anchors.top: parent.top
        anchors.right: textInfo.right
        anchors.margins: 6
        text: "\u2b07"
        tooltipText: qsTr("Export to GPX")
        onClicked: Qt.createComponent("SaveAsDialog.qml").createObject(parent)
    }

    TextArea {
        id: textInfo

        anchors.right: altitudeProfile.left
        anchors.left: parent.left
        anchors.top: clearButton.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 0
        color: colorSet.text
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
        width: 350
        height: 175
        antialiasing: true
        backgroundColor: "transparent"

        model: fileModel
    }
}
