import QtQuick 2.0
import QtQuick.Dialogs 1.2

FileDialog {
    id: fileDialog
    title: qsTr("Save as...")
    folder: shortcuts.home
    defaultSuffix: "gpx"
    selectExisting: false
    nameFilters: [ qsTr("GPX files (*.gpx)"), qsTr("All files (*)")]
    onAccepted: {
        plotInfo.file.exportToGpx(fileDialog.fileUrl)
        fileDialog.destroy()
    }
    onRejected: fileDialog.destroy()

    Component.onCompleted: open()
}
