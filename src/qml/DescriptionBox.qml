import QtQuick 2.0
import QtQuick.Controls 2.0

TextArea {
    anchors.margins: 6
    color: colorSet.text
    wrapMode: TextEdit.NoWrap
    textFormat: Text.RichText
    readOnly: true
    placeholderText: "<i>"+qsTr("No description")+"</i>"
//    persistentSelection: true
    background: BackGround {}
    onLinkActivated: Qt.openUrlExternally(link)
}
