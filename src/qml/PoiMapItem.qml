import QtQuick 2.15
import QtQuick.Controls 2.15
import Marmot 1.0 as Marmot

Marmot.MapMarker {
    property alias text: nameLabel.text
    property alias imageSource: image.source

    z: 10
    width: image.width
    height: image.height
    anchorPoint.x: width/2
    anchorPoint.y: height

    Image {
        id: image
        source: "qrc:/images/pin_red.svg"
    }

    Label {
        id: nameLabel
        anchors.horizontalCenter: image.horizontalCenter
        anchors.bottom: image.top
        anchors.bottomMargin: Marmot.Units.smallSpacing
        font.pointSize: application.font.pointSize*0.90
        font.italic: true
        padding: Marmot.Units.smallSpacing
        background: BackGround {}
    }
}
