#include <QString>
#include <QVector>
#include "TMatch.h"

enum class SerieType {
    SingleElimination,
    RoundRobin
};

class TSerie {
public:
    TSerie(const QString& name, const QVector<QString>& playerNames, SerieType type, int maxRank);

    QString getName() const;
    QVector<QString> getPlayerNames() const;
    SerieType getType() const;
    int getMaxRank() const;
    bool isPlaying() const;
    bool isStopped() const;
    void start();
    void stop();
    void addMatch(const TMatch& match);

private:
    QString mName;
    QVector<QString> mPlayerNames;
    SerieType mType;
    int mMaxRank;
    bool mIsPlaying;
    bool mIsStopped;
    QVector<TMatch> mMatches;
};

TSerie::TSerie(const QString& name, const QVector<QString>& playerNames, SerieType type, int maxRank)
    : mName(name), mPlayerNames(playerNames), mType(type), mMaxRank(maxRank),
      mIsPlaying(false), mIsStopped(false)
{}

QString TSerie::getName() const {
    return mName;
}

QVector<QString> TSerie::getPlayerNames() const {
    return mPlayerNames;
}

SerieType TSerie::getType() const {
    return mType;
}

int TSerie::getMaxRank() const {
    return mMaxRank;
}

bool TSerie::isPlaying() const {
    return mIsPlaying;
}

bool TSerie::isStopped() const {
    return mIsStopped;
}

void TSerie::start() {
    mIsPlaying = true;
}

void TSerie::stop() {
    mIsStopped = true;
}

void TSerie::addMatch(const TMatch& match) {
    mMatches.append(match);
}

void TSerie::generateRoundRobinMatches() {
    int n = mPlayerNames.size();
    QVector<QVector<QString>> rounds(n - 1, QVector<QString>(n / 2));
    QVector<QString> players = mPlayerNames;

    // Generate rounds
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n / 2; j++) {
            rounds[i][j] = players[j] + " vs " + players[n - j - 1];
        }

        QString lastPlayer = players[players.size() - 1];
        players.removeLast();
        players.insert(1, lastPlayer);
    }

    // Generate matches from rounds
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n / 2; j++) {
            QString player1 = rounds[i][j].split(" vs ")[0];
            QString player2 = rounds[i][j].split(" vs ")[1];
            TMatch match(player1, player2, mName);
            addMatch(match);
        }
    }
}

/*

Explanation:

    The generateRoundRobinMatches() member function generates the graph of matches for a round robin series and adds them to the mMatches vector.
    The first step is to generate the rounds using the player names in mPlayerNames. This is done by creating a 2D vector rounds with n - 1 rows and n / 2 columns, where n is the number of players. Each element of rounds represents a match between two players.
    The algorithm for generating the rounds is as follows:
        For each round i from 0 to n - 2:
            For each match j from 0 to n / 2 - 1:
                Set rounds[i][j] to be a string concatenation of the names of the jth player from the left and the jth player from the right, separated by " vs ".
            Move the last player in the list to the second position, and shift all other players one position to the right.
    Once the rounds are generated, we can iterate over them and create TMatch objects for each match, and add them to the mMatches vector. The algorithm for this is as follows:
        For each round i from 0 to n - 2:
            For each match j from 0 to n / 2 - 1:
                Set player1 to be the name of the jth player from the left in rounds[i][j], and player2 to be the name of the jth player from the right.
                Create a new TMatch object with player1, player2, and the name of the series, and add it to the mMatches vector using the addMatch() member function.
				
*/


void TSerie::generateSingleEliminationMatches() {
    int n = mPlayerNames.size();
    int numMatches = n / 2;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < numMatches; j++) {
            QString player1, player2;
            if (i == 0) {
                player1 = mPlayerNames[j * 2];
                player2 = mPlayerNames[j * 2 + 1];
            } else {
                int matchIndex = (i - 1) * numMatches + j;
                const TMatch& prevMatch = mMatches[matchIndex];
                player1 = prevMatch.getWinner();
                player2 = prevMatch.getLoser();
            }
            TMatch match(player1, player2, mName);
            addMatch(match);
        }
        numMatches /= 2;
    }
}

/*

Explanation:

    The generateSingleEliminationMatches() member function generates the graph of matches for a single elimination series and adds them to the mMatches vector.
    The first step is to compute the number of matches in the first round, which is equal to n / 2, where n is the number of players in the series.
    The algorithm for generating the matches is as follows:
        For each round i from 0 to log2(n) - 1:
            For each match j from 0 to numMatches - 1, where numMatches is the number of matches in the current round:
                If i == 0, set player1 to be the name of the jth player from the left in mPlayerNames, and player2 to be the name of the jth player from the right.
                Otherwise, compute the index of the previous round's match that the winner and loser of this match came from. This is done using the formula (i - 1) * numMatches + j.
                Get the winner and loser of the previous round's match from the mMatches vector using the getWinner() and getLoser() member functions of TMatch.
                Create a new TMatch object with player1, player2, and the name of the series, and add it to the mMatches vector using the addMatch() member function.
            Divide numMatches by 2 to compute the number of matches in the next round.
    The number of rounds required to complete the tournament is equal to log2(n).

*/

void TSerie::seedPlayers() {
    int numPlayers = mPlayers.size();
    int numRanks = mMaxRank == 0 ? numPlayers : (90 - mMaxRank) / 5 + 1;
    QVector<QList<Player*>> rankLists(numRanks);
    for (Player* player : mPlayers) {
        int rankIndex = player->getRank() == "NC" ? numRanks - 1 : (90 - player->getRank().toInt()) / 5;
        rankLists[rankIndex].append(player);
    }
    for (QList<Player*>& rankList : rankLists) {
        std::random_shuffle(rankList.begin(), rankList.end());
    }
    QVector<Player*> seededPlayers(numPlayers);
    int index = 0;
    for (int i = numRanks - 1; i >= 0; i--) {
        for (Player* player : rankLists[i]) {
            if (i == 0 || (index + 1) % 4 != 0) {
                seededPlayers[index++] = player;
            }
        }
    }
    mPlayers = seededPlayers;
}

/*

Explanation:

    The seedPlayers() member function seeds the players for the series by sorting them into random groups based on their ranks.
    The first step is to compute the number of ranks in the series. If mMaxRank is zero, all players are considered to be in the same rank. Otherwise, the number of ranks is computed by subtracting mMaxRank from 90 and dividing by 5, plus 1 to include the unranked players.
    The players are then sorted into rank-based lists using a QVector of QList objects. The rankIndex variable is computed by subtracting the player's rank from 90 and dividing by 5, or setting it to the last index if the player is unranked. The player is then added to the corresponding QList.
    Each QList is then shuffled randomly using std::random_shuffle.
    The players are then sorted into the final seeding order by iterating through the QList objects in reverse order of rank. For each rank, the first three players are added to the seeded player list in order, while the fourth player is skipped. This ensures that the top players are spread out in the seeding.
    Finally, the mPlayers vector is set to the seeded player list.
	
*/

QVector<TMatch> TSerie::getMatchesByLevel(int level) const {
        QVector<TMatch> levelMatches;
        for (const TMatch& match : matches) {
            if (match.level == level) {
                levelMatches.append(match);
            }
        }
        return levelMatches;
    }


QVector<int> TSerie::getLevels() const {
        QVector<int> levels;
        for (const TMatch& match : matches) {
            if (!levels.contains(match.level)) {
                levels.append(match.level);
            }
        }
        std::sort(levels.begin(), levels.end());
        return levels;
    }