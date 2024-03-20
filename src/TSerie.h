#ifndef TSERIE_H
#define TSERIE_H

#include <QObject>
#include <QQmlObjectListModel.h>
#include "qqmlhelpers.h"
#include "PlayerModel.h"
#include "Score.h"

class TMatch: public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(Player *, player1)
    QML_READONLY_PROPERTY(Player *, player2)
    QML_READONLY_PROPERTY(int, playerScore1)
    QML_READONLY_PROPERTY(int, playerScore2)
    QML_READONLY_PROPERTY(bool, playerWinner1)
    QML_READONLY_PROPERTY(bool, playerWinner2)
    QML_READONLY_PROPERTY(bool, isBye)
    QML_READONLY_PROPERTY(bool, isDouble)
    QML_READONLY_PROPERTY(bool, isHandicap)
public:
    explicit TMatch(QObject *parent = nullptr);

    void clearScore();
};

typedef QVector<TMatch *> TRound;

class TSerie : public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(QString, name)
    QML_READONLY_PROPERTY(QString, ranking)
    QML_READONLY_PROPERTY(QString, tournamentType)
    QML_READONLY_PROPERTY(QObject *, players)
    QML_READONLY_PROPERTY(QObject *, winners)

    QML_READONLY_PROPERTY(bool, podiumValidated)

    //Status of this serie: stopped, playing, finished
    QML_READONLY_PROPERTY(QString, status)

    //keep the number of round in this serie
    QML_READONLY_PROPERTY(int, rounds)

    QML_READONLY_PROPERTY(bool, isDouble)
    QML_READONLY_PROPERTY(bool, isHandicap)

    //This holds the current round. This is used to display the correct round in BroadcastingViews
    QML_READONLY_PROPERTY(int, currentRound)

    //This is used to save the visibility of the broadcast view
    QML_WRITABLE_PROPERTY(bool, viewVisible)

    QML_WRITABLE_PROPERTY(QString, serieUid)

public:
    explicit TSerie(QObject *parent = nullptr);
    virtual ~TSerie();

    static TSerie *fromJson(const QJsonObject &obj);
    QJsonObject toJson();

    QStringList getPlayerLicences();
    PlayerModel *getPlayerModel() { return players; }

    void clearPlayers();

    void startSerie();
    void stopSerie();
    void autoSeedPlayers();
    void removeAllPlayers();
    void showPodium();

    Q_INVOKABLE int matchCountForRound(int round);
    Q_INVOKABLE QObject *getPlayer1(int round, int match);
    Q_INVOKABLE QObject *getPlayer2(int round, int match);
    Q_INVOKABLE int scoreForMatch(int round, int match, int playerIdx);
    Q_INVOKABLE bool winnerForMatch(int round, int match, int playerIdx);
    Q_INVOKABLE void clickedOnMatch(int round, int match);

    typedef struct UnplayedMatch
    {
        int round;
        int match;
        TMatch *p_match = nullptr;
    } UnplayedMatch;

    QList<UnplayedMatch> unplayedNextMatches();

    TMatch *getMatchForRound(int round, int match);

signals:
    void matchesUpdated();

private slots:
    void playersModelChanged();

private:
    PlayerModel *players;
    QQmlObjectListModel<Player> *winners;

    QVector<TRound *> allMatches;

    QList<ScoreRR> scoresWinners; //for roundrobin
    QList<Player *> singleWinners; //for single elemination

    int nearestPowerOf2(long long N);

    int seedPlayer(int rank, int partSize);
    void prepareMatches();
    void clearAllMatches();

    void updateNextMatches();
    void updateCurrentRound();

    void calculateRRWinners();
    void calculateSingleWinners();

    bool allMatchesPlayed();
};

#endif // TSERIE_H
