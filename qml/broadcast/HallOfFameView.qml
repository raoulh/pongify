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

        Column {
            id: titleTxt

            Row {
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
    }

    Flickable {
        id: flickable

        function initPosition() {
            state = "top"
            contentX = 0
            if (flickable.contentItem.height > flickable.height) {
                timerAnim.restart()
            } else {
                //center Y
                contentY = Qt.binding(() => { return -(flickable.height - flickable.contentItem.height) / 2 })
            }

            if (flickable.contentItem.width < flickable.width) {
                //center X
                contentX = Qt.binding(() => { return -(flickable.width - flickable.contentItem.width) / 2 })
            }
        }

        Timer {
            id: timerAnim
            running: false
            repeat: false
            interval: 1000
            onTriggered: flickable.state = "bottom"
        }

        states: [
            State {
                name: "top"
                PropertyChanges {
                    target: flickable
                    contentY: 0
                }
            },
            State {
                name: "bottom"
                PropertyChanges {
                    target: flickable
                    contentY: contentHeight - flickable.height
                }
            }
        ]

        transitions: [
            Transition {
                from: "top"
                to: "bottom"
                NumberAnimation { target: flickable; properties: "contentY"; duration: 8000 }
            }
        ]

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

        contentWidth: hof.width
        contentHeight: hof.height

        HallOfFame {
            id: hof
            scaleFactor: 3
            winnerModel: viewSerie? viewSerie.winners: null
        }

    }
}
