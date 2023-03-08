#include <QtGlobal>
#include <QString>
#include <QVector>

// Class representing a player
class Player {
public:
    Player(QString name, QString rank) : name_(name), rank_(rank) {}
    QString name() const { return name_; }
    QString rank() const { return rank_; }
private:
    QString name_;
    QString rank_;
};

// Class representing a match
class TMatch {
public:
    TMatch(Player* player1, Player* player2) : player1_(player1), player2_(player2), winner_(nullptr) {}
    Player* player1() const { return player1_; }
    Player* player2() const { return player2_; }
    Player* winner() const { return winner_; }
    void setWinner(Player* player) { winner_ = player; }
private:
    Player* player1_;
    Player* player2_;
    Player* winner_;
};

// Class representing a match
class TMatch {
public:
    TMatch(Player* player1, Player* player2) : player1_(player1), player2_(player2), winner_(nullptr), status_("not started") {}
    Player* player1() const { return player1_; }
    Player* player2() const { return player2_; }
    Player* winner() const { return winner_; }
    QString status() const { return status_; }
    void setWinner(Player* player) { winner_ = player; }
    void setStatus(QString status) { status_ = status; }
    bool isCompleted() const { return winner_ != nullptr; }
private:
    Player* player1_;
    Player* player2_;
    Player* winner_;
    QString status_;
};

// Enumeration representing the type of a series
enum class TSerieType {
    RoundRobin,
    SingleElimination
};

// Class representing a series
class TSerie {
public:
    TSerie(QString name, QVector<Player*> players, TSerieType type, int maxRank)
        : name_(name), players_(players), type_(type), maxRank_(maxRank), status_("stopped"), matches_() {}
    QString name() const { return name_; }
    QVector<Player*> players() const { return players_; }
    TSerieType type() const { return type_; }
    int maxRank() const { return maxRank_; }
    QString status() const { return status_; }
    QVector<QVector<TMatch*>> matches() const { return matches_; }
    void setStatus(QString status) { status_ = status; }
    void generateRoundRobinMatches();
private:
    QString name_;
    QVector<Player*> players_;
    TSerieType type_;
    int maxRank_;
    QString status_;
    QVector<QVector<TMatch*>> matches_;
};

// Class representing a tournament
class Tournament {
public:
    Tournament() : series_() {}
    void addSeries(TSerie* series) { series_.append(series); }
private:
    QVector<TSerie*> series_;
};

// Member function to generate RoundRobin matches
void TSerie::generateRoundRobinMatches() {
    int n = players_.size();
    matches_.resize(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        matches_[i].resize(n / 2);
        for (int j = 0; j < n / 2; ++j) {
            int player1Index = (i + j) % (n - 1);
            int player2Index = (i + n - j - 1) % (n - 1);
            if (j == 0) {
                player2Index = n - 1;
            }
            Player* player1 = players_[player1Index];
            Player* player2 = players_[player2Index];
            matches_[i][j] = new TMatch(player1, player2);
        }
    }
}
