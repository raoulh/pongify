#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include <QObject>
#include <QDate>
#include <QQmlObjectListModel.h>
#include "qqmlhelpers.h"
#include "TSerie.h"
#include "TTable.h"

class Tournament: public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(QString, uuid)
    QML_READONLY_PROPERTY(QString, name)
    QML_READONLY_PROPERTY(QDateTime, date)
    QML_READONLY_PROPERTY(QString, status)
    QML_READONLY_PROPERTY(QObject *, series)
    QML_READONLY_PROPERTY(QString, infoText)
    QML_READONLY_PROPERTY(int, timeBroadcastChange)
    QML_READONLY_PROPERTY(int, broadcastScrollSpeed)
    QML_WRITABLE_PROPERTY(bool, defaultViewVisible)
    QML_READONLY_PROPERTY(QObject *, tables)

public:
    explicit Tournament(QObject *parent = nullptr);

    void addSerie(TSerie *s);
    void removeSerie(int idx);
    TSerie *getSerie(int idx);
    int serieCount() { return series->count(); }
    TSerie *getSerieUid(QString uid);
    int getSerieIndex(TSerie *s);

    void addTable(TTable *t);
    void removeTable(int idx);
    TTable *getTable(int idx);
    int tableCount() { return tables->count(); }
    TTable *getTableFromNumber(int num);

    static Tournament *fromJson(const QJsonObject &obj);
    QJsonObject toJson();

signals:
    void seriesStatusChanged(); //emited if any serie has changed its status

private:
    QQmlObjectListModel<TSerie> *series;
    QQmlObjectListModel<TTable> *tables;
};

#endif // TOURNAMENT_H
