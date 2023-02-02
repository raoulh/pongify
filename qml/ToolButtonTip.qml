import QtQuick
import QtQuick.Controls

ToolButton {
    id: button

    property string tooltipText: ""

    MouseArea {
        id: mouse
        hoverEnabled: true
        anchors.fill: parent
        property bool mouseHovered
        onEntered: mouseHovered = true
        onExited: mouseHovered = false
        onClicked: button.clicked()
    }

    ToolTip.text: tooltipText
    ToolTip.visible: mouse.mouseHovered
}
