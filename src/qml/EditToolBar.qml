import QtQuick 2.7
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
    onStateChanged: if (state == "visible") navigateButton.forceActiveFocus()

    background: BackGround {}

    RowLayout {
        anchors.fill: parent
        Button {
            id: navigateButton
            ToolTip.visible: hovered
            ToolTip.delay: 500
            ToolTip.text: qsTr("Navigate the map")
            icon.name: "transform-move"
            checked: true
            checkable: true
            autoExclusive: true
        }
        Button {
            id: deleteButton
            ToolTip.visible: hovered
            ToolTip.delay: 500
            ToolTip.text: qsTr("Delete a point")
            icon.name: "node-delete"
            checkable: true
            autoExclusive: true
        }
        Button {
            id: deleteZoneButton
            ToolTip.visible: hovered
            ToolTip.delay: 500
            ToolTip.text: qsTr("Select a zone")
            icon.name: "tool_rect_selection"
            checkable: true
            autoExclusive: true
        }
        Button {
            id: moveButton
            ToolTip.visible: hovered
            ToolTip.delay: 500
            ToolTip.text: qsTr("Move a point")
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
        Button {
            ToolTip.visible: hovered
            ToolTip.delay: 500
            ToolTip.text: qsTr("Export to GPX")
            icon.name: "document-save-as"
            onClicked: {
                var dialog = Qt.createComponent("SaveAsDialog.qml").createObject(parent)
                dialog.file = application.activeFile
            }
        }
        Button {
            ToolTip.visible: hovered
            ToolTip.delay: 500
            ToolTip.text: qsTr("Exit edit mode")
            icon.name: "dialog-close"
            onClicked: application.activeFile = null
        }
    }
}
