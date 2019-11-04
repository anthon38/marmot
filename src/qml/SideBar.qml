import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtLocation 5.13
import QtGraphicalEffects 1.13
import org.kde.kirigami 2.9 as Kirigami
import HikeManager 1.0

Drawer {
    id: sidebar

    closePolicy: Popup.CloseOnEscape | Popup.NoAutoClose
    modal: false

    LinearGradient {
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.right
        }
        width: 1.5*Kirigami.Units.smallSpacing
        start: Qt.point(0, 0)
        end: Qt.point(width, 0)
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Qt.rgba(0, 0, 0, 0.5)
            }
            GradientStop {
                position: 0.3
                color: Qt.rgba(0, 0, 0, 0.3)
            }
            GradientStop {
                position: 1.0
                color:  "transparent"
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        clip: true
        spacing: 0

        TabBar {
            id: bar

            Layout.margins: Kirigami.Units.smallSpacing
            Layout.fillWidth: true

            TabButton {
                text: qsTr("Files")
            }
            TabButton {
                text: qsTr("Search")
            }
        }

        SwipeView {

            Layout.fillWidth: true
            Layout.fillHeight: true

            currentIndex: bar.currentIndex

            ColumnLayout {
                spacing: 0

                RowLayout {

                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing

                    ToolButton {
                        display: Button.IconOnly
                        action: Kirigami.Action {
                            iconName: "document-open"
                            text: qsTr("Open...")
                            onTriggered: Qt.createComponent("OpenDialog.qml").createObject(sidebar)
                        }
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        ToolTip.text: action.text
                    }
                    Kirigami.ActionTextField {
                        id: modelFilterField

                        Layout.fillWidth: true

                        font.italic: text.length === 0
                        placeholderText: qsTr("Filter...")
                        enabled: filesModel.count > 0
                        onTextChanged: proxy.setFilterFixedString(text)
                        rightActions: [
                            Kirigami.Action {
                                id: clearAction
                                iconName: "edit-clear"
                                visible: modelFilterField.text !== ""
                                onTriggered: modelFilterField.text = ""
                            }
                        ]
                        Connections {
                            target: filesModel
                            onCountChanged: if (filesModel.count == 0) clearAction.trigger()
                        }
                    }
                    ToolButton {
                        display: Button.IconOnly
                        action: Kirigami.Action {
                            iconName: "document-close"
                            text: qsTr("Close all")
                            onTriggered: application.closeAllFiles()
                        }
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        ToolTip.text: action.text
                    }
                }

                Kirigami.ScrollablePage {

                    Layout.margins: Kirigami.Units.smallSpacing
                    Layout.fillHeight: true

                    ListView {
                        id: filesList

                        clip: true
                        model: SortFilterProxyModel {
                            id: proxy
                            sourceModel: filesModel
                            filterRole: Qt.UserRole+1
                        }
                        currentIndex: -1
                        highlightMoveVelocity: -1
                        boundsBehavior: Flickable.StopAtBounds

                        delegate: Kirigami.SwipeListItem {
                            contentItem: Label {
                                text: name
                            }
                            actions: [
                                Kirigami.Action {
                                    iconName: "document-edit"
                                    text: qsTr("Edit file")
                                    onTriggered: {
                                        if (application.activeFile === filesModel.get(proxy.sourceIndex(index))) {
                                            application.activeFile = null
                                        } else {
                                            application.activeFile = filesModel.get(proxy.sourceIndex(index))
                                        }
                                    }
                                },
                                Kirigami.Action {
                                    iconName: "document-close"
                                    text: qsTr("Close file")
                                    onTriggered: application.removeFile(proxy.sourceIndex(index))
                                }]
                            checked: application.activeFile === filesModel.get(proxy.sourceIndex(index))
                            onClicked: application.fitToTrack(proxy.sourceIndex(index))
                        }
                    }
                }
            }

            ColumnLayout {
                spacing: 0

                Kirigami.ActionTextField {
                    id: searchInput

                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing
                    Layout.fillWidth: true

                    font.italic: text.length === 0
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
                    leftActions: [
                        Kirigami.Action {
                            iconName: "search"
                            visible: searchInput.text !== ""
                            onTriggered: searchInput.accepted()
                        }
                    ]
                    rightActions: [
                        Kirigami.Action {
                            iconName: "edit-clear"
                            visible: searchInput.text !== ""
                            onTriggered: {
                                searchInput.text = ""
                                searchInput.accepted()
                            }
                        }
                    ]
                }

                Kirigami.ScrollablePage {

                    Layout.margins: Kirigami.Units.smallSpacing
                    Layout.fillHeight: true

                    ListView {
                        id: searchResultsList

                        clip: true
                        model: searchModel
                        currentIndex: -1
                        highlightMoveVelocity: -1
                        boundsBehavior: Flickable.StopAtBounds

                        delegate: Kirigami.BasicListItem {
                            reserveSpaceForIcon: false
                            label: address
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
                }
            }

        }

    }
}
