import QtQuick 2.0
import QtQuick.Controls 2.5
import org.kde.kirigami 2.9 as Kirigami

ToolButton {
    property var tooltipText
    property var iconSize: Kirigami.Units.iconSizes.small

    icon.width: iconSize
    icon.height: iconSize
    display: Button.IconOnly
    ToolTip.visible: hovered
    ToolTip.delay: 500
    ToolTip.text: tooltipText
}
