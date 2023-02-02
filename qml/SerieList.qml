import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    color: "#DADEDA"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            color: "#4fc1e9"
            Layout.preferredHeight: 26
            Layout.fillWidth: true

            Text {
                anchors.fill: parent
                leftPadding: 5
                text: "Test title bla bla bla bla"
                elide: Text.ElideRight
                color: "white"
                font {
                    pointSize: 14
                    bold: true
                }
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

            }
        }

        ToolBar {
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent

                ToolButtonTip {
                    icon.name: "add"
                    onClicked: stack.pop()
                    tooltipText: "Ajouter une nouvelle série"
                }

                ToolButtonTip {
                    icon.name: "edit"
                    onClicked: menu.open()
                    tooltipText: "Modifier une série"
                }

                Item { Layout.fillWidth: true; height: 1 }
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.vertical: ScrollBar {}
        }

    }
}
