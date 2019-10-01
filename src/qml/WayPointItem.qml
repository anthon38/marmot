import QtQuick 2.0
import QtLocation 5.13

MapQuickItem {
    property alias color: circle.color
    sourceItem: Rectangle {
        id: circle
        width: 10+2*border.width
        height: width
        radius: width
        border.width: 2
//        border.color: Qt.tint(color, "#e6ffffff") //white 90% transparency
        border.color: Qt.lighter(color, 2)
    }
    anchorPoint.x: width/2
    anchorPoint.y: height/2
}
