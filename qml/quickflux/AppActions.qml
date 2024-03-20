pragma Singleton
import QtQuick
import QuickFlux
import "./"

QtObject {

    function showMatchSelector(tableNum) {
        AppDispatcher.dispatch(ActionTypes.showMatchSelector, { tableNum: tableNum });
    }

    function moveToMatchBlock(uid, round, match) {
        AppDispatcher.dispatch(ActionTypes.moveToMatchBlock, { serieUid: uid, round: round, match: match });
    }

    function selectSerie(idx) {
        AppDispatcher.dispatch(ActionTypes.selectSerie, { index: idx });
    }

    function flashMatchBlock(uid, round, match) {
        AppDispatcher.dispatch(ActionTypes.flashMatchBlock, { serieUid: uid, round: round, match: match });
    }
}
