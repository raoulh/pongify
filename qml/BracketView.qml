import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: bracketView

    property QtObject serie
    property bool isEditable: false
    property real scaleFactor: 1.0

    //only display rounds starting/ending at those index
    property int startRoundIdx: 0
    property int endRoundIdx: serie ? serie.rounds : 0

    width: rowRounds.implicitWidth
    height: rowRounds.implicitHeight

    function sc(v) {
        return scaleFactor * v
    }

    //return the ColumnLayout item for the round param (local repeater index)
    function itemFromRound(round) {
        return roundRepeater.itemAt(round)
    }

    //Return x,y position of the match bloc in the BracketView (round is local repeater index)
    function findMatchBlocPosition(round, match) {
        let item = itemFromRound(round)
        if (!item) return Qt.rect(0, 0, 0, 0)
        let matchItem = item.children
        let matchBloc = matchItem[match]
        if (!matchBloc) return Qt.rect(0, 0, 0, 0)
        let pos = matchBloc.mapToItem(bracketView, 0, 0)
        return Qt.rect(pos.x, pos.y, matchBloc.width, matchBloc.height)
    }

    // Canvas overlay for bracket connector lines (drawn behind match blocks)
    Canvas {
        id: connectorCanvas
        anchors.fill: rowRounds
        z: -1

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            if (!serie) return

            var totalRounds = endRoundIdx - startRoundIdx
            if (totalRounds < 2) return

            // Skip connectors for round-robin (same match count per round = no tree)
            var count0 = serie.matchCountForRound(startRoundIdx)
            var count1 = serie.matchCountForRound(startRoundIdx + 1)
            if (count0 === count1) return

            ctx.strokeStyle = "#5a5a5a"
            ctx.lineWidth = sc(2)
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            var cornerRadius = sc(8)

            for (var localR = 0; localR < totalRounds - 1; localR++) {
                var absRNext = localR + 1 + startRoundIdx
                var matchesNextRound = serie.matchCountForRound(absRNext)

                for (var m = 0; m < matchesNextRound; m++) {
                    // Match m in next round is fed by matches 2m and 2m+1 in current round
                    var feeder1 = findMatchBlocPosition(localR, 2 * m)
                    var feeder2 = findMatchBlocPosition(localR, 2 * m + 1)
                    var target = findMatchBlocPosition(localR + 1, m)

                    if (feeder1.width === 0 || feeder2.width === 0 || target.width === 0)
                        continue

                    // Connector endpoints
                    var x1 = feeder1.x + feeder1.width
                    var y1 = feeder1.y + feeder1.height / 2
                    var x2 = feeder2.x + feeder2.width
                    var y2 = feeder2.y + feeder2.height / 2
                    var xT = target.x
                    var yT = target.y + target.height / 2

                    // Midpoint between source right edge and target left edge
                    var midX = (Math.max(x1, x2) + xT) / 2

                    // Clamp radius so it fits
                    var r = Math.min(cornerRadius,
                                     Math.abs(yT - y1) / 2,
                                     Math.abs(yT - y2) / 2,
                                     (midX - Math.max(x1, x2)) / 2)
                    if (r < 0) r = 0

                    // Top branch: feeder1 → horizontal → rounded corner → vertical to yT
                    ctx.beginPath()
                    ctx.moveTo(x1, y1)
                    ctx.lineTo(midX - r, y1)
                    ctx.arcTo(midX, y1, midX, y1 + r, r)
                    ctx.lineTo(midX, yT)
                    ctx.stroke()

                    // Bottom branch: feeder2 → horizontal → rounded corner → vertical to yT
                    ctx.beginPath()
                    ctx.moveTo(x2, y2)
                    ctx.lineTo(midX - r, y2)
                    ctx.arcTo(midX, y2, midX, y2 - r, r)
                    ctx.lineTo(midX, yT)
                    ctx.stroke()

                    // Horizontal connector: midX → target left edge
                    ctx.beginPath()
                    ctx.moveTo(midX, yT)
                    ctx.lineTo(xT, yT)
                    ctx.stroke()
                }
            }
        }
    }

    RowLayout {
        id: rowRounds
        spacing: sc(30)

        Repeater {
            id: roundRepeater
            model: serie ? endRoundIdx - startRoundIdx : 0

            ColumnLayout {
                id: colMatchs

                spacing: sc(20)

                Repeater {
                    id: matchRepeater
                    property int roundIndex: index + bracketView.startRoundIdx
                    model: serie ? serie.matchCountForRound(roundIndex) : 0

                    Item {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.fillHeight: true
                        Layout.preferredHeight: children[0].height
                        Layout.preferredWidth: children[0].width

                        MatchBloc {
                            id: matchBloc
                            anchors.centerIn: parent
                            isEditable: bracketView.isEditable
                            scaleFactor: bracketView.scaleFactor

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

                            isDouble: serie ? serie.isDouble : false
                            isHandicap: serie ? serie.isHandicap : false

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

    // Delayed paint to ensure layout is complete
    Timer {
        id: paintTimer
        interval: 100
        running: true
        repeat: false
        onTriggered: connectorCanvas.requestPaint()
    }

    // Repaint connectors when match data changes
    Connections {
        target: serie
        enabled: serie !== null
        function onMatchesUpdated() {
            paintTimer.restart()
        }
    }

    // Repaint on layout/scale changes
    onScaleFactorChanged: paintTimer.restart()
    onStartRoundIdxChanged: paintTimer.restart()
    onEndRoundIdxChanged: paintTimer.restart()

    Connections {
        target: rowRounds
        function onImplicitWidthChanged() { paintTimer.restart() }
        function onImplicitHeightChanged() { paintTimer.restart() }
    }
}
