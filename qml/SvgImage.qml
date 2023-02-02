import QtQuick

Item {
    id: img

    property alias source: svg.source
    property alias status: svg.status
    property alias cache: svg.cache
    readonly property bool valid: status === Image.Ready

    width: implicitWidth
    height: implicitHeight
    implicitWidth: svg.implicitWidth
    implicitHeight: svg.implicitHeight

    Image {
        id: svg

        fillMode: Image.PreserveAspectFit
        anchors.fill: parent
        sourceSize.width: width
        sourceSize.height: height
    }
}
