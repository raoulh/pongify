#include "TSerie.h"
#include <QJsonArray>
#include <QCollator>
#include <algorithm>
#include <random>
#include <cmath>
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
    update_handicapTable({0, 1, 2, 3, 4, 4, 5});
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

    //Load handicap table, fallback to basic [0,1,2,3,4] for backward compat
    if (obj.contains("handicapTable"))
    {
        QVariantList table;
        for (const auto &v : obj["handicapTable"].toArray())
            table.append(v.toInt());
        t->update_handicapTable(table);
    }
    else
    {
        t->update_handicapTable({0, 1, 2, 3, 4});
    }

    t->set_serieUid(obj["uid"].toString());
    t->set_viewVisible(obj.contains("view_visible") ? obj["view_visible"].toBool() : true);
    t->update_startTime(obj["start_time"].toString());
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

    QJsonArray handicapArr;
    for (const auto &v : get_handicapTable())
        handicapArr.append(v.toInt());
    obj.insert("handicapTable", handicapArr);

    obj.insert("uid", get_serieUid());
    obj.insert("view_visible", get_viewVisible());
    obj.insert("start_time", get_startTime());

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

int TSerie::computeHandicap(int ecart) const
{
    const auto &table = get_handicapTable();
    if (table.isEmpty() || ecart <= 0)
        return 0;
    if (ecart >= table.size())
        return table.last().toInt();
    return table.at(ecart).toInt();
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

void TSerie::replaceAllPlayers(PlayerModel *source)
{
    // Temporarily disconnect to prevent N+1 intermediate playersModelChanged()
    // calls during the clear+append sequence. Each intermediate call would
    // destroy and recreate all TMatch objects via prepareMatches(), risking
    // use-after-free in QML bindings observing the serie.
    disconnect(players, &PlayerModel::playersChanged, this, &TSerie::playersModelChanged);

    clearPlayers();
    scoresWinners.clear();

    for (int i = 0; i < source->rowCount(); i++)
        players->appendClone(source->item(i));

    connect(players, &PlayerModel::playersChanged, this, &TSerie::playersModelChanged);
    playersModelChanged();
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

int TSerie::meetingRound(int slot1, int slot2)
{
    if (slot1 == slot2)
        return 0;

    int diff = slot1 ^ slot2;
    return 1 + static_cast<int>(std::log2(diff));
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
    QSet<Player *> seededPlayers;
    for (int i = 0;i < seedMax;i++)
    {
        auto matchIdx = seedPlayer(i + 1, firstRound->count() * 2);
        firstRound->at(matchIdx / 2)->update_player1(allp.at(i));
        seededPlayers.insert(allp.at(i));

        if (byeCount > 0)
        {
            firstRound->at(matchIdx / 2)->update_player2(nullptr);
            firstRound->at(matchIdx / 2)->update_isBye(true);
            byeCount--;
        }
    }

    //remove seeded players from list (already sorted by ranking)
    allp.remove(0, seedMax);

    //Build map of slots already occupied by seeded players, grouped by club
    int totalSlots = firstRound->count() * 2;
    QHash<QString, QVector<int>> clubSlots;
    for (int i = 0;i < firstRound->count();i++)
    {
        auto m = firstRound->at(i);
        if (m->get_player1())
        {
            QString club = m->get_player1()->get_club();
            if (!club.isEmpty())
                clubSlots[club].append(i * 2);
        }
        if (m->get_player2())
        {
            QString club = m->get_player2()->get_club();
            if (!club.isEmpty())
                clubSlots[club].append(i * 2 + 1);
        }
    }

    //Assign remaining byes to the best remaining players (by ranking, already sorted)
    QSet<Player *> byePlayers;
    for (int i = 0;i < byeCount && i < allp.count();i++)
        byePlayers.insert(allp.at(i));

    //Separate players with a club from those without
    QVector<Player *> playersWithClub;
    QVector<Player *> playersNoClub;
    for (auto *p : allp)
    {
        if (p->get_club().isEmpty())
            playersNoClub.append(p);
        else
            playersWithClub.append(p);
    }

    //Group players by club, sorted by group size descending (most constrained first)
    QHash<QString, QVector<Player *>> clubGroups;
    for (auto *p : playersWithClub)
        clubGroups[p->get_club()].append(p);

    QList<QString> clubKeys = clubGroups.keys();
    std::sort(clubKeys.begin(), clubKeys.end(),
              [&clubGroups](const QString &a, const QString &b)
    {
        return clubGroups[a].count() > clubGroups[b].count();
    });

    //Build list of all free slots
    //A slot is: match_index * 2 for player1, match_index * 2 + 1 for player2
    //Slots needing a bye: player goes in player1 (even slot), player2 = nullptr + isBye
    //We track free matches (where player1 is empty) and half-free matches (player1 set, player2 empty, not bye)
    auto placePlayer = [&](Player *player)
    {
        bool needsBye = byePlayers.contains(player);
        QString club = player->get_club();

        //Collect candidate slots with their separation score
        struct Candidate { int matchIdx; bool asPlayer1; int score; };
        QVector<Candidate> candidates;

        for (int i = 0;i < firstRound->count();i++)
        {
            auto m = firstRound->at(i);

            if (needsBye)
            {
                //Bye players need an empty match (player1 slot)
                if (!m->get_player1() && !m->get_player2() && !m->get_isBye())
                    candidates.append({i, true, 0});
            }
            else
            {
                //Non-bye: can go in player1 of empty match, or player2 of half-filled non-bye match
                if (!m->get_player1() && !m->get_player2() && !m->get_isBye())
                    candidates.append({i, true, 0});
                if (m->get_player1() && !m->get_player2() && !m->get_isBye())
                    candidates.append({i, false, 0});
            }
        }

        if (candidates.isEmpty())
            return;

        //Score each candidate: minimum meetingRound with any same-club slot already placed
        const QVector<int> &sameClubSlots = clubSlots.value(club);

        for (auto &c : candidates)
        {
            int slot = c.asPlayer1 ? c.matchIdx * 2 : c.matchIdx * 2 + 1;

            if (club.isEmpty() || sameClubSlots.isEmpty())
            {
                c.score = totalSlots; //maximum score, no constraint
            }
            else
            {
                int minRound = totalSlots;
                for (int s : sameClubSlots)
                {
                    int r = meetingRound(slot, s);
                    if (r < minRound)
                        minRound = r;
                }
                c.score = minRound;
            }
        }

        //Find best score
        int bestScore = 0;
        for (const auto &c : candidates)
        {
            if (c.score > bestScore)
                bestScore = c.score;
        }

        //Collect all candidates with the best score
        QVector<Candidate> best;
        for (const auto &c : candidates)
        {
            if (c.score == bestScore)
                best.append(c);
        }

        //Pick randomly among the best candidates
        static std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> dist(0, best.count() - 1);
        auto chosen = best.at(dist(rng));

        //Place the player
        auto m = firstRound->at(chosen.matchIdx);
        int placedSlot;
        if (chosen.asPlayer1)
        {
            m->update_player1(player);
            placedSlot = chosen.matchIdx * 2;

            if (needsBye)
            {
                m->update_player2(nullptr);
                m->update_isBye(true);
            }
        }
        else
        {
            m->update_player2(player);
            placedSlot = chosen.matchIdx * 2 + 1;
        }

        //Update club slots tracking
        if (!club.isEmpty())
            clubSlots[club].append(placedSlot);
    };

    //Shuffle each group once upfront
    for (const auto &clubName : clubKeys)
    {
        auto &group = clubGroups[clubName];
        std::shuffle(group.begin(), group.end(), std::mt19937{std::random_device{}()});
    }
    std::shuffle(playersNoClub.begin(), playersNoClub.end(), std::mt19937{std::random_device{}()});

    //Place bye players FIRST — they require empty matches (no player1, no player2).
    //Non-bye players placed as player1 would consume those empty matches,
    //so all bye players must be placed before any non-bye player.
    for (const auto &clubName : clubKeys)
    {
        for (auto *p : clubGroups[clubName])
        {
            if (byePlayers.contains(p))
                placePlayer(p);
        }
    }
    for (auto *p : playersNoClub)
    {
        if (byePlayers.contains(p))
            placePlayer(p);
    }

    //Then place non-bye players (can go as player1 or player2)
    for (const auto &clubName : clubKeys)
    {
        for (auto *p : clubGroups[clubName])
        {
            if (!byePlayers.contains(p))
                placePlayer(p);
        }
    }
    for (auto *p : playersNoClub)
    {
        if (!byePlayers.contains(p))
            placePlayer(p);
    }

    //Repair phase: fix same-club conflicts in first round by swapping player2s
    auto getClub = [](Player *p) -> QString {
        return p ? p->get_club() : QString();
    };

    auto hasConflict = [&getClub](TMatch *m) -> bool {
        if (m->get_isBye() || !m->get_player1() || !m->get_player2())
            return false;
        QString c1 = getClub(m->get_player1());
        QString c2 = getClub(m->get_player2());
        return !c1.isEmpty() && c1 == c2;
    };

    for (int i = 0;i < firstRound->count();i++)
    {
        auto mi = firstRound->at(i);
        if (!hasConflict(mi))
            continue;

        bool fixed = false;

        //Try swapping player2 of conflicting match with player2 of another match
        for (int j = 0;j < firstRound->count() && !fixed;j++)
        {
            if (j == i) continue;
            auto mj = firstRound->at(j);
            if (mj->get_isBye() || !mj->get_player1() || !mj->get_player2())
                continue;

            //Simulate swap: mi.p2 <-> mj.p2
            Player *pi2 = mi->get_player2();
            Player *pj2 = mj->get_player2();

            //Check new match i: mi.p1 vs pj2
            QString ci1 = getClub(mi->get_player1());
            QString cj2 = getClub(pj2);
            bool newConflictI = !ci1.isEmpty() && ci1 == cj2;

            //Check new match j: mj.p1 vs pi2
            QString cj1 = getClub(mj->get_player1());
            QString ci2 = getClub(pi2);
            bool newConflictJ = !cj1.isEmpty() && cj1 == ci2;

            if (!newConflictI && !newConflictJ)
            {
                mi->update_player2(pj2);
                mj->update_player2(pi2);
                fixed = true;
            }
        }

        if (fixed)
            continue;

        //Try swapping player1 (only non-seeded) with player1 of another match
        for (int j = 0;j < firstRound->count() && !fixed;j++)
        {
            if (j == i) continue;
            auto mj = firstRound->at(j);
            if (mj->get_isBye() || !mj->get_player1() || !mj->get_player2())
                continue;

            Player *pi1 = mi->get_player1();
            Player *pj1 = mj->get_player1();

            //Don't move seeded players
            if (seededPlayers.contains(pi1) || seededPlayers.contains(pj1))
                continue;

            //Simulate swap: mi.p1 <-> mj.p1
            QString ci2 = getClub(mi->get_player2());
            QString cj1 = getClub(pj1);
            bool newConflictI = !ci2.isEmpty() && ci2 == cj1;

            QString cj2 = getClub(mj->get_player2());
            QString ci1 = getClub(pi1);
            bool newConflictJ = !cj2.isEmpty() && cj2 == ci1;

            if (!newConflictI && !newConflictJ)
            {
                mi->update_player1(pj1);
                mj->update_player1(pi1);
                fixed = true;
            }
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

    if (allMatches.isEmpty())
        return;

    //Podium already validated, fill the QML model and leave
    if (get_podiumValidated())
    {
        winners->clear();

        for (int i = 0;i < 8 && i < scoresWinners.count();i++)
            winners->append(scoresWinners.at(i).player, false);

        return;
    }

    if (get_currentRound() < allMatches.count() - 1)
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
