import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QuickFlux
import "quickflux"

Rectangle {
    id: bloc

    property bool isEditable: false
    property real scaleFactor: 1.0

    function sc(v) {
        return scaleFactor * v
    }

    property string playerFirstName1
    property string playerLastName1
    property string playerRank1
    property string playerClub1
    property string playerFirstNameSecond1
    property string playerLastNameSecond1
    property string playerRankSecond1

    property string playerFirstName2
    property string playerLastName2
    property string playerRank2
    property string playerClub2
    property string playerFirstNameSecond2
    property string playerLastNameSecond2
    property string playerRankSecond2

    property string score1
    property string score2

    property bool winner1
    property bool winner2

    property bool isDouble: false
    property bool isHandicap: false

    property int handicap1: 0
    property int handicap2: 0

    property int roundIdx: 0
    property int matchIdx: 0

    signal clicked()

    function flash() {
        flashAnim.start()
    }

    SequentialAnimation {
        id: flashAnim

        PauseAnimation {
            duration: 400
        }

        ScriptAction {
            script: flashRect.visible = true
        }

        ColorAnimation {
            target: flashRect.border
            property: "color"
            from: "#ffffff00"
            to: "#ff0000"
            duration: 160
            easing.type: Easing.Linear
        }

        ColorAnimation {
            target: flashRect.border
            property: "color"
            from: "#ff0000"
            to: "#ffffff00"
            duration: 160
            easing.type: Easing.Linear
        }

        ColorAnimation {
            target: flashRect.border
            property: "color"
            from: "#ffffff00"
            to: "#ff0000"
            duration: 160
            easing.type: Easing.Linear
        }

        ColorAnimation {
            target: flashRect.border
            property: "color"
            from: "#ff0000"
            to: "#ffffff00"
            duration: 160
            easing.type: Easing.Linear
        }

        ColorAnimation {
            target: flashRect.border
            property: "color"
            from: "#ffffff00"
            to: "#ff0000"
            duration: 160
            easing.type: Easing.Linear
        }

        ColorAnimation {
            target: flashRect.border
            property: "color"
            from: "#ff0000"
            to: "#ffffff00"
            duration: 160
            easing.type: Easing.Linear
        }

        ScriptAction {
            script: flashRect.visible = false
        }
    }

    color: bloc.mouseHovered? "#585848": playerFirstName1 == "" || winner2? "#b2b2b2": "#484848"
    radius: sc(6)

    implicitHeight: sc(110)
    implicitWidth: isDouble || isEditable ? sc(350) : sc(250)

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
                Layout.leftMargin: sc(6)
                Layout.rightMargin: sc(6)
                Layout.topMargin: sc(5)
                Layout.bottomMargin: sc(5)

                RowLayout {
                    spacing: 0
                    Text {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: playerFirstName1 == ""? "-": isDouble?
                                                          "%1 %2".arg(bloc.playerFirstName1).arg(bloc.playerLastName1):
                                                          playerFirstName1
                        color: "#ffffff"
                        elide: Text.ElideRight
                        font {
                            pointSize: sc(12)
                        }
                    }
                    Image {
                        source: "qrc:/img/medal.png"
                        visible: winner1

                        Layout.preferredHeight: sc(20)
                        Layout.preferredWidth: sc(20)
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        Layout.fillHeight: true
                        Layout.preferredWidth: implicitWidth
                        text: "+ %1".arg(handicap1)
                        color: "#84fa78"
                        font {
                            pointSize: sc(12)
                            bold: true
                        }
                        visible: isHandicap && handicap1 > 0
                    }

                    Item {
                        Layout.preferredWidth: sc(15)
                        height: 1
                        visible: isHandicap && handicap1 > 0
                    }

                    Text {
                        Layout.fillHeight: true
                        Layout.preferredWidth: implicitWidth
                        text: playerRank1
                        color: "#eed67d"
                        font {
                            pointSize: sc(12)
                        }
                    }
                }

                RowLayout {
                    spacing: 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Text {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: playerLastName1 == ""? "-": isDouble?
                                                         "%1 %2".arg(bloc.playerFirstNameSecond1).arg(bloc.playerLastNameSecond1):
                                                         playerLastName1
                        color: "#ffffff"
                        elide: Text.ElideRight
                        font {
                            pointSize: sc(12)
                        }
                    }
                    Text {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: playerClub1
                        color: "#cccccc"
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignTop
                        visible: !isDouble && isEditable
                        font {
                            pointSize: sc(10)
                            italic: true
                        }
                    }
                    Text {
                        Layout.fillHeight: true
                        Layout.preferredWidth: implicitWidth
                        text: playerRankSecond1
                        color: "#eed67d"
                        font {
                            pointSize: sc(12)
                        }
                        visible: isDouble
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
                Layout.leftMargin: sc(6)
                Layout.rightMargin: sc(6)
                Layout.topMargin: sc(5)
                Layout.bottomMargin: sc(5)

                RowLayout {
                    spacing: 0

                    Text {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: playerFirstName2 == ""? "-": isDouble?
                                                          "%1 %2".arg(bloc.playerFirstName2).arg(bloc.playerLastName2):
                                                          playerFirstName2
                        color: "#ffffff"
                        elide: Text.ElideRight
                        font {
                            pointSize: sc(12)
                        }
                    }
                    Image {
                        source: "qrc:/img/medal.png"
                        visible: winner2

                        Layout.preferredHeight: sc(20)
                        Layout.preferredWidth: sc(20)
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        Layout.fillHeight: true
                        Layout.preferredWidth: implicitWidth
                        text: "+ %1".arg(handicap2)
                        color: "#84fa78"
                        font {
                            pointSize: sc(12)
                            bold: true
                        }
                        visible: isHandicap && handicap2 > 0
                    }

                    Item {
                        Layout.preferredWidth: sc(15)
                        height: 1
                        visible: isHandicap && handicap2 > 0
                    }

                    Text {
                        Layout.fillHeight: true
                        Layout.preferredWidth: implicitWidth
                        text: playerRank2
                        color: "#eed67d"
                        font {
                            pointSize: sc(12)
                        }
                    }
                }

                RowLayout {
                    spacing: 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Text {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: playerLastName2 == ""? "-": isDouble?
                                                         "%1 %2".arg(bloc.playerFirstNameSecond2).arg(bloc.playerLastNameSecond2):
                                                         playerLastName2
                        color: "#ffffff"
                        elide: Text.ElideRight
                        font {
                            pointSize: sc(12)
                        }
                    }
                    Text {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: playerClub2
                        color: "#cccccc"
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignTop
                        visible: !isDouble && isEditable
                        font {
                            pointSize: sc(10)
                            italic: true
                        }
                    }
                    Text {
                        Layout.fillHeight: true
                        Layout.preferredWidth: implicitWidth
                        text: playerRankSecond2
                        color: "#eed67d"
                        font {
                            pointSize: sc(12)
                        }
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
            Layout.preferredWidth: sc(40)
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
                        pointSize: sc(16)
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
                        pointSize: sc(16)
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

        enabled: isEditable

        hoverEnabled: true
        onEntered: bloc.mouseHovered = true
        onExited: bloc.mouseHovered = false
        onClicked: bloc.clicked()
    }

    Rectangle {
        id: flashRect
        color: "transparent"
        anchors.fill: parent
        radius: parent.radius
        visible: false
        border.width: sc(5)
    }

    AppListener {
        Filter {
            type: ActionTypes.flashMatchBlock
            onDispatched: (filtertype, message) => {
                              if (message.serieUuid !== selectedSerie.uuid) return
                              if (message.round !== bloc.roundIdx || message.match !== bloc.matchIdx) return
                              bloc.flash()
                          }
        }
    }
}
