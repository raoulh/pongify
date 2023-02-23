#include "TSerie.h"
#include <QJsonArray>
#include <QCollator>
#include <algorithm>
#include <random>
#include <QQmlEngine>

TMatch::TMatch(QObject *parent):
    QObject(parent)
{
    update_player1(nullptr);
    update_player2(nullptr);
    update_playerScore1(-1);
    update_playerScore2(-1);
    update_playerWinner1(false);
    update_playerWinner2(false);
    update_isBye(false);
}

TSerie::TSerie(QObject *parent):
    QObject{parent}
{
    players = PlayerModel::createEmpty();
    update_players(players);

    connect(players, &PlayerModel::playersChanged, this, &TSerie::playersModelChanged);
    playersModelChanged();
}

TSerie::~TSerie()
{
    delete players;
}

TSerie *TSerie::fromJson(const QJsonObject &obj)
{
    if (!obj.contains("name") ||
        !obj.contains("players"))
    {
        return nullptr;
    }

    auto t = new TSerie();

    t->update_name(obj["name"].toString());
    t->update_tournamentType(obj["type"].toString());
    t->update_ranking(obj["ranking"].toString());
    t->update_status(obj["status"].toString());
    if (t->get_status().isEmpty())
        t->update_status("stopped");

    QJsonArray arr = obj["players"].toArray();
    for (int i = 0;i < arr.count();i++)
    {
        auto o = arr.at(i).toObject();
        t->players->loadPlayer(o);
    }

    return t;
}

QJsonObject TSerie::toJson()
{
    QJsonObject obj;
    obj.insert("name", get_name());
    obj.insert("type", get_tournamentType());
    obj.insert("ranking", get_ranking());
    obj.insert("status", get_status());

    QJsonArray arr;
    for (int i = 0;i < players->rowCount();i++)
        arr.append(players->item(i)->toJson());

    obj.insert("players", arr);

    return obj;
}

QStringList TSerie::getPlayerLicences()
{
    QStringList lst;
    for (int i = 0;i < players->rowCount();i++)
        lst.append(players->item(i)->get_license());
    return lst;
}

void TSerie::startSerie()
{
    if (get_status() != "stopped") return;

    update_status("playing");
}

/**
 * @param rank
 *            rank of the player, best player == 0, second best player == 1, ...
 * @param partSize
 *            number of total free positions. Has to be a power of 2 (e.g.
 *            2,4,8,16,32)
 * @return returns the start position of the player, zero based
 */
int TSerie::seedPlayer(int rank, int partSize)
{
    // base case, if rank == 1, return position 0
    if (rank <= 0)
        return 0;

    // if our rank is even we need to put the player into the right part
    // so we add half the part size to his position
    // and make a recursive call with half the rank and half the part size
    if (rank % 2 == 0) {
        return partSize / 2 + seedPlayer(rank / 2, partSize / 2);
    }

    // if the rank is uneven, we put the player in the left part
    // since rank is uneven we need to add + 1 so that it stays uneven
    return seedPlayer(rank / 2, partSize / 2);
}

void TSerie::autoSeedPlayers()
{
    //first get players and sort them
    QVector<Player *> allp;
    for (int i = 0;i < players->rowCount();i++)
        allp.append(players->item(i));

    QCollator sorter;

    std::sort(allp.begin(), allp.end(),
              [&sorter](Player *a, Player *b)
    {
        return sorter.compare(a->get_ranking(), b->get_ranking()) < 0;
    });

    if (allp.count() < 3)
        return;

    //clear all matches
    clearAllMatches();
    prepareMatches();

    auto firstRound = allMatches.at(0);

    //Set all Bye match for best players
    int byeCount = matchCountForRound(0) * 2 - allp.count();

    //seed max 25% of the best players manually, the rest will be randomly placed
    int seedMax = ceil(allp.count() * 0.25);
    for (int i = 0;i < seedMax;i++)
    {
        auto matchIdx = seedPlayer(i + 1, firstRound->count() * 2);
        firstRound->at(matchIdx / 2)->update_player1(allp.at(i));

        if (byeCount > 0)
        {
            firstRound->at(matchIdx / 2)->update_player2(nullptr);
            firstRound->at(matchIdx / 2)->update_isBye(true);
            byeCount--;
        }
    }

    //remove seeded players from list and randomize it
    allp.remove(0, seedMax);
    std::random_shuffle(allp.begin(), allp.end());

    for (int iround = 0, jplayer = 0;iround < firstRound->count() && jplayer < allp.count();iround++)
    {
        Player *pl = allp.at(jplayer);

        if (!firstRound->at(iround)->get_player1())
        {
            firstRound->at(iround)->update_player1(pl);
            jplayer++;
            pl = allp.at(jplayer);

            //if there is still some bye to place, put them
            if (byeCount > 0)
            {
                firstRound->at(iround)->update_player2(nullptr);
                firstRound->at(iround)->update_isBye(true);
                byeCount--;
            }
        }

        if (firstRound->at(iround)->get_player1() &&
            !firstRound->at(iround)->get_isBye() &&
            !firstRound->at(iround)->get_player2())
        {
            firstRound->at(iround)->update_player2(pl);
            jplayer++;
        }
    }
}

int TSerie::nearestPowerOf2(long long N)
{
    long long a = log2(N);

    if (pow(2, a) == N)
        return N;

    return pow(2, a + 1);
}

TMatch *TSerie::getMatchForRound(int round, int match)
{
    if (round < 0 || round >= allMatches.count())
        return nullptr;

    if (match < 0 || match >= allMatches.at(round)->count())
        return nullptr;

    return allMatches.at(round)->at(match);
}

void TSerie::prepareMatches()
{
    //Prepare models
    if (allMatches.count() > players->rowCount())
        clearAllMatches();

    //prepare correct number of rounds
    while (allMatches.count() < get_rounds())
    {
        auto r = new TRound();
        allMatches.append(r);
    }

    //prepare correct number of matches in each round
    for (int i = 0;i < allMatches.count();i++)
    {
        auto r = allMatches.at(i);
        while (r->count() < matchCountForRound(i))
        {
            auto m = new TMatch();
            r->append(m);
        }
    }
}

void TSerie::clearAllMatches()
{
    for (int i = 0;i < allMatches.count();i++)
    {
        qDeleteAll(allMatches.at(i)->begin(), allMatches.at(i)->end());
    }
    qDeleteAll(allMatches);
    allMatches.clear();
}

int TSerie::matchCountForRound(int round)
{
    if (get_tournamentType() == "single")
    {
        int playerCountWithBye = nearestPowerOf2(players->rowCount()) / 2;
        int m = playerCountWithBye;
        auto roundCalc = round + 1;
        while (m > 1 && --roundCalc > 0)
            m = m / 2;

        qDebug() << "matchs for round(" << round << "): " << m;

        return m;
    }
    else if (get_tournamentType() == "roundrobbin")
    {

    }

    return 0;
}

QObject *TSerie::getPlayer1(int round, int match)
{
    if (get_tournamentType() == "single")
    {
        auto m = getMatchForRound(round, match);
        if (!m) return nullptr;
        QQmlEngine::setObjectOwnership(m, QQmlEngine::CppOwnership);

        return m->get_player1();
    }

    return nullptr;
}

QObject *TSerie::getPlayer2(int round, int match)
{
    if (get_tournamentType() == "single")
    {
        auto m = getMatchForRound(round, match);
        if (!m) return nullptr;
        QQmlEngine::setObjectOwnership(m, QQmlEngine::CppOwnership);

        return m->get_player2();
    }

    return nullptr;
}

int TSerie::scoreForMatch(int round, int match, int playerIdx)
{
    if (get_tournamentType() == "single")
    {
        auto m = getMatchForRound(round, match);
        if (!m) return -1;

        if (playerIdx == 0)
            return m->get_playerScore1();
        else
            return m->get_playerScore2();
    }

    return -1;
}

bool TSerie::winnerForMatch(int round, int match, int playerIdx)
{
    if (get_tournamentType() == "single")
    {
        auto m = getMatchForRound(round, match);
        if (!m) return -1;

        if (playerIdx == 0)
            return m->get_playerWinner1();
        else
            return m->get_playerWinner2();
    }

    return false;
}

void TSerie::playersModelChanged()
{
    //Min players check
    if (players->rowCount() < 3)
    {
        update_rounds(0);
        return;
    }

    int firstround = nearestPowerOf2(players->rowCount()) / 2;
    auto rounds = 0;

    while (firstround > 0 && ++rounds)
        firstround = firstround / 2;

    update_rounds(rounds);

    qDebug() << "Serie: " << get_name() << " players:" << players->rowCount() << " rounds: " << rounds;

    prepareMatches();
}
