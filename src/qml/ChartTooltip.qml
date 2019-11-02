import QtQuick 2.0
import QtQuick.Controls 2.5

Label {
    id: root
    visible: false
    padding: 2
    background: Rectangle {
        anchors.bottom: parent.bottom
        width: 10
        height: 1
        color: root.color
    }
}
