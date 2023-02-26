#include <QtCore>

// Forward declaration of classes
class TMatch;
class TSerie;
class Player;
class Tournament;

// Class to represent a player
class Player
{
public:
    Player(const QString& name, const QString& rank)
        : m_name(name), m_rank(rank) {}

    QString getName() const { return m_name; }
    QString getRank() const { return m_rank; }

private:
    QString m_name;
    QString m_rank;
};

// Class to represent a match
class TMatch
{
public:
    TMatch(Player* p1, Player* p2)
        : m_p1(p1), m_p2(p2), m_winner(nullptr) {}

    Player* getPlayer1() const { return m_p1; }
    Player* getPlayer2() const { return m_p2; }
    Player* getWinner() const { return m_winner; }

    void setWinner(Player* winner) { m_winner = winner; }

private:
    Player* m_p1;
    Player* m_p2;
    Player* m_winner;
};

// Class to represent a tournament series
class TSerie
{
public:
    enum Type {
        RoundRobin,
        SingleElimination
    };

    TSerie(const QString& name, const QList<Player*>& players, Type type,
           const QString& maxRank, const QString& status = "stopped")
        : m_name(name), m_players(players), m_type(type), m_maxRank(maxRank),
          m_status(status), m_matches(QList<TMatch*>()) {}

    QString getName() const { return m_name; }
    QList<Player*> getPlayers() const { return m_players; }
    Type getType() const { return m_type; }
    QString getMaxRank() const { return m_maxRank; }
    QString getStatus() const { return m_status; }
    QList<TMatch*> getMatches() const { return m_matches; }

    void setStatus(const QString& status) { m_status = status; }

    // Function to seed the players for the series
    void seedPlayers()
    {
        // Seed the players randomly
        QList<Player*> shuffledPlayers = m_players;
        std::random_shuffle(shuffledPlayers.begin(), shuffledPlayers.end());

        // Sort the players by rank
        std::sort(shuffledPlayers.begin(), shuffledPlayers.end(),
                  [](Player* p1, Player* p2) { return p1->getRank() > p2->getRank(); });

        // Move the best players to the end of the list
        int numBestPlayers = qMin(4, shuffledPlayers.size() / 4);
        QList<Player*> bestPlayers = shuffledPlayers.mid(shuffledPlayers.size() - numBestPlayers);
        shuffledPlayers.erase(shuffledPlayers.end() - numBestPlayers, shuffledPlayers.end());
        shuffledPlayers.append(bestPlayers);

        // Set the seeded players for the series
        m_players = shuffledPlayers;
    }

    // Function to generate the matches for the series
    void generateMatches()
    {
        // Clear any existing matches
        m_matches.clear();

        // Generate the matches based on the series type
        if (m_type == RoundRobin) {
            // Generate a round-robin schedule
            for (int i = 0; i < m_players.size(); ++i) {
                for (int j = i + 1; j < m_players.size(); ++j) {
                    m_matches.append(new TMatch(m_players[i], m_players[j]));
                }
            }
        } else if (m_type == SingleElimination) {
            // Generate a single elimination schedule
            int numRounds = xxx
			int numPlayers = m_players.size();
			int numRoundsLog2 = qLn(numPlayers) / qLn(2);
			int numRoundsPow2 = qPow(2, numRoundsLog2);
			int numByes = numRoundsPow2 - numPlayers;

			// Seed the players for the series
			seedPlayers();

			// Generate the first round matches
			for (int i = 0; i < numPlayers - numByes; i += 2) {
				m_matches.append(new TMatch(m_players[i], m_players[i + 1]));
			}

			// Generate the remaining rounds
			for (int round = 2; round <= numRoundsPow2; round *= 2) {
				int numMatches = m_matches.size();

				for (int i = 0; i < numMatches; ++i) {
					// Get the match for this round
					TMatch* match = m_matches[i];

					// Get the winner of the previous round match
					Player* winner = match->getWinner();

					// Generate the match for this round
					if (i % 2 == 0) {
						// Create a new match for the winner
						m_matches.append(new TMatch(winner, nullptr));
					} else {
						// Get the previous match for this round
						TMatch* prevMatch = m_matches[i - 1];

						// Set the winner of the previous match as the second player
						prevMatch->setPlayer2(winner);
					}

					// Set the winner of the previous round match as the first player
					match->setPlayer1(winner);
				}
			}
		}
	}
	
private:
	QString m_name;
	QList<Player*> m_players;
	Type m_type;
	QString m_maxRank;
	QString m_status;
	QList<TMatch*> m_matches;
};

// Class to represent a tournament
class Tournament
{
public:
	Tournament(const QString& name)
		: m_name(name), m_series(QList<TSerie*>()) {}
		
	QString getName() const { return m_name; }
	QList<TSerie*> getSeries() const { return m_series; }

	// Function to add a series to the tournament
	void addSeries(TSerie* series)
	{
		m_series.append(series);
	}

	// Function to remove a series from the tournament
	void removeSeries(TSerie* series)
	{
		m_series.removeOne(series);
	}
	
private:
	QString m_name;
	QList<TSerie*> m_series;
};