import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: rowRounds

    property QtObject serie
    property bool isEditable: false
    property real scaleFactor: 1.0

    function sc(v) {
        return scaleFactor * v
    }

    spacing: sc(30)

    Repeater {
        model: serie? serie.rounds: 0

        ColumnLayout {
            id: colMatchs

            spacing: sc(20)

            Repeater {
                id: roundRep
                property int roundIndex: index
                model: serie? serie.matchCountForRound(roundIndex): 0

                Item {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.fillHeight: true
                    Layout.preferredHeight: children[0].height
                    Layout.preferredWidth: children[0].width

                    MatchBloc {
                        id: matchBloc
                        anchors.centerIn: parent
                        isEditable: rowRounds.isEditable
                        scaleFactor: rowRounds.scaleFactor

                        QtObject {
                            id: emptyPlayer
                            property string firstName
                            property string lastName
                            property string ranking
                            property string club
                            property string firstNameSecond
                            property string lastNameSecond
                            property string rankingSecond
                            property string clubSecond
                        }

                        property int matchIndex: index
                        property QtObject player1: emptyPlayer
                        property QtObject player2: emptyPlayer

                        playerFirstName1: player1.firstName
                        playerLastName1: player1.lastName
                        playerRank1: player1.ranking
                        playerClub1: player1.club
                        playerFirstNameSecond1: player1.firstNameSecond
                        playerLastNameSecond1: player1.lastNameSecond
                        playerRankSecond1: player1.rankingSecond

                        playerFirstName2: player2.firstName
                        playerLastName2: player2.lastName
                        playerRank2: player2.ranking
                        playerClub2: player2.club
                        playerFirstNameSecond2: player2.firstNameSecond
                        playerLastNameSecond2: player2.lastNameSecond
                        playerRankSecond2: player2.rankingSecond

                        isDouble: serie? serie.isDouble: false

                        onClicked: if (serie && isEditable) serie.clickedOnMatch(roundRep.roundIndex, matchIndex)

                        function updateMatchData() {
                            matchBloc.score1 = serie.scoreForMatch(roundRep.roundIndex, matchIndex, 0)
                            matchBloc.score2 = serie.scoreForMatch(roundRep.roundIndex, matchIndex, 1)
                            matchBloc.winner1 = serie.winnerForMatch(roundRep.roundIndex, matchIndex, 0)
                            matchBloc.winner2 = serie.winnerForMatch(roundRep.roundIndex, matchIndex, 1)

                            let player = serie.getPlayer1(roundRep.roundIndex, matchIndex)
                            if (!player) player = emptyPlayer
                            matchBloc.player1 = player

                            player = serie.getPlayer2(roundRep.roundIndex, matchIndex)
                            if (!player) player = emptyPlayer
                            matchBloc.player2 = player
                        }

                        Component.onCompleted: updateMatchData()

                        Connections {
                            target: serie

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
