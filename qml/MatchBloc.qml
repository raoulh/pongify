import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    id: bloc

    property string playerFirstName1
    property string playerLastName1
    property string playerRank1
    property string playerFirstName2
    property string playerLastName2
    property string playerRank2

    property string score1
    property string score2

    property bool winner1
    property bool winner2

    color: bloc.mouseHovered? "#585848": playerFirstName1 == "" || winner2? "#b2b2b2": "#484848"
    radius: 6

    implicitHeight: 110
    implicitWidth: 250

    property bool mouseHovered: false

    //Bottom rect for background color of player 2
    RoundedRectangle {
        anchors {
            fill: parent
            topMargin: bloc.height / 2
        }
        topLeftRounded: false
        topRightRounded: false
        radius: bloc.radius
        color: bloc.mouseHovered? "#585848": playerFirstName2 == "" || winner1? "#b2b2b2": "#484848"
    }

    RowLayout {
        id: baseRow
        spacing: 0
        anchors.fill: parent

        //players
        ColumnLayout {
            spacing: 0

            ColumnLayout {
                spacing: 0
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                Layout.topMargin: 5
                Layout.bottomMargin: 5

                RowLayout {
                    spacing: 0
                    Text {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: playerFirstName1 == ""? "-": playerFirstName1
                        color: "#ffffff"
                        elide: Text.ElideRight
                        font {
                            pointSize: 12
                        }
                    }
                    Image {
                        source: "qrc:/img/medal.png"
                        visible: winner1

                        Layout.preferredHeight: 20
                        Layout.preferredWidth: 20
                        fillMode: Image.PreserveAspectFit
                    }
                    Text {
                        Layout.fillHeight: true
                        text: playerRank1
                        color: "#eed67d"
                        font {
                            pointSize: 12
                        }
                    }
                }

                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: playerLastName1 == ""? "-": playerLastName1
                    color: "#ffffff"
                    elide: Text.ElideRight
                    font {
                        pointSize: 12
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#eed67d"
            }

            ColumnLayout {
                spacing: 0
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                Layout.topMargin: 5
                Layout.bottomMargin: 5

                RowLayout {
                    spacing: 0

                    Text {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: playerFirstName2 == ""? "-": playerFirstName2
                        color: "#ffffff"
                        elide: Text.ElideRight
                        font {
                            pointSize: 12
                        }
                    }
                    Image {
                        source: "qrc:/img/medal.png"
                        visible: winner2

                        Layout.preferredHeight: 20
                        Layout.preferredWidth: 20
                        fillMode: Image.PreserveAspectFit
                    }
                    Text {
                        Layout.fillHeight: true
                        text: playerRank2
                        color: "#eed67d"
                        font {
                            pointSize: 12
                        }
                    }
                }

                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: playerLastName2 == ""? "-": playerLastName2
                    color: "#ffffff"
                    elide: Text.ElideRight
                    font {
                        pointSize: 12
                    }
                }
            }

        }

        //Score
        RoundedRectangle {
            topLeftRounded: false
            bottomLeftRounded: false
            color: winner2 || playerFirstName1 == ""? "#faf1d0" : "#eed67d"
            radius: bloc.radius
            Layout.preferredWidth: 40
            Layout.preferredHeight: bloc.height

            RoundedRectangle {
                anchors {
                    fill: parent
                    topMargin: parent.height / 2
                }
                topLeftRounded: false
                topRightRounded: false
                bottomLeftRounded: false
                radius: parent.radius
                color: winner1 || playerFirstName2 == ""? "#faf1d0" : "#eed67d"
            }

            ColumnLayout {
                spacing: 0
                anchors.fill: parent

                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignCenter
                    text: score1 == "" || score1 == "-1" ? "-": score1
                    color: winner2? "#8b8b8b" : "#000000"
                    font {
                        pointSize: 16
                        bold: true
                    }
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                }

                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignCenter
                    text: score2 == "" || score2 == "-1"? "-": score2
                    color: winner1? "#8b8b8b" : "#000000"
                    font {
                        pointSize: 16
                        bold: true
                    }
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                }
            }
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent

        hoverEnabled: true
        onEntered: bloc.mouseHovered = true
        onExited: bloc.mouseHovered = false
    }
}
