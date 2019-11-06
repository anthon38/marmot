import QtQuick 2.0
import QtQuick.Controls 2.13

Item {
    implicitWidth: label.implicitWidth
    implicitHeight: label.implicitHeight

    Label {
        id: label
        anchors.fill: parent
        text: "Lorem ipsum..."
    }
}
