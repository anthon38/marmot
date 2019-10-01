import QtQuick 2.0
import QtQuick.Dialogs 1.3

FileDialog {
    id: fileDialog
    title: qsTr("Open...")
    folder: shortcuts.home
    selectMultiple: true // QTBUG-47782
    nameFilters: [ qsTr("Track files (*.kml *.gpx)"), qsTr("KML files (*.kml)"), qsTr("GPX files (*.gpx)"), qsTr("All files (*)")]
    onAccepted: {
        application.open(fileDialog.fileUrls)
        fileDialog.destroy()
    }
    onRejected: fileDialog.destroy()

    Component.onCompleted: open()
}
