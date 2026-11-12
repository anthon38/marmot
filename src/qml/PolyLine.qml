import QtQuick 2.15
import Marmot 1.0 as Marmot

Marmot.MapTrack {
    color: track.color
    lineWidth: Marmot.Units.gridUnit*0.5

    // Shows the statistics of the track at a position of the map
    function showInfo(position) {
        trackInfoItem.text = "<center><i>"+track.name+"</i></center>"+track.statistics
        trackInfoMapItem.coordinate = mapView.map.toCoordinate(position)
        trackInfoMapItem.visible = true
    }

    function fitToView() {
        mapView.map.fitViewportToGeoShape(track.boundingBox, 200)
    }
}
