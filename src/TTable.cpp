#include "TTable.h"
#include "Tournament.h"

TTable::TTable(QObject *parent):
    QObject{parent}
{
    clearTable();

    connect(this, &TTable::matchChanged, this, [this](QObject *m)
    {
        update_free(get_match() == nullptr);

        if (m)
        {
            auto match = reinterpret_cast<TMatch *>(m);

            auto checkFinished = [this, match]()
            {
                qDebug() << "Checking if match is finished";

                if (match->get_playerWinner1() || match->get_playerWinner2())
                    clearTable(); //match is finished. clear table

                if (!match->get_player1() || !match->get_player2())
                    clearTable(); //match is reset. clear table
            };

            //check if match is finished
            auto conn = connect(match, &TMatch::playerWinner1Changed, this, checkFinished);
            connections.append(conn);
            conn = connect(match, &TMatch::playerWinner2Changed, this, checkFinished);
            connections.append(conn);

            //also reset table if match is removed
            conn = connect(match, &TMatch::player1Changed, this, checkFinished);
            connections.append(conn);
            conn = connect(match, &TMatch::player2Changed, this, checkFinished);
            connections.append(conn);
        }
    });
}

TTable::~TTable()
{
}

TTable *TTable::fromJson(Tournament *tn, const QJsonObject &obj)
{
    if (!obj.contains("tableNumber"))
        return nullptr;

    auto t = new TTable(tn);
    t->update_tableNumber(obj["tableNumber"].toInt());

    auto uid = obj["serieUid"].toString();
    auto serie = tn->getSerieUid(uid);
    auto round = obj["round"].toInt();
    auto match = obj["match"].toInt();

    if (serie)
        t->setRoundMatch(serie, round, match);

    return t;
}

QJsonObject TTable::toJson()
{
    QJsonObject obj;
    obj["tableNumber"] = get_tableNumber();
    obj["serieUid"] = currentSerie? currentSerie->get_serieUid(): "";
    obj["round"] = currentRound;
    obj["match"] = currentMatchIndex;

    return obj;
}

void TTable::setRoundMatch(TSerie *serie, int round, int match)
{
    currentSerie = serie;
    currentRound = round;
    currentMatchIndex = match;

    update_serieName(serie? serie->get_name(): "");
    auto p1 = reinterpret_cast<Player *>(serie? serie->getPlayer1(round, match): nullptr);
    auto p2 = reinterpret_cast<Player *>(serie? serie->getPlayer2(round, match): nullptr);
    auto dbl = reinterpret_cast<Player *>(serie? serie->get_isDouble(): false);

    update_player1(p1);
    update_player2(p2);
    update_isDouble(dbl);

    if (p1)
    {
        update_player1_firstName(p1->get_firstName());
        update_player1_lastName(p1->get_lastName());
        update_player1Second_firstName(p1->get_firstNameSecond());
        update_player1Second_lastName(p1->get_lastNameSecond());
    }
    else
    {
        update_player1_firstName("");
        update_player1_lastName("");
        update_player1Second_firstName("");
        update_player1Second_lastName("");
    }

    if (p2)
    {
        update_player2_firstName(p2->get_firstName());
        update_player2_lastName(p2->get_lastName());
        update_player2Second_firstName(p2->get_firstNameSecond());
        update_player2Second_lastName(p2->get_lastNameSecond());
    }
    else
    {
        update_player2_firstName("");
        update_player2_lastName("");
        update_player2Second_firstName("");
        update_player2Second_lastName("");
    }

    auto m = serie? serie->getMatchForRound(round, match): nullptr;
    update_match(m);
}

void TTable::clearTable()
{
    for (auto &conn: connections)
        disconnect(conn);
    connections.clear();

    currentSerie = nullptr;
    currentRound = -1;
    currentMatchIndex = -1;
    update_match(nullptr);
    update_free(true);
    update_player1_firstName("");
    update_player1_lastName("");
    update_player2_firstName("");
    update_player2_lastName("");
    update_player1Second_firstName("");
    update_player1Second_lastName("");
    update_player2Second_firstName("");
    update_player2Second_lastName("");
    update_serieName("");
    update_player1(nullptr);
    update_player2(nullptr);
    update_isDouble(false);
}
