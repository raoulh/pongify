#include "Tournament.h"
#include <QJsonObject>

Tournament::Tournament(QObject *parent):
    QObject{parent}
{

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

    //TODO load series

    return t;
}

QJsonObject Tournament::toJson()
{
    return {
        { "uuid", get_uuid() },
        { "name", get_name() },
        { "date", get_date().toString(Qt::ISODate) },
        { "status", get_status() },
    };
}
