import QtQuick 2.0
import QtLocation 5.3

MapQuickItem {
    id: marker

    property var wayPoint: null

    anchorPoint.x: image.width/2
    anchorPoint.y: image.height
    coordinate: if (wayPoint) wayPoint.coordinate

    sourceItem: Image {
        id: image
        source: "qrc:/images/pin_red.svg"

        MouseArea {
            anchors.fill: parent
            onClicked: {
                map.addMapItem(markerDescription)
                markerdbox.clear()
                markerdbox.text = "<center><i>"+wayPoint.name+"</i></center>\n\n"+wayPoint.description
                markerDescription.coordinate = marker.coordinate
                markerDescription.anchorPoint.x = -image.width/2
                markerDescription.anchorPoint.y = markerdbox.height+image.height
            }
        }
    }
}
