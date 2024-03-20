import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QuickFlux
import "quickflux"

SplitView {

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
        id: right

        SplitView.minimumWidth: 260
        SplitView.preferredWidth: 260
        SplitView.maximumWidth: 600
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
