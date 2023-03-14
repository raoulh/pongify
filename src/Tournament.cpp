#include "Tournament.h"
#include <QJsonObject>
#include <QJsonArray>
#include "TStorage.h"

Tournament::Tournament(QObject *parent):
    QObject{parent}
{
    series = new QQmlObjectListModel<TSerie>(this, "name");
    update_series(series);
}

void Tournament::addSerie(TSerie *s)
{
    s->setParent(this);
    series->append(s);

    connect(s, &TSerie::matchesUpdated, this, [this]()
    {
        TStorage::Instance()->saveToDisk(this);
    });

    connect(s, &TSerie::statusChanged, this, &Tournament::seriesStatusChanged);
}

void Tournament::removeSerie(int idx)
{
    if (idx < 0 || idx >= series->count())
        return;

    auto s = series->at(idx);
    disconnect(s, &TSerie::statusChanged, this, &Tournament::seriesStatusChanged);

    series->remove(idx);
    delete s;
}

TSerie *Tournament::getSerie(int idx)
{
    if (idx < 0 || idx >= series->count())
        return nullptr;
    return series->at(idx);
}

Tournament *Tournament::fromJson(const QJsonObject &obj)
{
    if (!obj.contains("name") ||
        !obj.contains("date") ||
        !obj.contains("status") ||
        !obj.contains("uuid"))
    {
        return nullptr;
    }

    auto t = new Tournament();

    t->update_name(obj["name"].toString());
    t->update_date(QDateTime::fromString(obj["date"].toString(), Qt::ISODate));
    t->update_status(obj["status"].toString());
    t->update_uuid(obj["uuid"].toString());
    t->update_infoText(obj["info_text"].toString());

    QJsonArray arr = obj["series"].toArray();
    for (int i = 0;i < arr.count();i++)
    {
        auto o = arr.at(i).toObject();
        TSerie *s = TSerie::fromJson(o);

        if (s)
            t->addSerie(s);
    }

    return t;
}

QJsonObject Tournament::toJson()
{
    QJsonArray arr;
    for (int i = 0;i < series->count();i++)
        arr.append(series->at(i)->toJson());

    return {
        { "uuid", get_uuid() },
        { "name", get_name() },
        { "date", get_date().toString(Qt::ISODate) },
        { "status", get_status() },
        { "series", arr },
        { "info_text", get_infoText() },
    };
}
