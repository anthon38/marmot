import QtQuick 2.0
import org.kde.kirigami 2.9 as Kirigami

Rectangle {
    color: Qt.rgba(Kirigami.Theme.backgroundColor.r, Kirigami.Theme.backgroundColor.g, Kirigami.Theme.backgroundColor.b, 0.9)
    radius: 4
    border.width: 1
    border.color: Kirigami.Theme.alternateBackgroundColor
    MouseArea {
        anchors.fill: parent
        onWheel: wheel.accepted = true
    }
}
