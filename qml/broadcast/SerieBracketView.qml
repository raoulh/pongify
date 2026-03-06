import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Rectangle {
    color: "#FAFAFA"

    function startViewShow() {
        flickable.initPosition()
    }

    Rectangle {
        id: title
        z: 100
        color: "#FAFAFA"
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
            margins: 10
        }
        width: titleTxt.implicitWidth
        height: titleTxt.implicitHeight
        radius: 10

        Row {
            id: titleTxt
            spacing: 20

            Text {

                padding: 10
                text: viewSerie? "Série: %1".arg(viewSerie.name): ""
                elide: Text.ElideRight
                color: "#7AA4AC"
                font {
                    pointSize: 38
                    bold: true
                }
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                padding: 10
                text: viewSerie?
                          viewSerie.status === "stopped"? "[Non démarré]" :
                          viewSerie.status === "playing"? "[Jeu en cours]" :
                          viewSerie.status === "finished"? "[Terminé]" :
                                                              "" : ""
                elide: Text.ElideRight
                color: "#79A3AB"
                font {
                    pointSize: 38
                    bold: false
                    italic: true
                }
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Text {
        anchors {
            top: title.bottom
            horizontalCenter: title.horizontalCenter
            topMargin: 50
        }

        padding: 10
        text: "Finale"
        visible: viewSerie && viewSerie.status === "playing" && viewSerie.currentRound === viewSerie.rounds - 1
        elide: Text.ElideRight
        color: "#7AA4AC"
        font {
            pointSize: 46
            bold: true
        }
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }


    property real scrollTarget: 0
    property int scrollDownDuration: 0

    Flickable {
        id: flickable
        interactive: false

        function initPosition() {
            scrollAnimation.stop()
            contentY = 0
            contentX = 0

            if (flickable.contentHeight > flickable.height) {
                var distance = flickable.contentHeight - flickable.height
                var downMs = Math.max(1000, distance / currentTournament.broadcastScrollSpeed * 1000)
                var cycleTime = downMs + 2000 + downMs + 2000
                var viewTime = Math.max(currentTournament.timeBroadcastChange, 1000 + downMs + 2000)

                scrollTarget = distance
                scrollDownDuration = downMs

                broadcastWindow.setCurrentViewTimer(viewTime)
                timerAnim.restart()
            } else {
                //center Y
                contentY = Qt.binding(() => { return -(flickable.height - flickable.contentItem.height) / 2 })
                broadcastWindow.setCurrentViewTimer(currentTournament.timeBroadcastChange)
            }

            if (flickable.contentWidth < flickable.width) {
                //center X
                contentX = Qt.binding(() => { return -(flickable.width - flickable.contentItem.width) / 2 })
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
            left: parent.left; leftMargin: 25
            top: title.bottom; topMargin: 15
            bottom: parent.bottom; bottomMargin: 25
            right: parent.right; rightMargin: 25
        }

        ScrollIndicator.vertical: ScrollIndicator {
            parent: flickable.parent
            anchors {
                top: flickable.top
                left: flickable.right; leftMargin: 20
                bottom: flickable.bottom
            }
        }
        ScrollIndicator.horizontal: ScrollIndicator {
            parent: flickable.parent
            anchors {
                left: flickable.left
                right: flickable.right
                top: flickable.bottom; topMargin: 20
            }
        }

        boundsBehavior: Flickable.StopAtBounds

        contentWidth: bracketView.width
        contentHeight: bracketView.height

        BracketView {
            id: bracketView

            serie: viewSerie
            scaleFactor: 2

            startRoundIdx: viewSerie? viewSerie.currentRound === viewSerie.rounds - 1? viewSerie.currentRound - 1: viewSerie.currentRound: 0
        }
    }

    BroadcastQrOverlay {
        deepLink: "serie/" + viewSerieIndex + "/round/" + (viewSerie ? viewSerie.currentRound : 0)
    }
}
