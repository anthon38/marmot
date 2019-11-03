import QtQuick 2.0
import QtLocation 5.13
import org.kde.kirigami 2.9 as Kirigami

MapPolyline {
    id: polyLineItem
    property var track: null

    path: track.path
    line.color: track.color
    line.width: 5*Kirigami.Units.devicePixelRatio

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            if (mouse.button == Qt.RightButton) {
                map.addMapItem(markerDescription)
                markerdbox.clear()
                markerdbox.text = "<center><i>"+track.name+"</i></center>"+track.statistics
                markerDescription.coordinate = map.toCoordinate(Qt.point(mouse.x+polyLineItem.x, mouse.y+polyLineItem.y))
                markerDescription.anchorPoint.x = 0
                markerDescription.anchorPoint.y = markerdbox.height
            } else if (mouse.button == Qt.LeftButton) {
                application.activeTrack = track
            }
        }
        onDoubleClicked: {
            if (mouse.button == Qt.LeftButton) {
                map.fitViewportToGeoShape(track.boundingBox, 200)
            }
        }
    }
}
