import QtQuick 2.0
import QtQuick.Controls 2.3

TabButton {
    id: control
    implicitHeight: contentItem.implicitHeight+16
    implicitWidth: contentItem.implicitWidth+16
    hoverEnabled: true
    contentItem: CustomLabel {
        text: control.text
        font: control.font
        horizontalAlignment: Text.AlignHCenter
    }

    background: Rectangle {
        color: Qt.rgba(colorSet.window.r, colorSet.window.g, colorSet.window.b, (control.pressed || control.checked) ? 0.9 : control.hovered ? 0.2 : 0.4)
        border.color: Qt.lighter(Qt.rgba(colorSet.window.r, colorSet.window.g, colorSet.window.b, 0.9), 2)
        border.width: 1
    }
}
