/*************************************************************************
 *  Copyright (c) 2019 Anthony Vital <anthony.vital@gmail.com>           *
 *                                                                       *
 *  This file is part of Marmot.                                         *
 *                                                                       *
 *  Marmot is free software: you can redistribute it and/or modify       *
 *  it under the terms of the GNU General Public License as published by *
 *  the Free Software Foundation, either version 3 of the License, or    *
 *  (at your option) any later version.                                  *
 *                                                                       *
 *  Marmot is distributed in the hope that it will be useful,            *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with Marmot. If not, see <http://www.gnu.org/licenses/>.       *
 *************************************************************************/

import QtQuick 2.0
import QtQuick.Dialogs 6.2
import QtCore

FileDialog {
    property var file: null
    id: fileDialog
    title: qsTr("Save as...")
    currentFolder: StandardPaths.writableLocation(StandardPaths.HomeLocation)
    defaultSuffix: "gpx"
    fileMode: FileDialog.SaveFile
    nameFilters: [ qsTr("GPX files (*.gpx)"), qsTr("All files (*)")]
    onAccepted: {
        if (file)
            file.exportToGpx(fileDialog.selectedFile)
        fileDialog.destroy()
    }
    onRejected: fileDialog.destroy()

    Component.onCompleted: open()
}
