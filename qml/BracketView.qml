import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: rowRounds

    spacing: 30

    Repeater {
        model: selectedSerie? selectedSerie.rounds: 0

        ColumnLayout {
            id: colMatchs

            spacing: 20

            Repeater {
                id: roundRep
                property int roundIndex: index
                model: selectedSerie? selectedSerie.matchCountForRound(roundIndex): 0

                Item {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.fillHeight: true
                    Layout.preferredHeight: children[0].height
                    Layout.preferredWidth: children[0].width

                    MatchBloc {
                        anchors.centerIn: parent

                        QtObject {
                            id: emptyPlayer
                            property string firstName
                            property string lastName
                            property string ranking
                        }

                        property int matchIndex: index
                        property QtObject player1: selectedSerie && selectedSerie.getPlayer1(roundRep.roundIndex, matchIndex)?
                                                     selectedSerie.getPlayer1(roundRep.roundIndex, matchIndex):
                                                       emptyPlayer
                        property QtObject player2: selectedSerie && selectedSerie.getPlayer2(roundRep.roundIndex, matchIndex)?
                                                       selectedSerie.getPlayer2(roundRep.roundIndex, matchIndex):
                                                         emptyPlayer

                        playerFirstName1: player1.firstName
                        playerLastName1: player1.lastName
                        playerRank1: player1.ranking

                        playerFirstName2: player2.firstName
                        playerLastName2: player2.lastName
                        playerRank2: player2.ranking

                        score1: selectedSerie? selectedSerie.scoreForMatch(roundRep.roundIndex, matchIndex, 0): ""
                        score2: selectedSerie? selectedSerie.scoreForMatch(roundRep.roundIndex, matchIndex, 1): ""

                        winner1: selectedSerie? selectedSerie.winnerForMatch(roundRep.roundIndex, matchIndex, 0): false
                        winner2: selectedSerie? selectedSerie.winnerForMatch(roundRep.roundIndex, matchIndex, 1): false
                    }
                }
            }
        }
    }
}
