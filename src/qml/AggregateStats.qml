import QtQuick 2.0
import QtQuick.Controls 2.5
import org.kde.kirigami 2.9 as Kirigami

Label {
    padding: Kirigami.Units.smallSpacing

    background: BackGround {}

    function update() {
        var distance = 0
        var climb = 0
        for (var i = 0; i < filesModel.count; ++i) {
            for (var j = 0; j < filesModel.get(i).tracks.length; ++j) {
                distance += filesModel.get(i).tracks[j].distance3D
                climb += filesModel.get(i).tracks[j].climb
            }
        }
        text = "<i><b>"+qsTr("Aggregate: ")+"</b></i>"+qsTr("Distance: ")+(distance/1000.0).toFixed(2)+" km | "+qsTr("Climb: ")+climb.toFixed(0)+" m"
    }

    MouseArea {
        anchors.fill: parent
    }
}
