import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtLocation 5.13
import QtPositioning 5.5

BackGround {
    id: sidebar

    clip: true

    states: [
        State {
            name: "visible"
            AnchorChanges {
                target: sideBar
                anchors.left: parent.left
                anchors.right: undefined
            }
        }
    ]
    transitions: [
        Transition {
            AnchorAnimation { duration: 125 }
        }
    ]

    TabBar {
        id: bar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 4
        CustomTabButton {
            text: qsTr("Files")
        }
        CustomTabButton {
            text: qsTr("Search")
        }
    }

    SwipeView {
        anchors {
            top: bar.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        currentIndex: bar.currentIndex

        ColumnLayout {
            spacing: -4

            CustomToolButton {
                id: openButton

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                Layout.margins: 4
                text: qsTr("Open...")
                onClicked: Qt.createComponent("OpenDialog.qml").createObject(sidebar)
            }

            ListView {
                id: filesList

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter
                Layout.margins: 4

                clip: true
                model: filesModel
                currentIndex: -1
                highlightMoveVelocity: -1
                boundsBehavior: Flickable.StopAtBounds

                delegate: CustomLabel {
                    default property alias children: mouseArea.data
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: name
                    elide: Text.ElideRight
                    padding: 4

                    MouseArea{
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onContainsMouseChanged: {
                            if (containsMouse) {
                                filesList.currentIndex = index
                            } else {
                                filesList.currentIndex = -1
                            }
                        }
                        onClicked: application.fitToTrack(index)

                        CustomToolButton {
                            anchors {
                                top: parent.top
                                right: parent.right
                                bottom: parent.bottom
                                margins: 4
                            }
                            text: "x"
                            onClicked: application.removeFile(index)
                        }
                    }
                }
                highlight: Rectangle {
                    color: Qt.rgba(colorSet.highlight.r, colorSet.highlight.g, colorSet.highlight.b, 0.4)
                    border.color: colorSet.highlight
                    border.width: 1
                    radius: 4
                }
            }

        }

        ColumnLayout {
            spacing: -4

            TextField {
                id: searchInput

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                Layout.margins: 4

                hoverEnabled: true
                color: enabled ? colorSet.text : Qt.darker(colorSet.text, 2)
                font.italic: text.length == 0
                placeholderText: qsTr("Search...")
                enabled: searchGeocodeModel.status != GeocodeModel.Loading
                onAccepted: {
                    var coordinateStrings = text.split(',')
                    if (coordinateStrings.length === 2) {
                        var first = parseFloat(coordinateStrings[0])
                        var second = parseFloat(coordinateStrings[1])
                        var coordinate = QtPositioning.coordinate(first, second)
                        if (!coordinate.isValid)
                            coordinate = QtPositioning.coordinate(second, first)
                        if (coordinate.isValid) {
                            searchGeocodeModel.query = coordinate
                            return
                        }
                    }
                    searchGeocodeModel.query = text
                }

                background: Rectangle {
                    color: Qt.rgba(colorSet.window.r, colorSet.window.g, colorSet.window.b, 0.9)
                    border.color: (searchInput.hovered || searchInput.activeFocus) ? colorSet.highlight : Qt.lighter(Qt.rgba(colorSet.window.r, colorSet.window.g, colorSet.window.b, 0.9), 2)
                    border.width: 1
                    radius: 4
                }
            }

            ListView {
                id: searchResultsList

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter
                Layout.margins: 4

                clip: true
                model: searchModel
                currentIndex: -1
                highlightMoveVelocity: -1
                boundsBehavior: Flickable.StopAtBounds

                delegate: CustomLabel {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: address
                    elide: Text.ElideRight
                    padding: 4

                    MouseArea{
                        anchors.fill: parent
                        hoverEnabled: true
                        onContainsMouseChanged: {
                            if (containsMouse) {
                                searchResultsList.currentIndex = index
                            } else {
                                searchResultsList.currentIndex = -1
                            }
                        }
                        onClicked: {
                            if (searchGeocodeModel.get(index).boundingBox.isValid) {
                                map.fitViewportToGeoShape(searchGeocodeModel.get(index).boundingBox, 200)
                            } else if (searchGeocodeModel.get(index).coordinate.isValid) {
                                map.center = searchGeocodeModel.get(index).coordinate
                                map.zoomLevel = 16
                            }
                        }
                    }
                }
                highlight: Rectangle {
                    color: Qt.rgba(colorSet.highlight.r, colorSet.highlight.g, colorSet.highlight.b, 0.4)
                    border.color: colorSet.highlight
                    border.width: 1
                    radius: 4
                }
            }

        }

    }
}
