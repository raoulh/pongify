import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

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

        contentWidth: brackerView.width
        contentHeight: brackerView.height

        BracketView {
            id: brackerView
        }
    }
}
