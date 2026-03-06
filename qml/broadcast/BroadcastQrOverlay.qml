import QtQuick

Rectangle {
    id: overlay
    property string deepLink: "home"

    visible: broadcastWindow.webPublishEnabled
    anchors {
        right: parent.right
        bottom: parent.bottom
        margins: 20
    }
    width: col.implicitWidth + 24
    height: col.implicitHeight + 24
    radius: 12
    color: "#F0FFFFFF"
    border.color: "#CCCCCC"
    border.width: 1

    Column {
        id: col
        anchors.centerIn: parent
        spacing: 6

        Image {
            width: 180
            height: 180
            source: overlay.visible ? "image://qrcode/" + overlay.deepLink : ""
            sourceSize: Qt.size(180, 180)
            cache: false
            fillMode: Image.PreserveAspectFit
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Scannez pour suivre en live"
            font.pointSize: 10
            font.bold: true
            color: "#333333"
        }
    }
}
