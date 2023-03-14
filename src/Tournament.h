#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include <QObject>
#include <QDate>
#include <QQmlObjectListModel.h>
#include "qqmlhelpers.h"
#include "TSerie.h"

class Tournament: public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(QString, uuid)
    QML_READONLY_PROPERTY(QString, name)
    QML_READONLY_PROPERTY(QDateTime, date)
    QML_READONLY_PROPERTY(QString, status)
    QML_READONLY_PROPERTY(QObject *, series)
    QML_READONLY_PROPERTY(QString, infoText)

public:
    explicit Tournament(QObject *parent = nullptr);

    void addSerie(TSerie *s);
    void removeSerie(int idx);
    TSerie *getSerie(int idx);
    int serieCount() { return series->count(); }

    static Tournament *fromJson(const QJsonObject &obj);
    QJsonObject toJson();

signals:
    void seriesStatusChanged(); //emited if any serie has changed its status

private:
    QQmlObjectListModel<TSerie> *series;
};

#endif // TOURNAMENT_H
