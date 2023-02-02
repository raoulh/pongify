#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include <QObject>
#include <QDate>
#include "qqmlhelpers.h"

class Tournament: public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(QString, uuid)
    QML_READONLY_PROPERTY(QString, name)
    QML_READONLY_PROPERTY(QDateTime, date)
    QML_READONLY_PROPERTY(QString, status)

public:
    explicit Tournament(QObject *parent = nullptr);

    static Tournament *fromJson(const QJsonObject &obj);
    QJsonObject toJson();
};

#endif // TOURNAMENT_H
