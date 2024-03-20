#include "TableMatchModel.h"

TableMatchItem::TableMatchItem(QObject *parent):
    QObject{parent}
{
    clearMatch();
}

TableMatchItem::~TableMatchItem()
{
}

void TableMatchItem::setRoundMatch(TSerie *serie, int round, int match)
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

void TableMatchItem::clearMatch()
{
    currentSerie = nullptr;
    currentRound = -1;
    currentMatchIndex = -1;
    update_player1_firstName("");
    update_player1_lastName("");
    update_player2_firstName("");
    update_player2_lastName("");
    update_serieName("");
}
