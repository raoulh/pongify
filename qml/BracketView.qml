import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: rowRounds

    spacing: 30

    Repeater {
        model: selectedSerie.rounds

        ColumnLayout {
            id: colMatchs

            spacing: 20

            Repeater {
                id: roundRep
                property int roundIndex: index
                model: selectedSerie.matchCountForRound(roundIndex)

                Item {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.fillHeight: true
                    Layout.preferredHeight: children[0].height
                    Layout.preferredWidth: children[0].width

                    MatchBloc {
                        anchors.centerIn: parent

                        property int matchIndex: index
                        property QtObject player1: selectedSerie.getPlayer1(roundRep.roundIndex, matchIndex)
                        property QtObject player2: selectedSerie.getPlayer2(roundRep.roundIndex, matchIndex)

                        playerFirstName1: player1? player1.firstName: ""
                        playerLastName1: player1? player1.lastName: ""
                        playerRank1: player1? player1.ranking: ""

                        playerFirstName2: player2? player2.firstName: ""
                        playerLastName2: player2? player2.lastName: ""
                        playerRank2: player2? player2.ranking: ""

                        score1: selectedSerie.scoreForMatch(roundRep.roundIndex, matchIndex, 0)
                        score2: selectedSerie.scoreForMatch(roundRep.roundIndex, matchIndex, 1)

                        winner1: selectedSerie.winnerForMatch(roundRep.roundIndex, matchIndex, 0)
                        winner2: selectedSerie.winnerForMatch(roundRep.roundIndex, matchIndex, 1)
                    }
                }
            }
        }
    }
}
