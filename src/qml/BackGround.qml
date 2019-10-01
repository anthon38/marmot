import QtQuick 2.0
//import QtQuick.Controls.Universal 2.12

Rectangle {
    color: Qt.rgba(colorSet.window.r, colorSet.window.g, colorSet.window.b, 0.9)
//    color: colorSet.window
    radius: 4

    MouseArea {
        anchors.fill: parent
        onWheel: wheel.accepted = true
    }
}
