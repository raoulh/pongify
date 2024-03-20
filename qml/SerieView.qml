import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QuickFlux
import "quickflux"

Rectangle {
    color: "#FAFAFA"
    clip: true

    Rectangle {
        id: title
        z: 100
        color: "#FAFAFA"
        anchors {
            top: parent.top
            left: parent.left
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
                text: selectedSerie? "Série: %1".arg(selectedSerie.name): ""
                elide: Text.ElideRight
                color: "#7AA4AC"
                font {
                    pointSize: 20
                    bold: true
                }
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                padding: 10
                text: selectedSerie?
                          selectedSerie.status === "stopped"? "[Non démarré]" :
                          selectedSerie.status === "playing"? "[Jeu en cours]" :
                          selectedSerie.status === "finished"? "[Terminé]" :
                                                              "" : ""
                elide: Text.ElideRight
                color: "#79A3AB"
                font {
                    pointSize: 18
                    bold: false
                    italic: true
                }
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Flickable {
        id: flickable
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

            serie: selectedSerie
            isEditable: true
        }

        NumberAnimation {
            id: animationX
            target: flickable
            property: "contentX"
            duration: 500
            easing.type: Easing.InOutQuad
        }

        NumberAnimation {
            id: animationY
            target: flickable
            property: "contentY"
            duration: 500
            easing.type: Easing.InOutQuad
        }

        function moveToPosition(x, y, rectWidth, rectHeight) {
            var targetX = x + rectWidth / 2 - flickable.width / 2;
            var targetY = y + rectHeight / 2 - flickable.height / 2;

            targetX = Math.max(0, Math.min(targetX, flickable.contentWidth - flickable.width));
            targetY = Math.max(0, Math.min(targetY, flickable.contentHeight - flickable.height));

            animationX.to = targetX
            animationY.to = targetY
            animationX.restart()
            animationY.restart()
        }
    }

    AppListener {
        Filter {
            type: ActionTypes.moveToMatchBlock
            onDispatched: (filtertype, message) => {
                              if (message.serieUuid !== selectedSerie.uuid) return
                              let pos = bracketView.findMatchBlocPosition(message.round, message.match)
                              flickable.moveToPosition(pos.x, pos.y, pos.width, pos.height)
                          }
        }
    }
}
