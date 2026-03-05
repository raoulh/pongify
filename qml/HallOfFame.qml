import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ColumnLayout {
    id: hof
    property real scaleFactor: 1.0
    property bool temporary: false
    property bool isDouble: false

    property QtObject winnerModel

    function sc(v) {
        return scaleFactor * v
    }

    width: sc(300)
    spacing: sc(4)

    Rectangle {
        Layout.preferredHeight: rowHeader.height
        Layout.preferredWidth: sc(250)

        color: "#8b8b8b"

        RowLayout {
            id: rowHeader
            Image {
                Layout.preferredHeight: hof.sc(20)
                Layout.preferredWidth: hof.sc(20)
                Layout.margins: hof.sc(10)
                source: "qrc:/img/podium.png"
                fillMode: Image.PreserveAspectFit
            }

            Text {
                Layout.fillWidth: true
                Layout.margins: hof.sc(10)
                text: temporary? "Podium provisoire": "Podium"
                font.bold: true
                color: "white"
                elide: Text.ElideMiddle
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignRight
                font {
                    pointSize: sc(12)
                }
            }
        }
    }

    Repeater {
        model: winnerModel

        RowLayout {
            Layout.preferredWidth: sc(300)
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: sc(36)
                Layout.margins: hof.sc(5)

                radius: hof.sc(4)
                color: "#eed67d"

                RowLayout {
                    id: rowName
                    width: parent.width
                    anchors.centerIn: parent

                    Text {
                        leftPadding: hof.sc(10)
                        text: index + 1
                        font.bold: true
                        color: "#b79217"
                        elide: Text.ElideMiddle
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        font {
                            pointSize: sc(12)
                        }
                    }

                    Text {
                        rightPadding: hof.sc(10)
                        Layout.fillWidth: true
                        text: hof.isDouble && firstNameSecond !== ""?
                                  "%1 %2 / %3 %4".arg(firstName).arg(lastName).arg(firstNameSecond).arg(lastNameSecond):
                                  "%1 %2".arg(firstName).arg(lastName)
                        font.bold: true
                        color: "black"
                        elide: Text.ElideMiddle
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignRight
                        font {
                            pointSize: sc(12)
                            bold: true
                        }
                    }
                }
            }

            Text {
                Layout.margins: hof.sc(10)
                text: ranking
                font.bold: true
                color: "#b79217"
                elide: Text.ElideMiddle
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font {
                    pointSize: sc(12)
                }
            }
        }
    }
}
