#include "Tournament.h"
#include <QJsonObject>
#include <QJsonArray>
#include "TStorage.h"

Tournament::Tournament(QObject *parent):
    QObject{parent}
{
    series = new QQmlObjectListModel<TSerie>(this, "name");
    update_series(series);
    tables = new QQmlObjectListModel<TTable>(this, "tableNumber");
    update_tables(tables);
    set_defaultViewVisible(true);

    connect(this, &Tournament::defaultViewVisibleChanged, this, [this]()
    {
        TStorage::Instance()->saveToDisk(this);
    });
}

void Tournament::addSerie(TSerie *s)
{
    s->setParent(this);
    series->append(s);

    connect(s, &TSerie::matchesUpdated, this, [this]()
    {
        TStorage::Instance()->saveToDisk(this);
    });

    connect(s, &TSerie::viewVisibleChanged, this, [this]()
    {
        TStorage::Instance()->saveToDisk(this);
    });

    connect(s, &TSerie::statusChanged, this, &Tournament::seriesStatusChanged);
    emit seriesStatusChanged();
}

void Tournament::removeSerie(int idx)
{
    if (idx < 0 || idx >= series->count())
        return;

    auto s = series->at(idx);
    disconnect(s, &TSerie::statusChanged, this, &Tournament::seriesStatusChanged);

    series->remove(idx);
    delete s;

    emit seriesStatusChanged();
}

TSerie *Tournament::getSerie(int idx)
{
    if (idx < 0 || idx >= series->count())
        return nullptr;
    return series->at(idx);
}

TSerie *Tournament::getSerieUid(QString uid)
{
    for (int i = 0;i < series->count();i++)
    {
        if (series->at(i)->get_serieUid() == uid)
            return series->at(i);
    }

    return nullptr;
}

int Tournament::getSerieIndex(TSerie *s)
{
    for (int i = 0;i < series->count();i++)
    {
        if (series->at(i)->get_serieUid() == s->get_serieUid())
            return i;
    }

    return -1;
}

void Tournament::addTable(TTable *t)
{
    t->setParent(this);
    tables->append(t);

    connect(t, &TTable::matchChanged, this, [this]()
            {
                TStorage::Instance()->saveToDisk(this);
            });
}

void Tournament::removeTable(int idx)
{
    if (idx < 0 || idx >= tables->count())
        return;

    auto t = tables->at(idx);
    tables->remove(idx);
    delete t;
}

TTable *Tournament::getTable(int idx)
{
    if (idx < 0 || idx >= tables->count())
        return nullptr;
    return tables->at(idx);
}

TTable *Tournament::getTableFromNumber(int num)
{
    for (int i = 0;i < tables->count();i++)
    {
        if (tables->at(i)->get_tableNumber() == num)
            return tables->at(i);
    }

    return nullptr;
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
    t->update_timeBroadcastChange(obj["time_broadcast_change"].toInt());
    if (t->get_timeBroadcastChange() < 2000)
        t->update_timeBroadcastChange(10000);
    t->update_broadcastScrollSpeed(obj["broadcast_scroll_speed"].toInt());
    if (t->get_broadcastScrollSpeed() < 10)
        t->update_broadcastScrollSpeed(80);
    t->set_defaultViewVisible(obj.contains("default_view_visible") ? obj["default_view_visible"].toBool() : true);

    QJsonArray arr = obj["series"].toArray();
    for (int i = 0;i < arr.count();i++)
    {
        auto o = arr.at(i).toObject();
        TSerie *s = TSerie::fromJson(o);

        if (s)
            t->addSerie(s);
    }

    arr = obj["tables"].toArray();
    for (int i = 0;i < arr.count();i++)
    {
        auto o = arr.at(i).toObject();
        TTable *table = TTable::fromJson(t, o);

        if (table)
            t->addTable(table);
    }

    return t;
}

QJsonObject Tournament::toJson()
{
    QJsonArray arr;
    for (int i = 0;i < series->count();i++)
        arr.append(series->at(i)->toJson());

    QJsonArray arr2;
    for (int i = 0;i < tables->count();i++)
        arr2.append(tables->at(i)->toJson());

    return {
        { "uuid", get_uuid() },
        { "name", get_name() },
        { "date", get_date().toString(Qt::ISODate) },
        { "status", get_status() },
        { "series", arr },
        { "tables", arr2 },
        { "info_text", get_infoText() },
        { "time_broadcast_change", get_timeBroadcastChange() },
        { "broadcast_scroll_speed", get_broadcastScrollSpeed() },
        { "default_view_visible", get_defaultViewVisible() },
    };
}
