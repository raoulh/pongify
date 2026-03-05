#include "TSerie.h"
#include <QJsonArray>
#include <QCollator>
#include <algorithm>
#include <random>
#include <QQmlEngine>
#include <QMenu>
#include <QAction>
#include "DialogEditScore.h"
#include <QMessageBox>
#include "DialogChangePlayer.h"
#include "DialogPodiumRR.h"
#include "DialogPodiumSingle.h"

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
    update_isDouble(false);
    update_isHandicap(false);

    connect(this, &TMatch::player1Changed, this, [this](auto *player)
    {
        // Disconnect previous player's destroyed signal
        if (m_player1DestroyedConn)
            disconnect(m_player1DestroyedConn);

        if (player)
        {
            m_player1DestroyedConn = connect(player, &Player::destroyed, this, [this]()
            {
                update_player1(nullptr);
            });
        }
    });

    connect(this, &TMatch::player2Changed, this, [this](auto *player)
    {
        // Disconnect previous player's destroyed signal
        if (m_player2DestroyedConn)
            disconnect(m_player2DestroyedConn);

        if (player)
        {
            m_player2DestroyedConn = connect(player, &Player::destroyed, this, [this]()
            {
                update_player2(nullptr);
            });
        }
    });
}

void TMatch::clearScore()
{
    update_playerScore1(-1);
    update_playerScore2(-1);
    update_playerWinner1(false);
    update_playerWinner2(false);
}

TSerie::TSerie(QObject *parent):
    QObject{parent}
{
    players = PlayerModel::createEmpty();
    QQmlEngine::setObjectOwnership(players, QQmlEngine::CppOwnership);
    update_players(players);
    update_currentRound(0);
    update_podiumValidated(false);
    set_viewVisible(true);
    set_serieUid(QUuid::createUuid().toString());

    winners = new QQmlObjectListModel<Player>(this, "name");
    update_winners(winners);

    connect(players, &PlayerModel::playersChanged, this, &TSerie::playersModelChanged);
    playersModelChanged();

    update_isDouble(false);
    update_isHandicap(false);
    connect(this, &TSerie::tournamentTypeChanged, this, &TSerie::playersModelChanged);
}

TSerie::~TSerie()
{
    delete players;
    clearAllMatches();
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
    t->update_isDouble(obj["double"].toBool());
    t->update_isHandicap(obj["handicap"].toBool());
    t->set_serieUid(obj["uid"].toString());
    t->set_viewVisible(obj.contains("view_visible") ? obj["view_visible"].toBool() : true);
    if (t->get_serieUid().isEmpty())
        t->set_serieUid(QUuid::createUuid().toString());

    QJsonArray arr = obj["players"].toArray();
    for (int i = 0;i < arr.count();i++)
    {
        auto o = arr.at(i).toObject();
        t->players->loadPlayer(o);
    }

    t->clearAllMatches();

    //get all rounds
    arr = obj["rounds"].toArray();
    for (int i = 0;i < arr.count();i++)
    {
        auto round = arr.at(i).toArray();
        auto r = new TRound();
        for (int j = 0;j < round.count();j++)
        {
            auto match = round.at(j).toObject();
            auto m = new TMatch();

            m->update_isDouble(t->get_isDouble());
            m->update_isHandicap(t->get_isHandicap());
            m->update_playerScore1(match["playerScore1"].toInt());
            m->update_playerScore2(match["playerScore2"].toInt());
            m->update_playerWinner1(match["playerWinner1"].toBool());
            m->update_playerWinner2(match["playerWinner2"].toBool());
            m->update_isBye(match["isBye"].toBool());

            auto player = t->players->getFromLicense(match["player1"].toString());
            m->update_player1(player);
            player = t->players->getFromLicense(match["player2"].toString());
            m->update_player2(player);

            r->append(m);
        }
        t->allMatches.append(r);
    }

    t->update_podiumValidated(false);
    arr = obj["podium"].toArray();
    if (!arr.isEmpty())
    {
        t->update_podiumValidated(true);

        if (t->get_tournamentType() == "single")
        {
            t->singleWinners.clear();

            for (int i = 0;i < arr.count();i++)
            {
                auto playerLic = arr.at(i).toString();

                auto player = t->players->getFromLicense(playerLic);
                if (player)
                    t->singleWinners.append(player);
            }
        }
        else if (t->get_tournamentType() == "roundrobin")
        {
            t->scoresWinners.clear();

            for (int i = 0;i < arr.count();i++)
            {
                auto playerScore = arr.at(i).toObject();

                auto player = t->players->getFromLicense(playerScore["player"].toString());
                ScoreRR s{ player,
                            playerScore["score"].toInt(),
                            playerScore["setWin"].toInt(),
                            playerScore["setLoose"].toInt(),
                            playerScore["winCount"].toInt()
                         };

                if (s.player)
                    t->scoresWinners.append(s);
            }
        }
    }

    t->updateNextMatches();
    return t;
}

QJsonObject TSerie::toJson()
{
    QJsonObject obj;
    obj.insert("name", get_name());
    obj.insert("type", get_tournamentType());
    obj.insert("ranking", get_ranking());
    obj.insert("status", get_status());
    obj.insert("double", get_isDouble());
    obj.insert("handicap", get_isHandicap());
    obj.insert("uid", get_serieUid());
    obj.insert("view_visible", get_viewVisible());

    QJsonArray arr;
    for (int i = 0;i < players->rowCount();i++)
        arr.append(players->item(i)->toJson());

    obj.insert("players", arr);

    //save all rounds
    arr = {};
    for (int i = 0;i < allMatches.count();i++)
    {
        QJsonArray roundArr;

        for (int j = 0;j < allMatches.at(i)->count();j++)
        {
            auto match = allMatches.at(i)->at(j);
            QJsonObject mObj = {
                { "playerScore1", match->get_playerScore1() },
                { "playerScore2", match->get_playerScore2() },
                { "playerWinner1", match->get_playerWinner1() },
                { "playerWinner2", match->get_playerWinner2() },
                { "isBye", match->get_isBye() },
                { "player1", match->get_player1()? match->get_player1()->get_license(): "" },
                { "player2", match->get_player2()? match->get_player2()->get_license(): "" },
            };
            roundArr.append(mObj);
        }

        arr.append(roundArr);
    }
    obj.insert("rounds", arr);

    if (get_podiumValidated())
    {
        QJsonArray arr;

        if (get_tournamentType() == "single")
        {
            for (int i = 0;i < singleWinners.count();i++)
                arr.append(singleWinners.at(i)->get_license());
        }
        else if (get_tournamentType() == "roundrobin")
        {
            for (int i = 0;i < scoresWinners.count();i++)
            {
                const auto &s = scoresWinners.at(i);

                QJsonObject mObj = {
                    { "player", s.player? s.player->get_license(): "" },
                    { "score", s.score },
                    { "setWin", s.setWin },
                    { "setLoose", s.setLoose },
                    { "winCount", s.winCount },
                };
                arr.append(mObj);
            }
        }
        obj.insert("podium", arr);
    }

    return obj;
}

QStringList TSerie::getPlayerLicences()
{
    QStringList lst;
    for (int i = 0;i < players->rowCount();i++)
        lst.append(players->item(i)->get_license());
    return lst;
}

void TSerie::clearPlayers()
{
    singleWinners.clear();
    winners->clear();
    players->clear();
}

void TSerie::startSerie()
{
    //Serie status:
    // - stopped  --> by default when created
    // - playing  --> once the serie is started
    // - finished --> when the score is entered for the final match, hall of fame can be displayed for the serie

    if (get_status() != "stopped") return;

    if (allMatches.count() < 2)
        return;

    if (get_tournamentType() == "single")
    {
        //Check if all players are playing the 1st round
        auto firstRound = allMatches.at(0);
        for (int i = 0;i < players->rowCount();i++)
        {
            bool found = false;
            for (int j = 0;j < firstRound->count();j++)
            {
                auto match = firstRound->at(j);
                if (match->get_player1() == players->item(i) ||
                    match->get_player2() == players->item(i))
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                QMessageBox::warning(nullptr, "Attention",
                                     "Tous les joueurs ne sont pas placés dans le premier tour.\n"
                                     "Impossible de démarrer la série.");

                return;
            }
        }

        auto addToSet = [](QSet<Player *> &s, Player *p)
        {
            if (!p)
                return false;

            if (!s.contains(p))
            {
                s.insert(p);
                return false;
            }

            return true;
        };

        QSet<Player *> s;
        for (int i = 0;i < firstRound->count();i++)
        {
            auto match = firstRound->at(i);

            if (addToSet(s, match->get_player1()) ||
                addToSet(s, match->get_player2()))
            {
                QMessageBox::warning(nullptr, "Attention",
                                     "Il y a des joueurs en double dans le premier tour. Veuillez corriger.");

                return;
            }
        }
    }

    update_status("playing");
}

void TSerie::stopSerie()
{
    if (get_status() != "playing") return;

    if (!get_podiumValidated())
    {
        QMessageBox::warning(nullptr, "Attention",
                             "Le podium n'a pas été validé.\n"
                             "Veuillez ouvrir le fenetre du podium pour cette série et valider les résultats.\n\n"
                             "Une fois le podium validé aucun changement ne pourra plus être effectué.");

        return;
    }

    update_status("finished");
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
    if (rank <= 1)
        return 0;

    // if our rank is even we need to put the player into the right part
    // so we add half the part size to his position
    // and make a recursive call with half the rank and half the part size
    if (rank % 2 == 0) {
        return partSize / 2 + seedPlayer(rank / 2, partSize / 2);
    }

    // if the rank is uneven, we put the player in the left part
    // since rank is uneven we need to add + 1 so that it stays uneven
    return seedPlayer(rank / 2 + 1, partSize / 2);
}

void TSerie::autoSeedPlayers()
{
    //first get players and sort them
    QVector<Player *> allp;
    for (int i = 0;i < players->rowCount();i++)
        allp.append(players->item(i));

    QCollator sorter;

    std::sort(allp.begin(), allp.end(),
              [&sorter, this](Player *a, Player *b)
    {
        QString rA, rB;
        if (get_isDouble())
        {
            rA = sorter.compare(a->get_ranking(), a->get_rankingSecond()) < 0? a->get_ranking(): a->get_rankingSecond();
            rB = sorter.compare(b->get_ranking(), b->get_rankingSecond()) < 0? b->get_ranking(): b->get_rankingSecond();
        }
        else
        {
            rA = a->get_ranking();
            rB = b->get_ranking();
        }
        return sorter.compare(rA, rB) < 0;
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
    std::shuffle(allp.begin(), allp.end(), std::mt19937{std::random_device{}()});

    for (int iround = 0, jplayer = 0;iround < firstRound->count() && jplayer < allp.count();iround++)
    {
        Player *pl = allp.at(jplayer);

        if (!firstRound->at(iround)->get_player1())
        {
            firstRound->at(iround)->update_player1(pl);
            jplayer++;
            if (jplayer >= allp.count())
                break;
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

    updateNextMatches();
    emit matchesUpdated();
}

void TSerie::removeAllPlayers()
{
    clearAllMatches();
    prepareMatches();
}

void TSerie::showPodium()
{
    if (allMatches.isEmpty()) return;

    bool allPlayed = allMatchesPlayed();

    if (get_tournamentType() == "roundrobin")
    {
        calculateRRWinners();

        DialogPodiumRR d(allPlayed, scoresWinners, get_isDouble(), get_status() == "finished");
        if (d.exec() == QDialog::Accepted && allPlayed)
        {
            update_podiumValidated(true);
            scoresWinners = d.getWinners();
            calculateRRWinners(); //update QML model

            emit matchesUpdated(); //save to disk
        }
    }
    else if (get_tournamentType() == "single")
    {
        calculateSingleWinners();

        DialogPodiumSingle d(allPlayed, singleWinners, get_isDouble());
        if (d.exec() == QDialog::Accepted && allPlayed)
        {
            update_podiumValidated(true);
            singleWinners = d.getWinners();
            calculateSingleWinners(); //update QML model

            emit matchesUpdated(); //save to disk
        }
    }
}

int TSerie::nearestPowerOf2(long long N)
{
    if (N <= 0)
        return 1;

    // Check if N is already a power of 2
    if ((N & (N - 1)) == 0)
        return static_cast<int>(N);

    // Find the next power of 2
    long long result = 1;
    while (result < N)
        result <<= 1;

    return static_cast<int>(result);
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
    clearAllMatches();

    if (get_tournamentType() == "single")
    {
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
                m->update_isDouble(get_isDouble());
                m->update_isHandicap(get_isHandicap());
                r->append(m);
            }
        }
    }
    else if (get_tournamentType() == "roundrobin")
    {
        QVector<Player *> vplayers(players->rowCount());
        for (int i = 0;i < players->rowCount();i++)
            vplayers.replace(i, players->item(i));

        if (vplayers.size() % 2 == 1)
            vplayers.append(nullptr);

        int half = vplayers.size() / 2;
        QVector<Player *> rotated = vplayers.mid(1, -1);

        for (int i = 0;i < get_rounds();i++)
        {
            TRound *r = new TRound();
            allMatches.append(r);

            for (int m = 0;m < half;m++)
            {
                //get first player in first half
                int idx1 = m;

                //get second player in second half reversed
                int idx2 = vplayers.count() - 1 - m;

                auto match = new TMatch();
                match->update_isDouble(get_isDouble());
                match->update_isHandicap(get_isHandicap());
                match->update_player1(idx1 == 0? vplayers[0] : rotated[idx1 - 1]);
                match->update_player2(rotated[idx2 - 1]);
                if (!match->get_player1() || !match->get_player2())
                    match->update_isBye(true);
                r->append(match);
            }

            std::rotate(rotated.rbegin(), rotated.rbegin() + 1, rotated.rend());
        }
    }

    emit matchesUpdated();
}

void TSerie::clearAllMatches()
{
    for (int i = 0;i < allMatches.count();i++)
    {
        qDeleteAll(allMatches.at(i)->begin(), allMatches.at(i)->end());
    }
    qDeleteAll(allMatches);
    allMatches.clear();

    emit matchesUpdated();
}

void TSerie::updateNextMatches()
{
    if (get_tournamentType() == "single")
    {
        for (int i = 0;i < allMatches.count();i++)
        {
            auto round = allMatches.at(i);

            for (int j = 0, i_next = 0;j < round->count();j++)
            {
                if (i + 1 >= allMatches.count())
                    break; //last round

                auto match = round->at(j);
                auto nextMatch = allMatches.at(i + 1)->at(i_next);

                //get winner
                Player *winner = nullptr;
                if (match->get_playerWinner1() || match->get_isBye())
                    winner = match->get_player1();
                else if (match->get_playerWinner2())
                    winner = match->get_player2();

                bool hasChanged = false;
                Player *p1 = nextMatch->get_player1();
                Player *p2 = nextMatch->get_player2();

                if (j % 2 == 0)
                {
                    nextMatch->update_player1(winner);
                    if (p1 != winner)
                        hasChanged = true;
                }
                else
                {
                    nextMatch->update_player2(winner);
                    if (p2 != winner)
                        hasChanged = true;
                }

                //Clear score if player1/2 changed from before
                if (hasChanged)
                    nextMatch->clearScore();

                if ((j + 1) % 2 == 0)
                    i_next++; //increment next round match on pair idx only
            }
        }
    }

    updateCurrentRound();

    if (get_tournamentType() == "roundrobin")
        calculateRRWinners();
    else if (get_tournamentType() == "single")
        calculateSingleWinners();
}

void TSerie::updateCurrentRound()
{
    //if (get_tournamentType() == "single")
    {
        for (int i = 0;i < allMatches.count();i++)
        {
            auto round = allMatches.at(i);
            auto roundIsFinished = true;

            for (int j = 0;j < round->count();j++)
            {
                auto match = round->at(j);

                if (match->get_isBye()) continue; //bye is a win

                if (!match->get_playerWinner1() &&
                    !match->get_playerWinner2())
                {
                    roundIsFinished = false;
                    break;
                }
            }

            if (!roundIsFinished ||
                i == allMatches.count() - 1) //last round
            {
                qDebug() << "updateCurrentRound(" << get_name() <<  "): --> " << i;
                update_currentRound(i);
                break;
            }
        }
    }
}

//Calculate hall of fame for Round Robin type
void TSerie::calculateRRWinners()
{
    QHash<Player *, ScoreRR> scores;

    //Podium already validated, fill the QML model and leave
    if (get_podiumValidated())
    {
        winners->clear();

        for (int i = 0;i < 8 && i < scoresWinners.count();i++)
            winners->append(scoresWinners.at(i).player, false);

        return;
    }

    if (get_currentRound() < 3)
    {
        scoresWinners.clear();
        winners->clear();
        return;
    }

    /*
     * Regles:
     *
     *  - 2 points pour une victoire en 2 sets gagnants
     *  - 1 point pour une victoire en 3 sets gagnants
     *  - 0 point pour une défaite
     *
     *  Criteres en cas d'égalité:
     *
     *  - Nombre de victoires
     *  - Résultat du match entre les joueurs à égalité
     *  - Différence de sets gagnés et perdus
     *  - Différence de points gagnés et perdus
     *
     */

    for (int i = 0;i <= get_currentRound();i++)
    {
        auto round = allMatches.at(i);

        for (int j = 0;j < round->count();j++)
        {
            auto match = round->at(j);

            //do not count this match if not played or bye
            if (!match->get_player1() ||
                !match->get_player2() ||
                match->get_isBye())
                continue;

            auto p1 = match->get_player1();
            auto p2 = match->get_player2();

            auto score1 = scores.value(p1);
            auto score2 = scores.value(p2);

            qDebug() << "Match (" << i << "," << j << "): " << p1->get_firstName() << " " << p1->get_lastName() << " VS " <<
                p2->get_firstName() << " " << p2->get_lastName();

            if (match->get_playerWinner1())
            {
                if (match->get_playerScore2() == 0)
                    score1.score += 2;
                else
                    score1.score += 1;

                score1.winCount++;
                qDebug() << "Score " << p1->get_firstName() << " " << p1->get_lastName() << ": " << score1.winCount;
            }

            score1.setWin += match->get_playerScore1();
            score1.setLoose += match->get_playerScore2();

            if (match->get_playerWinner2())
            {
                if (match->get_playerScore1() == 0)
                    score2.score += 2;
                else
                    score2.score += 1;

                score2.winCount++;
                qDebug() << "Score " << p2->get_firstName() << " " << p2->get_lastName() << ": " << score2.winCount;
            }

            score2.setWin += match->get_playerScore2();
            score2.setLoose += match->get_playerScore1();

            scores.insert(p1, score1);
            scores.insert(p2, score2);
        }
    }

    QList<Player *> sortwin = scores.keys();

    std::sort(sortwin.begin(), sortwin.end(),
              [&scores, this](Player *a, Player *b)
    {
        auto score1 = scores.value(a);
        auto score2 = scores.value(b);

        if (score1.winCount != score2.winCount)
            return score1.winCount > score2.winCount;

        if (score1.score != score2.score)
            return score1.score > score2.score;

        if (score1.setWin != score2.setWin)
            return score1.setWin > score2.setWin;

        if (score1.setLoose != score2.setLoose)
            return score1.setLoose < score2.setLoose;

        //Compare result from match between players only if Serie is finished (to account all matches)
        if (get_currentRound() == allMatches.count() - 1)
        {
            //find match between both player
            for (int i = 0;i < get_currentRound();i++)
            {
                auto round = allMatches.at(i);
                for (int j = 0;j < round->count();j++)
                {
                    auto match = round->at(j);
                    if (match->get_player1() == a && match->get_player2() == b)
                        return match->get_playerWinner1();
                    if (match->get_player1() == b && match->get_player2() == a)
                        return match->get_playerWinner2();
                }
            }
        }

        return false;
    });

    scoresWinners.clear();
    winners->clear();
    for (int i = 0;i < sortwin.count();i++)
    {
        if (i < 8)
            winners->append(sortwin.at(i), false);

        auto p = sortwin.at(i);
        auto score = scores.value(p);
        score.player = p;
        scoresWinners.append(score);
    }
}

void TSerie::calculateSingleWinners()
{
    //Podium already validated, fill the QML model and leave
    if (get_podiumValidated())
    {
        winners->clear();

        for (int i = 0;i < 8 && i < singleWinners.count();i++)
            winners->append(singleWinners.at(i), false);

        return;
    }

    if ((get_currentRound() < get_rounds() - 1 && !allMatchesPlayed()) ||
        get_rounds() <= 0)
    {
        singleWinners.clear();
        winners->clear();
        return;
    }

    //rank winners
    QList<Player *> ranks(8, nullptr);

    for (int i = get_rounds() - 1;i >= 0 && i > get_rounds() - 4;i--)
    {
        for (int j = 0;j < allMatches.at(i)->count();j++)
        {
            auto match = allMatches.at(i)->at(j);

            if (i == get_rounds() - 1) //last round -> 1st & 2nd rank
            {
                if (match->get_playerWinner1())
                {
                    ranks[0] = match->get_player1();
                    ranks[1] = match->get_player2();
                }
                else
                {
                    ranks[1] = match->get_player1();
                    ranks[0] = match->get_player2();
                }
            }
            else if (i == get_rounds() - 2) //3rd & 4th rank
            {
                if (match->get_player1() == ranks[0])
                    ranks[2] = match->get_player2();
                else if (match->get_player2() == ranks[0])
                    ranks[2] = match->get_player1();

                if (match->get_player1() == ranks[1])
                    ranks[3] = match->get_player2();
                else if (match->get_player2() == ranks[1])
                    ranks[3] = match->get_player1();
            }
            else if (i == get_rounds() - 3) //5th & 6th rank
            {
                if (match->get_player1() == ranks[0])
                    ranks[4] = match->get_player2();
                else if (match->get_player2() == ranks[0])
                    ranks[4] = match->get_player1();

                if (match->get_player1() == ranks[1])
                    ranks[5] = match->get_player2();
                else if (match->get_player2() == ranks[1])
                    ranks[5] = match->get_player1();

                if (match->get_player1() == ranks[2])
                    ranks[6] = match->get_player2();
                else if (match->get_player2() == ranks[2])
                    ranks[6] = match->get_player1();

                if (match->get_player1() == ranks[3])
                    ranks[7] = match->get_player2();
                else if (match->get_player2() == ranks[3])
                    ranks[7] = match->get_player1();
            }
        }
    }

    singleWinners.clear();
    winners->clear();
    for (int i = 0;i < 8;i++)
    {
        if (ranks[i])
        {
            winners->append(ranks[i], false);
            singleWinners.append(ranks[i]);
        }
    }
}

bool TSerie::allMatchesPlayed()
{
    if (allMatches.isEmpty())
        return false;

    bool lastRoundPlayed = true;

    //check if last round has been played entirely
    auto round = allMatches.last();
    for (int i = 0;i < round->count();i++)
    {
        auto match = round->at(i);

        if (match->get_isBye()) continue; //bye is a win

        if (!match->get_playerWinner1() &&
            !match->get_playerWinner2())
        {
            lastRoundPlayed = false;
            break;
        }
    }

    return get_currentRound() == allMatches.count() - 1 &&
            lastRoundPlayed &&
            get_status() == "playing";
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

        //qDebug() << "matchs for round(" << round << "): " << m;

        return m;
    }
    else if (get_tournamentType() == "roundrobin")
    {
        int playerCount = players->rowCount();
        if (playerCount % 2 == 1)
            playerCount++;

        int count = playerCount / 2;
        qDebug() << "matchCountForRound:" << count;

        return count;
    }

    return 0;
}

QObject *TSerie::getPlayer1(int round, int match)
{
    auto m = getMatchForRound(round, match);
    if (!m) return nullptr;

    auto p = m->get_player1();
    QQmlEngine::setObjectOwnership(p, QQmlEngine::CppOwnership);

    return p;
}

QObject *TSerie::getPlayer2(int round, int match)
{
    auto m = getMatchForRound(round, match);
    if (!m) return nullptr;

    auto p = m->get_player2();
    QQmlEngine::setObjectOwnership(p, QQmlEngine::CppOwnership);

    return p;
}

int TSerie::scoreForMatch(int round, int match, int playerIdx)
{
    auto m = getMatchForRound(round, match);
    if (!m) return -1;

    if (playerIdx == 0)
        return m->get_playerScore1();
    else
        return m->get_playerScore2();
}

bool TSerie::winnerForMatch(int round, int match, int playerIdx)
{
    auto m = getMatchForRound(round, match);
    if (!m) return false;

    if (playerIdx == 0)
    {
        if (m->get_player1() && m->get_isBye())
            return true; //when bye player is automatically a winner
        return m->get_playerWinner1();
    }
    else
    {
        if (m->get_player2() && m->get_isBye())
            return true; //when bye player is automatically a winner
        return m->get_playerWinner2();
    }
}

void TSerie::clickedOnMatch(int round, int match)
{
    QMenu menu;
    QAction *action = nullptr;
    auto m = getMatchForRound(round, match);
    bool actionDisabled = false;

    if (m)
    {
        //disable action if
        if (m->get_isBye() || !m->get_player1() || !m->get_player2() ||
            get_status() == "finished")
            actionDisabled = true;
    }

    action = menu.addAction(QIcon::fromTheme("high-score"), tr("Score du match"));
    if (actionDisabled) action->setDisabled(true);
    connect(action, &QAction::triggered, this, [=]()
    {
        DialogEditScore d(m);
        if (d.exec() == QDialog::Accepted)
        {
            m->update_playerScore1(d.getScorePlayer1());
            m->update_playerScore2(d.getScorePlayer2());

            //update winner
            m->update_playerWinner1(m->get_playerScore1() > m->get_playerScore2());
            m->update_playerWinner2(!m->get_playerWinner1());

            updateNextMatches();
            emit matchesUpdated();
            emit currentRoundChanged(m_currentRound); //force emit this signal to update broadcast layout if needed
        }
    });

    action = menu.addAction(QIcon::fromTheme("trash"), tr("Effacer le score"));
    if (actionDisabled) action->setDisabled(true);
    connect(action, &QAction::triggered, this, [=]()
    {
        m->clearScore();
        updateNextMatches();
        emit matchesUpdated();
        emit currentRoundChanged(m_currentRound); //force emit this signal to update broadcast layout if needed
    });

    if (round == 0 &&
        get_tournamentType() == "single")
    {
        action = menu.addAction(QIcon::fromTheme("athlete"), tr("Changer joueurs"));
        if (get_status() != "stopped")
            action->setDisabled(true);
        connect(action, &QAction::triggered, this, [=]()
        {
            DialogChangePlayer d(players, m, get_isDouble());
            if (d.exec() == QDialog::Accepted)
            {
                m->update_player1(d.getPlayer1());
                m->update_player2(d.getPlayer2());

                if ((m->get_player1() && !m->get_player2()) ||
                    (!m->get_player1() && m->get_player2()))
                    m->update_isBye(true);
                else
                    m->update_isBye(false);

                updateNextMatches();
                emit matchesUpdated();
            }
        });
    }

    menu.exec(QCursor::pos());
}

QList<TSerie::UnplayedMatch> TSerie::unplayedNextMatches()
{
    QList<TSerie::UnplayedMatch> lst;

    for (int i = 0;i < allMatches.count();i++)
    {
        auto round = allMatches.at(i);
        for (int j = 0;j < round->count();j++)
        {
            auto match = round->at(j);
            if (match->get_playerWinner1() ||
                match->get_playerWinner2())
            {
                //match finished
                continue;
            }

            if (!match->get_player1() || !match->get_player2())
            {
                //bye or empty match
                continue;
            }

            UnplayedMatch m;
            m.round = i;
            m.match = j;
            m.p_match = match;
            lst.append(m);
        }
    }

    return lst;
}

void TSerie::playersModelChanged()
{
    int rounds = 0;

    //Min players check
    if (players->rowCount() < 3)
    {
        update_rounds(0);
        update_currentRound(0);
        return;
    }

    if (get_tournamentType() == "single")
    {
        int firstround = nearestPowerOf2(players->rowCount()) / 2;
        while (firstround > 0 && ++rounds)
            firstround = firstround / 2;
    }
    else if (get_tournamentType() == "roundrobin")
    {
        rounds = players->rowCount();
        if (rounds % 2 == 0)
            rounds--;
    }

    update_rounds(rounds);

    qDebug() << "Serie: " << get_name() << " players:" << players->rowCount() << " rounds: " << rounds;

    // Do not reset matches if the serie is currently playing or finished,
    // as prepareMatches() would wipe all scores and bracket progression.
    // rounds must still be computed above so QML can render the bracket.
    if (get_status() == "playing" || get_status() == "finished")
        return;

    prepareMatches();
    updateCurrentRound();

    emit matchesUpdated();
}
