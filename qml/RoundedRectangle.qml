import QtQuick

Rectangle {
    property bool topLeftRounded: true
    property bool topRightRounded: true
    property bool bottomLeftRounded: true
    property bool bottomRightRounded: true

    Rectangle {
        color: parent.color
        width: parent.radius
        height: width
        anchors { left: parent.left; top: parent.top }
        visible: !topLeftRounded
    }

    Rectangle {
        color: parent.color
        width: parent.radius
        height: width
        anchors { right: parent.right; top: parent.top }
        visible: !topRightRounded
    }

    Rectangle {
        color: parent.color
        width: parent.radius
        height: width
        anchors { left: parent.left; bottom: parent.bottom }
        visible: !bottomLeftRounded
    }

    Rectangle {
        color: parent.color
        width: parent.radius
        height: width
        anchors { right: parent.right; bottom: parent.bottom }
        visible: !bottomRightRounded
    }
}
