import QtQuick 2.0
import QtLocation 5.13
import org.kde.kirigami 2.9 as Kirigami

MapQuickItem {
    property alias color: circle.color
    sourceItem: Rectangle {
        id: circle
        width: 5*Kirigami.Units.devicePixelRatio+2*border.width
        height: width
        radius: width
        border.width: Kirigami.Units.devicePixelRatio
//        border.color: Qt.tint(color, "#e6ffffff") //white 90% transparency
        border.color: Qt.lighter(color, 2)
    }
    anchorPoint.x: width/2
    anchorPoint.y: height/2
}
