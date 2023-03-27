import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    color: "#DADEDA"
    implicitHeight: col.implicitHeight

    ColumnLayout {
        id: col
        spacing: 0
        width: parent.width

        Rectangle {
            color: "#4fc1e9"
            Layout.preferredHeight: 28
            Layout.fillWidth: true

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignVCenter

                Item { height: 1; Layout.preferredWidth: 2 }

                SvgImage {
                    Layout.alignment: Qt.AlignCenter
                    source: "qrc:/img/broadcast.svg"
                    width: 18
                }

                Text {
                    leftPadding: 2
                    text: "Diffusion"
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

        }

        ToolBar {
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent

                ToolButtonTip {
                    icon.name: "play-button2"
                    onClicked: mainWindow.broadcastStart()
                    tooltipText: "Diffuser sur un ecran"
                    enabled: !mainWindow.broadcastActive
                    icon.color: enabled? "white" : "#a7d9eb"
                }

                ToolButtonTip {
                    icon.name: "stop-button2"
                    onClicked: mainWindow.broadcastStop()
                    tooltipText: "Arreter la diffusion"
                    enabled: mainWindow.broadcastActive
                    icon.color: enabled? "white" : "#a7d9eb"
                }

                ToolButtonTip {
                    icon.name: "previous"
                    onClicked: mainWindow.broadcastPrevious()
                    tooltipText: "Afficher le précédent"
                    enabled: mainWindow.broadcastActive
                    icon.color: enabled? "white" : "#a7d9eb"
                }

                ToolButtonTip {
                    icon.name: "next"
                    onClicked: mainWindow.broadcastNext()
                    tooltipText: "Afficher le suivant"
                    enabled: mainWindow.broadcastActive
                    icon.color: enabled? "white" : "#a7d9eb"
                }

                ToolButtonTip {
                    icon.name: "edit"
                    onClicked: mainWindow.broadcastEditInfo()
                    tooltipText: "Modifier le texte informatif"
                    icon.color: enabled? "white" : "#a7d9eb"
                }

                Item { Layout.fillWidth: true; height: 1 }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 300
            clip: true

            ListView {
                id: listSerie
                anchors.fill: parent

                model: mainWindow.broadcastViews
                currentIndex: mainWindow.currentBrodcastViewIndex

                delegate: ItemDelegate {
                    id: control

                    width: parent? parent.width: 0
                    height: 32
                    text: name

                    contentItem: RowLayout {
                        width: 300
                        height: 26
                        spacing: 4

                        Item { Layout.preferredWidth: 5; height: 1 }

                        Image {
                            Layout.preferredHeight: 20
                            Layout.preferredWidth: 20
                            source: mainWindow.currentBrodcastViewIndex === index?
                                        "qrc:/img/radio-button-selected.png" :
                                        "qrc:/img/radio-button.png"
                            fillMode: Image.PreserveAspectFit
                        }

                        Text {
                            Layout.fillWidth: true
                            rightPadding: control.spacing
                            text: control.text
                            font.bold: true
                            color: control.enabled ? "#0f4932" : "#bdbebf"
                            elide: Text.ElideMiddle
                            verticalAlignment: Text.AlignVCenter
                        }

                        Switch {
                            checked: viewVisible
                            onCheckedChanged: {
                                if (viewSerie) {
                                    viewSerie.viewVisible = checked
                                } else {
                                    viewVisible = checked
                                }
                            }
                        }

                        Item { Layout.preferredWidth: 5; height: 1 }
                    }

                    background: Rectangle {
                        border.color: listSerie.currentIndex == index?  "#489258": "#D7DED9"
                        border.width: 1
                        radius: 6
                        color: control.down ? "#eeeeee" :
                               listSerie.currentIndex == index? "#dedade":
                                                                "#fefefe"
                    }
                }

                ScrollBar.vertical: ScrollBar {}
            }
        }
    }
}
