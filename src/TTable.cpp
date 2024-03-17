#include "TTable.h"

TTable::TTable(QObject *parent):
    QObject{parent}
{
    clearMatch();
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
}

void TTable::clearMatch()
{
    currentSerie = nullptr;
    currentRound = -1;
    currentMatchIndex = -1;
    update_match(nullptr);
}

bool TTable::isFree()
{
    return get_match() == nullptr;
}
