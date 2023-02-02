#include "TSerie.h"
#include <QJsonArray>

TSerie::TSerie(QObject *parent):
    QObject{parent}
{
    players = new QQmlObjectListModel<Player>(this, "name");
    update_players(players);
}

TSerie::~TSerie()
{
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
        Player *p = Player::fromJson(o);

        if (p)
            t->players->append(p);
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
    for (int i = 0;i < players->count();i++)
        arr.append(players->at(i)->toJson());

    obj.insert("players", arr);

    return obj;
}
