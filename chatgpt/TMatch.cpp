#include <QString>
#include <QPair>

class TMatch {
public:
    TMatch(const QPair<QString, QString>& playerNames);

    QPair<QString, QString> getPlayerNames() const;
    void setPlayerScore(const QString& playerName, int score);
    QPair<int, int> getPlayerScores() const;

private:
    QPair<QString, QString> mPlayerNames;
    QPair<int, int> mPlayerScores;
};

TMatch::TMatch(const QPair<QString, QString>& playerNames)
    : mPlayerNames(playerNames), mPlayerScores(QPair<int, int>(-1, -1))
{}

QPair<QString, QString> TMatch::getPlayerNames() const {
    return mPlayerNames;
}

void TMatch::setPlayerScore(const QString& playerName, int score) {
    if (playerName == mPlayerNames.first) {
        mPlayerScores.first = score;
    } else if (playerName == mPlayerNames.second) {
        mPlayerScores.second = score;
    }
}

QPair<int, int> TMatch::getPlayerScores() const {
    return mPlayerScores;
}
