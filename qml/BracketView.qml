import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: rowRounds

    property QtObject serie
    property bool isEditable: false
    property real scaleFactor: 1.0

    //only display rounds starting/ending at those index
    property int startRoundIdx: 0
    property int endRoundIdx: serie? serie.rounds: 0

    function sc(v) {
        return scaleFactor * v
    }

    //return the ColumnLayout item for the round param
    function itemFromRound(round) {
        return roundRepeater.itemAt(round)
    }

    //Return x,y position of the match bloc in the BracketView
    function findMatchBlocPosition(round, match) {
        let item = itemFromRound(round)
        let matchItem = item.children
        let matchBloc = matchItem[match]
        let pos = matchBloc.mapToItem(bracketView, 0, 0)
        return Qt.rect(pos.x, pos.y, matchBloc.width, matchBloc.height)
    }

    spacing: sc(30)

    Repeater {
        id: roundRepeater
        model: serie? endRoundIdx - startRoundIdx: 0

        ColumnLayout {
            id: colMatchs

            spacing: sc(20)

            Repeater {
                id: matchRepeater
                property int roundIndex: index + rowRounds.startRoundIdx
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
                        isHandicap: serie? serie.isHandicap: false

                        onClicked: if (serie && isEditable) serie.clickedOnMatch(matchRepeater.roundIndex, matchIndex)

                        function updateMatchData() {
                            matchBloc.score1 = serie.scoreForMatch(matchRepeater.roundIndex, matchIndex, 0)
                            matchBloc.score2 = serie.scoreForMatch(matchRepeater.roundIndex, matchIndex, 1)
                            matchBloc.winner1 = serie.winnerForMatch(matchRepeater.roundIndex, matchIndex, 0)
                            matchBloc.winner2 = serie.winnerForMatch(matchRepeater.roundIndex, matchIndex, 1)
                            matchBloc.roundIdx = matchRepeater.roundIndex
                            matchBloc.matchIdx = matchIndex

                            let player1 = serie.getPlayer1(matchRepeater.roundIndex, matchIndex)
                            let player2 = serie.getPlayer2(matchRepeater.roundIndex, matchIndex)

                            let handicapP1 = 0
                            let handicapP2 = 0
                            if (serie.isHandicap && player1 && player2) {
                                let rankP1 = player1.ranking === "NC"? 95 : player1.ranking
                                let rankP2 = player2.ranking === "NC"? 95 : player2.ranking

                                let ecart = Math.floor(Math.abs(rankP1 - rankP2) / 5)
                                if (rankP1 > rankP2) {
                                    handicapP1 = serie.computeHandicap(ecart)
                                } else if (rankP2 > rankP1) {
                                    handicapP2 = serie.computeHandicap(ecart)
                                }
                            }

                            if (!player1) player1 = emptyPlayer
                            matchBloc.player1 = player1
                            if (!player2) player2 = emptyPlayer
                            matchBloc.player2 = player2

                            matchBloc.handicap1 = handicapP1
                            matchBloc.handicap2 = handicapP2
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
