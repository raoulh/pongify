import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QuickFlux
import "quickflux"

Item {

    SplitView {
        anchors.fill: parent

        SerieList {
            id: left

            SplitView.minimumWidth: 260
            SplitView.preferredWidth: 260
            SplitView.maximumWidth: 600
        }

        SerieView {
            id: center

            SplitView.fillWidth: true
        }

        TableMgmt {
            id: tableMgmt

            property int minWidth: 260
            property int prefWidth: 260
            property int maxWidth: 600

            SplitView.minimumWidth: minWidth
            SplitView.preferredWidth: prefWidth
            SplitView.maximumWidth: maxWidth

            function toggle() {
                if (tableMgmt.minWidth === 0) {
                    tableMgmt.minWidth = 260
                    tableMgmt.prefWidth = 260
                } else {
                    tableMgmt.minWidth = 0
                    tableMgmt.prefWidth = 0
                }
            }
        }
    }

    Text {
        color: "#dedade"
        width: 35
        height: 35
        x: tableMgmt.x - width
        y: tableMgmt.y

        font {
            bold: true
            pointSize: 20
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        text: tableMgmt.minWidth === 0? "<" : ">"

        MouseArea {
            anchors.fill: parent
            onClicked: {
                tableMgmt.toggle()
            }
        }
    }

    AppListener {
        Filter {
            type: ActionTypes.showMatchSelector
            onDispatched: (filtertype, message) => {
                              dialogMatchSelector.show((idx) => {
                                  mainWindow.startMatchOnTable(idx, message.tableNum)
                              }, message.tableNum)
                          }
        }
    }

    DialogMatchSelector {
        id: dialogMatchSelector
    }
}
