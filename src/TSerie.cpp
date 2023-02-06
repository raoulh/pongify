#include "TSerie.h"
#include <QJsonArray>

TSerie::TSerie(QObject *parent):
    QObject{parent}
{
    players = PlayerModel::createEmpty();
    update_players(players);
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
