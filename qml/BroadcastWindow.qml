import QtQuick
import QtQuick.Controls

SwipeView {
    id: swipeView

    width: 800
    height: 600

    currentIndex: broadcastWindow.currentViewIndex

    onCurrentIndexChanged: {
        if (currentItem && currentItem.item) {
            if (currentItem.item.hasOwnProperty('startViewShow'))
                currentItem.item.startViewShow()
            else
                broadcastWindow.setCurrentViewTimer(currentTournament.timeBroadcastChange)
        }
    }

    Repeater {
        model: broadcastWindow.views

        Loader {
            clip: true
            property QtObject serieModel: viewSerie
            source: viewUrl
            onLoaded: {
                if (SwipeView.isCurrentItem) {
                    if (item.hasOwnProperty('startViewShow'))
                        item.startViewShow()
                    else
                        broadcastWindow.setCurrentViewTimer(currentTournament.timeBroadcastChange)
                }
            }
        }
    }
}
