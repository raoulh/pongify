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
                    text: "Gestion des tables"
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
                    icon.name: "add"
                    onClicked: mainWindow.newTable()
                    tooltipText: "Ajouter une nouvelle table"
                }

                ToolButtonTip {
                    icon.name: "trash"
                    onClicked: mainWindow.deleteTable()
                    tooltipText: "Supprimer une table"
                }


                Item { Layout.fillWidth: true; height: 1 }

                ToolButtonTip {
                    icon.name: "play-button2"
                    onClicked: mainWindow.selectMatchTable(listTable.currentIndex)
                    tooltipText: "Démarrer un match sur la table sélectionnée"
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ListView {
                id: listTable
                anchors.fill: parent

                model: currentTournament? currentTournament.tables: undefined

                delegate: ItemDelegate {
                    id: control

                    width: parent? parent.width: 0
                    height: contentItem.height
                    text: tableNumber

                    onClicked: {
                        listTable.currentIndex = index
                        mainWindow.selectTable(index)
                    }

                    contentItem: Item {
                        width: parent.width
                        height: colTable.height

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.RightButton
                            onClicked: (mouse) => {
                                if (mouse.button === Qt.RightButton) {
                                    mainWindow.showTableMenu(index)
                                }
                            }
                        }

                        ColumnLayout {
                            id: colTable
                            width: parent.width
                            spacing: 0

                            Item { Layout.fillWidth: true; height: 8 }

                            RowLayout {
                                height: rect.height

                                Rectangle {
                                    id: rect
                                    Layout.preferredHeight: tableNumTxt.implicitHeight + 8
                                    Layout.preferredWidth: tableNumTxt.implicitWidth + 8
                                    color: listTable.currentIndex == index ? "#de8787" : "#2c9f59"

                                    Text {
                                        id: tableNumTxt
                                        anchors.centerIn: parent
                                        text: "Table #%1".arg(control.text)
                                        font.bold: true
                                        color: "white"
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                Text {
                                    Layout.fillWidth: true

                                    text: free? "": serieName
                                    font.bold: false
                                    color: "#4d4d4d"
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    text: free? "Libre" : "En cours"
                                    font.bold: false
                                    color: "#808080"
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Item { Layout.preferredWidth: 4; height: 1 }
                            }

                            RowLayout {
                                spacing: 0
                                height: colA.implicitHeight

                                Rectangle {
                                    id: rectA
                                    height: colA.implicitHeight + 16
                                    Layout.fillWidth: true

                                    color: "#2c9f59"

                                    opacity: free? 0.6: 1.0

                                    ColumnLayout {
                                        id: colA
                                        anchors {
                                            margins: 8
                                            fill: parent
                                        }

                                        Text {
                                            text: free? "": player1_firstName
                                            elide: Text.ElideRight
                                            font.pointSize: 12
                                            color: "white"
                                            width: rectA.width - 16
                                        }

                                        Text {
                                            text: free? "": player1_lastName
                                            elide: Text.ElideRight
                                            font.pointSize: 12
                                            color: "white"
                                            width: rectA.width - 16
                                        }
                                    }
                                }

                                Rectangle {
                                    color: "white"
                                    opacity: free? 0.6: 1.0
                                    Layout.preferredWidth: 4
                                    height: colA.implicitHeight + 16
                                }

                                Rectangle {
                                    id: rectB
                                    color: "#2c9f59"

                                    opacity: free? 0.6: 1.0

                                    height: colA.implicitHeight + 16
                                    Layout.fillWidth: true

                                    ColumnLayout {
                                        anchors {
                                            margins: 8
                                            fill: parent
                                        }

                                        Text {
                                            text: free? "": player2_firstName
                                            elide: Text.ElideRight
                                            font.pointSize: 12
                                            color: "white"
                                            width: rectB.width - 16
                                        }

                                        Text {
                                            text: free? "": player2_lastName
                                            elide: Text.ElideRight
                                            font.pointSize: 12
                                            color: "white"
                                            width: rectB.width - 16
                                        }
                                    }
                                }
                            }
                        }
                    }

                    background: Rectangle {
                        color: control.down ? "#eeeeee" :
                               listTable.currentIndex === index? "#dedade":
                                                                "#fefefe"
                    }
                }

                ScrollBar.vertical: ScrollBar {}
            }
        }
    }
}
