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
                        id: matchBloc
                        anchors.centerIn: parent

                        QtObject {
                            id: emptyPlayer
                            property string firstName
                            property string lastName
                            property string ranking
                            property string firstNameSecond
                            property string lastNameSecond
                            property string rankingSecond
                        }

                        property int matchIndex: index
                        property QtObject player1: emptyPlayer
                        property QtObject player2: emptyPlayer

                        playerFirstName1: player1.firstName
                        playerLastName1: player1.lastName
                        playerRank1: player1.ranking
                        playerFirstNameSecond1: player1.firstNameSecond
                        playerLastNameSecond1: player1.lastNameSecond
                        playerRankSecond1: player1.rankingSecond

                        playerFirstName2: player2.firstName
                        playerLastName2: player2.lastName
                        playerRank2: player2.ranking
                        playerFirstNameSecond2: player2.firstNameSecond
                        playerLastNameSecond2: player2.lastNameSecond
                        playerRankSecond2: player2.rankingSecond

                        isDouble: selectedSerie? selectedSerie.isDouble: false

                        onClicked: if (selectedSerie) selectedSerie.clickedOnMatch(roundRep.roundIndex, matchIndex)

                        function updateMatchData() {
                            matchBloc.score1 = selectedSerie.scoreForMatch(roundRep.roundIndex, matchIndex, 0)
                            matchBloc.score2 = selectedSerie.scoreForMatch(roundRep.roundIndex, matchIndex, 1)
                            matchBloc.winner1 = selectedSerie.winnerForMatch(roundRep.roundIndex, matchIndex, 0)
                            matchBloc.winner2 = selectedSerie.winnerForMatch(roundRep.roundIndex, matchIndex, 1)

                            let player = selectedSerie.getPlayer1(roundRep.roundIndex, matchIndex)
                            if (!player) player = emptyPlayer
                            matchBloc.player1 = player

                            player = selectedSerie.getPlayer2(roundRep.roundIndex, matchIndex)
                            if (!player) player = emptyPlayer
                            matchBloc.player2 = player
                        }

                        Component.onCompleted: updateMatchData()

                        Connections {
                            target: selectedSerie

                            function onMatchesUpdated() {
                                matchBloc.updateMatchData()
                            }
                        }
                    }
                }
            }
        }
    }
}
