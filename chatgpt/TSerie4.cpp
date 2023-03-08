typedef QVector<TMatch*> TRound;

class TSerie {
public:
    TSerie(QString name, QList<Player*> players, QString type, int maxRank) : name_(name), players_(players), type_(type), maxRank_(maxRank), status_("stopped") {}
    void generateRoundRobinMatches();
    // other member functions omitted for brevity
private:
    QString name_;
    QList<Player*> players_;
    QString type_;
    int maxRank_;
    QString status_;
    QVector<TRound*> allMatches_;
};

void TSerie::generateRoundRobinMatches() {
    // Calculate number of rounds
    int numRounds = players_.size() - 1;
    if (numRounds % 2 == 1) {
        numRounds++;
    }

    // Create rounds of matches
    for (int round = 0; round < numRounds; round++) {
        TRound* matches = new TRound();
        allMatches_.append(matches);

        // Create matches for this round
        for (int i = 0; i < players_.size() / 2; i++) {
            int player1Index = (round + i) % (players_.size() - 1);
            int player2Index = (round + players_.size() - i - 1) % (players_.size() - 1);
            if (player2Index == players_.size() - 1) {
                player2Index = players_.size() - 2;
            }
            if (players_[player1Index]->rank() <= maxRank_ && players_[player2Index]->rank() <= maxRank_) {
                TMatch* match = new TMatch(players_[player1Index], players_[player2Index]);
                matches->append(match);
            }
        }

        // Move last player to second position, except in first round
        if (round > 0) {
            players_.move(players_.size() - 1, 1);
        }
    }
}
