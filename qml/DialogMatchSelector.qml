import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "quickflux"

Dialog {
    id: dialog

    function show(cbAccepted, tableNum) {
        internal.fnAccepted = cbAccepted
        internal.tableNum = tableNum
        listMatch.currentIndex = 0
        dialog.open()
    }

    QtObject {
        id: internal

        property var fnAccepted: () => {}
        property int tableNum: 0
    }

    modal: true
    focus: true
    padding: 0

    x: (window.width - width) / 2
    y: (window.height - height) / 2

    width: Math.min(window.width, window.height) / 3 * 2
    height: Math.min(window.height * 0.85, 720)
    clip: true

    header: Rectangle {
        height: 56
        color: "#4fc1e9"
        radius: 2

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 12

            Text {
                text: "Sélectionner le match à démarrer"
                color: "white"
                font.pixelSize: 17
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            Rectangle {
                width: 36
                height: 36
                radius: 18
                color: "#3BAFDA"

                Text {
                    anchors.centerIn: parent
                    text: "#%1".arg(internal.tableNum)
                    color: "white"
                    font.pixelSize: 14
                    font.bold: true
                }
            }
        }
    }

    footer: Rectangle {
        height: 60
        color: "#F5F5F5"

        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: "#E0E0E0"
        }

        RowLayout {
            anchors.centerIn: parent
            anchors.rightMargin: 16
            spacing: 12

            // Cancel button
            Rectangle {
                width: 120
                height: 40
                radius: 4
                color: cancelMa.containsMouse ? "#F0F0F0" : "transparent"
                border.color: "#BBBBBB"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "Annuler"
                    color: "#666666"
                    font.pixelSize: 14
                }

                MouseArea {
                    id: cancelMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: dialog.reject()
                }
            }

            // OK button
            Rectangle {
                width: 140
                height: 40
                radius: 4
                color: startMa.pressed ? "#238a47" : startMa.containsMouse ? "#33b060" : "#2c9f59"

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 6

                    Text {
                        text: "▶"
                        color: "white"
                        font.pixelSize: 12
                    }

                    Text {
                        text: "Démarrer"
                        color: "white"
                        font.pixelSize: 15
                        font.bold: true
                    }
                }

                MouseArea {
                    id: startMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        internal.fnAccepted(listMatch.currentIndex)
                        dialog.close()
                    }
                }
            }
        }
    }

    background: Rectangle {
        color: "#F5F5F5"
        radius: 6

        // Re-draw header corners over the rounded top
        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 56
            color: "#4fc1e9"
            radius: 6

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 28
                color: "#4fc1e9"
            }
        }
    }

    ColumnLayout {
        id: col
        spacing: 0
        width: parent.width
        height: parent.height

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            color: "transparent"

            ListView {
                id: listMatch
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8
                clip: true

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }

                model: mainWindow.matchTableModel

                delegate: Item {
                    id: delegateRoot
                    width: listMatch.width
                    height: cardRect.height

                    required property int index
                    required property string serieName
                    required property bool isDouble
                    required property string player1_firstName
                    required property string player1_lastName
                    required property string player2_firstName
                    required property string player2_lastName
                    required property string player1Second_firstName
                    required property string player1Second_lastName
                    required property string player2Second_firstName
                    required property string player2Second_lastName

                    property bool isSelected: index === listMatch.currentIndex
                    property bool isHovered: cardMa.containsMouse

                    Rectangle {
                        id: cardRect
                        width: parent.width
                        height: cardContent.height
                        radius: 6
                        color: delegateRoot.isSelected ? "#F0FAF3" :
                               delegateRoot.isHovered ? "#FAFAFA" : "#FFFFFF"
                        border.color: delegateRoot.isSelected ? "#2c9f59" : "#E0E0E0"
                        border.width: delegateRoot.isSelected ? 2 : 1

                        RowLayout {
                            id: cardContent
                            width: parent.width
                            spacing: 0

                            // Left color bar indicator
                            Rectangle {
                                Layout.preferredWidth: 5
                                Layout.fillHeight: true
                                Layout.topMargin: 6
                                Layout.bottomMargin: 6
                                Layout.leftMargin: 1
                                radius: 3
                                color: delegateRoot.isSelected ? "#2c9f59" : "#D0D0D0"
                            }

                            // Match info
                            ColumnLayout {
                                Layout.fillWidth: true
                                Layout.margins: 12
                                Layout.leftMargin: 10
                                spacing: 4

                                Text {
                                    text: delegateRoot.serieName
                                    color: "#7AA4AC"
                                    font.pixelSize: 15
                                    font.bold: true
                                }

                                Row {
                                    Layout.fillWidth: true
                                    spacing: 6

                                    Text {
                                        text: delegateRoot.isDouble ?
                                                  "%1 %2 / %3 %4".arg(delegateRoot.player1_firstName).arg(delegateRoot.player1_lastName)
                                                      .arg(delegateRoot.player1Second_firstName).arg(delegateRoot.player1Second_lastName) :
                                                  "%1 %2".arg(delegateRoot.player1_firstName).arg(delegateRoot.player1_lastName)
                                        color: "#222222"
                                        font.pixelSize: 14
                                        font.bold: true
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Rectangle {
                                        width: vsLbl.width + 12
                                        height: vsLbl.height + 4
                                        radius: 3
                                        color: "#EDE7F6"
                                        anchors.verticalCenter: parent.verticalCenter

                                        Text {
                                            id: vsLbl
                                            anchors.centerIn: parent
                                            text: "vs"
                                            color: "#7E57C2"
                                            font.pixelSize: 11
                                            font.bold: true
                                            font.italic: true
                                        }
                                    }

                                    Text {
                                        text: delegateRoot.isDouble ?
                                                  "%1 %2 / %3 %4".arg(delegateRoot.player2_firstName).arg(delegateRoot.player2_lastName)
                                                      .arg(delegateRoot.player2Second_firstName).arg(delegateRoot.player2Second_lastName) :
                                                  "%1 %2".arg(delegateRoot.player2_firstName).arg(delegateRoot.player2_lastName)
                                        color: "#222222"
                                        font.pixelSize: 14
                                        font.bold: true
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }
                            }

                            // Checkmark indicator
                            Rectangle {
                                Layout.preferredWidth: 28
                                Layout.preferredHeight: 28
                                Layout.rightMargin: 12
                                Layout.alignment: Qt.AlignVCenter
                                radius: 14
                                color: delegateRoot.isSelected ? "#2c9f59" : "#E8E8E8"

                                Text {
                                    anchors.centerIn: parent
                                    text: "✓"
                                    color: delegateRoot.isSelected ? "white" : "#C0C0C0"
                                    font.pixelSize: 16
                                    font.bold: true
                                }
                            }
                        }

                        MouseArea {
                            id: cardMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: listMatch.currentIndex = delegateRoot.index
                        }
                    }
                }
            }

            // Empty state
            ColumnLayout {
                anchors.centerIn: parent
                visible: mainWindow.matchTableModel.count === 0
                spacing: 8

                Text {
                    text: "—"
                    color: "#BBBBBB"
                    font.pixelSize: 40
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Aucun match possible pour le moment"
                    color: "#999999"
                    font.pixelSize: 16
                    font.italic: true
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }
}
