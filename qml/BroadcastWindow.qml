import QtQuick
import QtQuick.Controls

SwipeView {
    id: swipeView

    width: 800
    height: 600

    currentIndex: broadcastWindow.currentViewIndex

    Repeater {
        model: broadcastWindow.views

        Loader {
            property QtObject serieModel: viewSerie
            source: viewUrl
        }
    }
}
