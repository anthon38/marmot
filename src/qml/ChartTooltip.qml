import QtQuick 2.0
import QtQuick.Controls 2.0

CustomLabel {
    visible: false
    padding: 2
    background: Rectangle {
        anchors.bottom: parent.bottom
        width: 10
        height: 1
        color: colorSet.text
    }
}
