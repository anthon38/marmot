import QtQuick 2.0
import QtQuick.Controls 2.5
import QtLocation 5.13

MapItemGroup {
    property alias text: nameLabel.text
    property alias coordinate: poimapItem.coordinate
    property alias imageSource: image.source

    MapQuickItem {
        id: poimapItem

        anchorPoint.x: image.width/2
        anchorPoint.y: image.height
        sourceItem: Image {
            id: image
            source: "qrc:/images/pin_red.svg"
        }
    }

    MapQuickItem {
        id: labelMapItem

        coordinate: poimapItem.coordinate
        anchorPoint.x: width/2
        anchorPoint.y: height+image.height+4
        sourceItem: Label {
            id: nameLabel
            font.pointSize: application.font.pointSize*0.90
            font.italic: true
            padding: 4
            background: BackGround {}
        }
    }
}
