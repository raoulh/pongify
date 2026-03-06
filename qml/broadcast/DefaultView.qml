import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#FAFAFA"

    function startViewShow() {
        flickable.initPosition()
    }

    Image {
        source: "qrc:/img/logo.svg"
        anchors {
            top: parent.top
            left: parent.left
            topMargin: 5
            leftMargin: 5
        }
    }

    Rectangle {
        id: titleBlock
        z: 100
        color: "#FAFAFA"
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: titleCol.y + titleCol.implicitHeight + 20

        Column {
            id: titleCol
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: parent.top
                topMargin: 50
            }
            spacing: 10

            Text {
                id: title
                text: currentTournament.name
                anchors.horizontalCenter: parent.horizontalCenter
                font {
                    pointSize: 30
                    bold: true
                }
            }
            Text {
                text: currentTournament.date.toLocaleString(Qt.locale("fr_FR"), "dddd, d MMMM yyyy")
                anchors.horizontalCenter: parent.horizontalCenter
                font {
                    pointSize: 20
                }
            }
        }
    }

    property real scrollTarget: 0
    property int scrollDownDuration: 0

    Flickable {
        id: flickable
        interactive: false

        function initPosition() {
            scrollAnimation.stop()
            contentY = 0

            if (flickable.contentHeight > flickable.height) {
                var distance = flickable.contentHeight - flickable.height
                var downMs = Math.max(1000, distance / currentTournament.broadcastScrollSpeed * 1000)
                var viewTime = Math.max(currentTournament.timeBroadcastChange, 1000 + downMs + 2000)

                scrollTarget = distance
                scrollDownDuration = downMs

                broadcastWindow.setCurrentViewTimer(viewTime)
                timerAnim.restart()
            } else {
                broadcastWindow.setCurrentViewTimer(currentTournament.timeBroadcastChange)
            }
        }

        Timer {
            id: timerAnim
            running: false
            repeat: false
            interval: 1000
            onTriggered: scrollAnimation.start()
        }

        SequentialAnimation {
            id: scrollAnimation
            loops: Animation.Infinite

            NumberAnimation {
                target: flickable; property: "contentY"
                to: scrollTarget; duration: scrollDownDuration
            }
            PauseAnimation { duration: 2000 }
            NumberAnimation {
                target: flickable; property: "contentY"
                to: 0; duration: scrollDownDuration
            }
            PauseAnimation { duration: 2000 }
        }

        anchors {
            left: parent.left; leftMargin: 40
            top: titleBlock.bottom; topMargin: 10
            bottom: parent.bottom; bottomMargin: 25
            right: parent.right; rightMargin: 40
        }

        boundsBehavior: Flickable.StopAtBounds
        contentWidth: contentCol.width
        contentHeight: contentCol.height

        Column {
            id: contentCol
            width: flickable.width
            spacing: 30

            Text {
                visible: currentTournament.infoText !== ""
                width: parent.width
                textFormat: Text.RichText
                text: currentTournament.infoText
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: 16
            }

            Column {
                width: Math.min(parent.width, 700)
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 12
                visible: currentTournament.series ? currentTournament.series.count > 0 : false

                Text {
                    text: "Programme"
                    font {
                        pointSize: 24
                        bold: true
                    }
                    color: "#4fc1e9"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Repeater {
                    model: currentTournament.series

                    Rectangle {
                        width: parent.width
                        height: serieRow.implicitHeight + 20
                        radius: 8
                        color: "#f0f4f8"
                        border.color: "#e2e8f0"
                        border.width: 1

                        RowLayout {
                            id: serieRow
                            anchors {
                                left: parent.left; leftMargin: 20
                                right: parent.right; rightMargin: 20
                                verticalCenter: parent.verticalCenter
                            }
                            spacing: 20

                            Text {
                                visible: qtObject.startTime !== ""
                                text: qtObject.startTime
                                font {
                                    pointSize: 22
                                    bold: true
                                }
                                color: "#4fc1e9"
                                Layout.preferredWidth: 80
                            }

                            Text {
                                text: qtObject.name
                                font.pointSize: 22
                                color: "#2d3748"
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Text {
                                text: qtObject.status === "playing" ? "En cours" :
                                      qtObject.status === "finished" ? "Terminé" :
                                      "En attente"
                                font {
                                    pointSize: 18
                                    italic: true
                                }
                                color: qtObject.status === "playing" ? "#38a169" :
                                       qtObject.status === "finished" ? "#d69e2e" :
                                       "#a0aec0"
                            }
                        }
                    }
                }
            }
        }
    }

    BroadcastQrOverlay {
        deepLink: "home"
    }
}
