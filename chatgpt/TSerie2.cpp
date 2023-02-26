// Class to represent a series
class TSerie
{
public:
    enum Type { RoundRobin, SingleElimination };

    TSerie(const QString& name, const QList<Player*>& players, Type type, const QString& maxRank, const QString& status)
        : m_name(name), m_players(players), m_type(type), m_maxRank(maxRank), m_status(status), m_matches(QList<TMatch*>()) {}

    QString getName() const { return m_name; }
    QList<Player*> getPlayers() const { return m_players; }
    Type getType() const { return m_type; }
    QString getMaxRank() const { return m_maxRank; }
    QString getStatus() const { return m_status; }
    QList<TMatch*> getMatches() const { return m_matches; }

    // Function to generate the matches for the series
    void generateMatches()
    {
        // Clear any existing matches
        m_matches.clear();

        // Seed the players for the series
        seedPlayers();

        if (m_type == RoundRobin) {
            // Generate the round-robin matches
            int numPlayers = m_players.size();
            QVector<QList<Player*>> rounds(numPlayers - 1);
            for (int i = 0; i < numPlayers - 1; ++i) {
                int mid = numPlayers / 2;
                for (int j = 0; j < mid; ++j) {
                    int k = numPlayers - 1 - j;
                    if (j == 0) {
                        m_matches.append(new TMatch(m_players[j], m_players[k]));
                    } else {
                        rounds[i].append(m_players[j]);
                        rounds[i].append(m_players[k]);
                    }
                }
                std::rotate(m_players.begin() + 1, m_players.begin() + numPlayers - 1, m_players.end());
            }
            for (int i = 0; i < rounds.size(); ++i) {
                for (int j = 0; j < rounds[i].size(); j += 2) {
                    m_matches.append(new TMatch(rounds[i][j], rounds[i][j + 1]));
                }
            }
        } else {
            // Generate the single-elimination matches
            int numPlayers = m_players.size();
            int numRoundsLog2 = qLn(numPlayers) / qLn(2);
            int numRoundsPow2 = qPow(2, numRoundsLog2);
            int numByes = numRoundsPow2 - numPlayers;

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
						// Update the existing match with the winner
						match->setPlayer2(winner);
					}
				}
			}

			// Add the players with byes to the matches list
			for (int i = 0; i < numByes; ++i) {
				m_matches.append(new TMatch(nullptr, nullptr));
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
	
