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

    connect(this, &TMatch::player1Changed, this, [this](auto *player)
    {
        connect(player, &Player::destroyed, this, [this]()
        {
            update_player1(nullptr);
        });
    });

    connect(this, &TMatch::player2Changed, this, [this](auto *player)
    {
        connect(player, &Player::destroyed, this, [this]()
        {
            update_player2(nullptr);
        });
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

    connect(players, &PlayerModel::playersChanged, this, &TSerie::playersModelChanged);
    playersModelChanged();

    update_isDouble(false);
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
    //Serie status:
    // - stopped  --> by default when created
    // - playing  --> once the serie is started
    // - finished --> when the score is entered for the final match, hall of fame can be displayed for the serie

    if (get_status() != "stopped") return;

    if (allMatches.count() < 2)
        return;

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
                                 "Tous les joueurs ne sont pas placés dans le premier tour.\nImpossible de démarrer la série.");

            return;
        }
    }

    auto addToSet = [](QSet<Player *> &s, Player *p)
    {
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

    update_status("playing");
}

void TSerie::stopSerie()
{
    if (get_status() != "playing") return;



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

    updateNextMatches();
    emit matchesUpdated();
}

void TSerie::removeAllPlayers()
{
    clearAllMatches();
    prepareMatches();
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

        auto p = m->get_player1();
        QQmlEngine::setObjectOwnership(p, QQmlEngine::CppOwnership);

        return p;
    }

    return nullptr;
}

QObject *TSerie::getPlayer2(int round, int match)
{
    if (get_tournamentType() == "single")
    {
        auto m = getMatchForRound(round, match);
        if (!m) return nullptr;

        auto p = m->get_player2();
        QQmlEngine::setObjectOwnership(p, QQmlEngine::CppOwnership);

        return p;
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

    return false;
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
        if (m->get_isBye() || !m->get_player1() || !m->get_player1())
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
        }
    });

    action = menu.addAction(QIcon::fromTheme("trash"), tr("Effacer le score"));
    if (actionDisabled) action->setDisabled(true);
    connect(action, &QAction::triggered, this, [=]()
    {
        m->clearScore();
        updateNextMatches();
        emit matchesUpdated();
    });

    if (round == 0)
    {
        action = menu.addAction(QIcon::fromTheme("athlete"), tr("Changer joueurs"));
        if (get_status() != "stopped")
            action->setDisabled(true);
        connect(action, &QAction::triggered, this, [=]()
        {
            DialogChangePlayer d(players, m, get_isDouble());
            if (d.exec() == QDialog::Accepted)
            {
                m->update_player1(d.getPlayer1_1());
                m->update_player2(d.getPlayer2_1());

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

    emit matchesUpdated();
}
