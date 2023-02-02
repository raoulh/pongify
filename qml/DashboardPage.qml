import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {

    SvgImage {
        id: logo
        source: "qrc:/img/logo.svg"
    }

    ColumnLayout {
        GroupBox {
            Layout.preferredWidth: logo.implicitWidth + 100
            Layout.preferredHeight: 300

            title: "Tournois récents"

            Rectangle {
                width: parent.width
                height: parent.height
                clip: true

                ListView {
                    anchors.fill: parent

                    model: storage

                    delegate: ItemDelegate {
                        id: control

                        text: name
                        width: parent? parent.width: 0
                        height: bt.implicitHeight + 10

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

                            Text {
                                Layout.preferredWidth: implicitWidth
                                rightPadding: control.spacing
                                text: date
                                font {
                                    italic: true
                                }
                                color: control.enabled ? "#28cc8b" : "#bdbebf"
                                elide: Text.ElideLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            Button {
                                Layout.preferredWidth: implicitWidth
                                text: "Ouvrir"
                                onClicked: mainWindow.openTournament(index)
                            }

                            Button {
                                id: bt
                                Layout.preferredWidth: implicitWidth
                                icon.name: "trash"
                                icon.height: 16
                                onClicked: mainWindow.deleteTournament(index)
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {}
                }
            }
        }

        Button {
            Layout.alignment: Qt.AlignHCenter

            text: "Créer un nouveau tournoi"
            onClicked: mainWindow.newTournament()
        }
    }
}
