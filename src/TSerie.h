#ifndef TSERIE_H
#define TSERIE_H

#include <QObject>
#include <QQmlObjectListModel.h>
#include "qqmlhelpers.h"
#include "PlayerModel.h"

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
public:
    explicit TMatch(QObject *parent = nullptr);
};

typedef QVector<TMatch *> TRound;

class TSerie : public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(QString, name)
    QML_READONLY_PROPERTY(QString, ranking)
    QML_READONLY_PROPERTY(QString, tournamentType)
    QML_READONLY_PROPERTY(QObject *, players)

    //Status of this serie: stopped, playing, finished
    QML_READONLY_PROPERTY(QString, status)

    //keep the number of round in this serie
    QML_READONLY_PROPERTY(int, rounds)

public:
    explicit TSerie(QObject *parent = nullptr);
    virtual ~TSerie();

    static TSerie *fromJson(const QJsonObject &obj);
    QJsonObject toJson();

    QStringList getPlayerLicences();
    PlayerModel *getPlayerModel() { return players; }

    void startSerie();
    void autoSeedPlayers();
    void removeAllPlayers();

    Q_INVOKABLE int matchCountForRound(int round);
    Q_INVOKABLE QObject *getPlayer1(int round, int match);
    Q_INVOKABLE QObject *getPlayer2(int round, int match);
    Q_INVOKABLE int scoreForMatch(int round, int match, int playerIdx);
    Q_INVOKABLE bool winnerForMatch(int round, int match, int playerIdx);

    Q_INVOKABLE void clickedOnMatch(int round, int match);

signals:
    void matchesUpdated();

private slots:
    void playersModelChanged();

private:
    PlayerModel *players;

    QVector<TRound *> allMatches;

    int nearestPowerOf2(long long N);
    TMatch *getMatchForRound(int round, int match);

    int seedPlayer(int rank, int partSize);
    void prepareMatches();
    void clearAllMatches();
};

#endif // TSERIE_H
