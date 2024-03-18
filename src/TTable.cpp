#include "TTable.h"

TTable::TTable(QObject *parent):
    QObject{parent}
{
    clearMatch();

    connect(this, &TTable::matchChanged, this, [this]()
    {
        update_free(get_match() == nullptr);
    });
}

TTable::~TTable()
{
}

TTable *TTable::fromJson(const QJsonObject &obj)
{
    if (!obj.contains("tableNumber"))
        return nullptr;

    auto t = new TTable();
    t->update_tableNumber(obj["tableNumber"].toInt());
    return t;
}

QJsonObject TTable::toJson()
{
    QJsonObject obj;
    obj["tableNumber"] = get_tableNumber();

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

    if (p1)
    {
        update_player1_firstName(p1->get_firstName());
        update_player1_lastName(p1->get_lastName());
    }
    else
    {
        update_player1_firstName("");
        update_player1_lastName("");
    }

    if (p2)
    {
        update_player2_firstName(p2->get_firstName());
        update_player2_lastName(p2->get_lastName());
    }
    else
    {
        update_player2_firstName("");
        update_player2_lastName("");
    }
}

void TTable::clearMatch()
{
    currentSerie = nullptr;
    currentRound = -1;
    currentMatchIndex = -1;
    update_match(nullptr);
    update_free(true);
    update_player1_firstName("");
    update_player1_lastName("");
    update_player2_firstName("");
    update_player2_lastName("");
    update_serieName("");
}
