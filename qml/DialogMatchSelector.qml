import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "quickflux"

Dialog {
    id: dialog

    function show(cbAccepted, tableNum) {
        internal.fnAccepted = cbAccepted

        dialog.title = title
        lbl.text = "Sélectionner le match à démarrer sur la table #%1".arg(tableNum)
        dialog.open()
    }

    QtObject {
        id: internal

        property var fnAccepted: () => {}
    }

    onAccepted: () => internal.fnAccepted(listMatch.currentIndex)

    modal: true
    focus: true

    x: (window.width - width) / 2
    y: (window.height - height) / 2

    width: Math.min(window.width, window.height) / 3 * 2
    contentHeight: col.height

    standardButtons: Dialog.Ok | Dialog.Cancel

    ButtonGroup {
         id: buttonGroup
     }

    ColumnLayout {
        id: col
        spacing: 20
        width: parent.width

        Label {
            id: lbl
            Layout.fillWidth: true
            wrapMode: Label.Wrap
            font.pixelSize: 16
            font.bold: true
        }

        Rectangle{
            Layout.preferredHeight: 600
            Layout.fillWidth: true
            clip: true
            color: "#FAFAFA"

            ListView {
                id: listMatch
                anchors.fill: parent

                ScrollBar.vertical: ScrollBar {}

                model: ListModel {
                    ListElement { serieName: "NC - 65"; player1_firstName: "Pierre"; player1_lastName: "Lapointe"; player2_firstName: "Jean"; player2_lastName: "Tremblay" }
                    ListElement { serieName: "NC - 65"; player1_firstName: "Pierre"; player1_lastName: "Lapointe"; player2_firstName: "Jean"; player2_lastName: "Tremblay" }
                    ListElement { serieName: "NC - 65"; player1_firstName: "Pierre"; player1_lastName: "Lapointe"; player2_firstName: "Jean"; player2_lastName: "Tremblay" }
                    ListElement { serieName: "NC - 65"; player1_firstName: "Pierre"; player1_lastName: "Lapointe"; player2_firstName: "Jean"; player2_lastName: "Tremblay" }
                }

                delegate: RadioDelegate {
                    id: control
                    width: parent? parent.width: 0
                    height: contentItem.height

                    checked: false
                    ButtonGroup.group: buttonGroup

                    onClicked: {
                        listMatch.currentIndex = index
                    }

                    contentItem: Item {
                        width: parent.width
                        height: colMatch.height

                        ColumnLayout {
                            id: colMatch
                            width: parent.width

                            Item { Layout.preferredHeight: 5; width: 1 }

                            Text {
                                text: "Série: %1".arg(serieName)
                                font.pixelSize: 16
                                font.bold: true
                            }

                            Text {
                                text: "%1 %2 vs %3 %4".arg(player1_firstName).arg(player1_lastName).arg(player2_firstName).arg(player2_lastName)
                                font.pixelSize: 14
                            }

                            Item { Layout.preferredHeight: 5; width: 1 }
                        }
                    }

                    background: Rectangle {
                        implicitWidth: 100
                        implicitHeight: parent.contentItem.height
                        color: control.down ? "#eeeeee" :
                               listMatch.currentIndex === index? "#dedade":
                                                                "#fefefe"
                    }
                }
            }
        }
    }
}
