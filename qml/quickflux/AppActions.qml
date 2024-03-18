pragma Singleton
import QtQuick
import QuickFlux
import "./"

QtObject {

    function showMatchSelector(tableNum) {
        AppDispatcher.dispatch(ActionTypes.showMatchSelector, { tableNum: tableNum });
    }

}
