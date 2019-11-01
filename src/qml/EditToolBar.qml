import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3


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
            AnchorAnimation { duration: 125 }
        }
    ]

    background: BackGround {}

    RowLayout {
        anchors.fill: parent
        CustomToolButton {
            id: navigateButton
            text: qsTr("n")
            tooltipText: qsTr("Navigate the map")
            checked: true
            checkable: true
            autoExclusive: true
        }
        CustomToolButton {
            id: deleteButton
            text: qsTr("x")
            tooltipText: qsTr("Delete a point")
            checkable: true
            autoExclusive: true
        }
        CustomToolButton {
            id: deleteZoneButton
            text: qsTr("xZ")
            tooltipText: qsTr("Delete points within a zone")
            checkable: true
            autoExclusive: true
        }
        CustomToolButton {
            id: moveButton
            text: qsTr("m")
            tooltipText: qsTr("Move a point")
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
            text: "\u2b07"
            tooltipText: qsTr("Export to GPX")
            onClicked: {
                var dialog = Qt.createComponent("SaveAsDialog.qml").createObject(parent)
                dialog.file = application.activeFile
            }
        }
    }
}
