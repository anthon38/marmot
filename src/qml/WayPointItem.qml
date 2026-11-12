import QtQuick 2.15
import Marmot 1.0 as Marmot

Marmot.MapMarker {
    property alias color: circle.color

    width: circle.width
    height: circle.height
    anchorPoint.x: width/2
    anchorPoint.y: height/2

    Rectangle {
        id: circle
        width: 5*Marmot.Units.devicePixelRatio+2*border.width
        height: width
        radius: width
        border.width: Marmot.Units.devicePixelRatio
//        border.color: Qt.tint(color, "#e6ffffff") //white 90% transparency
        border.color: Qt.lighter(color, 2)
    }
}
