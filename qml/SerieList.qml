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
            Layout.preferredHeight: 50
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 5

                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    leftPadding: 5
                    text: currentTournament.name
                    elide: Text.ElideRight
                    color: "white"
                    font {
                        pointSize: 14
                        bold: true
                    }
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }

                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    leftPadding: 5
                    text: currentTournament.date.toLocaleString(Qt.locale("fr_FR"), "dddd, d MMMM yyyy")
                    elide: Text.ElideRight
                    color: "white"
                    font {
                        pointSize: 12
                        bold: false
                    }
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        ToolBar {
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent

                ToolButtonTip {
                    icon.name: "add"
                    onClicked: mainWindow.newSerie()
                    tooltipText: "Ajouter une nouvelle série"
                }

                Item { Layout.fillWidth: true; height: 1 }
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            model: currentTournament? currentTournament.series: undefined

            delegate: ItemDelegate {
                id: control

                width: parent? parent.width: 0
                height: bt.implicitHeight + 10
                text: name

                contentItem: RowLayout {
                    width: 300
                    spacing: 4

                    Text {
                        Layout.fillWidth: true
                        rightPadding: control.spacing
                        text: control.text
                        font.bold: true
                        color: control.enabled ? "#0f4932" : "#bdbebf"
                        elide: Text.ElideMiddle
                        verticalAlignment: Text.AlignVCenter
                    }

                    Button {
                        id: bt
                        Layout.preferredWidth: implicitWidth
                        icon.name: "menu"
                        icon.height: 16
                        onClicked: mainWindow.showSerieMenu(index)
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }

    }
}
