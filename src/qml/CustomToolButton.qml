import QtQuick 2.6
import QtQuick.Controls 2.3

Button {
    id: control

    property alias tooltipText: tooltip.text

    implicitHeight: contentItem.implicitHeight+8
    implicitWidth: implicitHeight
    hoverEnabled: true

    FontLoader {
        id: dejavuFont
        source: "qrc:/fonts/DejaVuSans.ttf"
        onStatusChanged: {
            if (status == FontLoader.Ready) {
                control.font.family = dejavuFont.name
            }
        }
    }

    contentItem: CustomLabel {
        text: control.text
        font: control.font
        horizontalAlignment: Text.AlignHCenter
    }

    background: Rectangle {
        color: (pressed || checked) ? colorSet.highlight : Qt.rgba(colorSet.window.r, colorSet.window.g, colorSet.window.b, 0.9)
        border.color: (hovered || checked) ? colorSet.highlight : Qt.lighter(Qt.rgba(colorSet.window.r, colorSet.window.g, colorSet.window.b, 0.9), 2)
        border.width: 1
        radius: 4
    }

    ToolTip {
        id: tooltip

        parent: control
        visible: hovered && (text != "")
        delay: 500

        contentItem: CustomLabel {
            text: tooltip.text
        }

        background: BackGround {
            border.color: Qt.lighter(color, 2)
            border.width: 1
        }
    }
}
