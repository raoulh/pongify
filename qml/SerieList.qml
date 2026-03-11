import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QuickFlux
import "quickflux"

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
                    text: currentTournament? currentTournament.name: ""
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
                    text: currentTournament? currentTournament.date.toLocaleString(Qt.locale("fr_FR"), "dddd, d MMMM yyyy"): ""
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

                ToolButtonTip {
                    icon.name: "casino2"
                    onClicked: mainWindow.createPools()
                    tooltipText: "Créer des poules automatiquement"
                }

                Item { Layout.fillWidth: true; height: 1 }
            }
        }

        SplitView {
            orientation: Qt.Vertical
            Layout.fillWidth: true
            Layout.fillHeight: true

            Item {
                SplitView.fillHeight: true
                SplitView.minimumHeight: 80
                clip: true

                ListView {
                    id: listSerie
                    anchors.fill: parent

                    model: currentTournament? currentTournament.series: undefined

                    delegate: ItemDelegate {
                        id: control

                        width: parent? parent.width: 0
                        height: bt.implicitHeight + 10
                        text: name

                        onClicked: AppActions.selectSerie(index)

                        contentItem: RowLayout {
                            width: 300
                            spacing: 4

                            Item { Layout.preferredWidth: 5; height: 1 }

                            Image {
                                Layout.preferredHeight: 20
                                Layout.preferredWidth: 20
                                source: qtObject.status == "playing"?
                                            "qrc:/icons/pongify/32x32@4/play-button.png" :
                                            "qrc:/img/podium.png"
                                fillMode: Image.PreserveAspectFit

                                visible: qtObject.status != "stopped"
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

                            Text {
                                text: qtObject.startTime
                                visible: qtObject.startTime !== ""
                                color: "#666666"
                                font.pointSize: 10
                                verticalAlignment: Text.AlignVCenter
                            }

                            Button {
                                id: bt
                                Layout.preferredWidth: implicitWidth
                                icon.name: "menu"
                                icon.height: 16
                                onClicked: {
                                    AppActions.selectSerie(index)
                                    mainWindow.showSerieMenu(index)
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

            BroadcastControls {
                SplitView.minimumHeight: 80
                SplitView.preferredHeight: 250
            }
        }
    }

    AppListener {
        Filter {
            type: ActionTypes.selectSerie
            onDispatched: (filtertype, message) => {
                              listSerie.currentIndex = message.index
                              mainWindow.selectSerie(message.index)
                          }
        }
    }
}
