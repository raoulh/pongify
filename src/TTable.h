#ifndef TTABLE_H
#define TTABLE_H

#include <QObject>
#include <QQmlObjectListModel.h>
#include "qqmlhelpers.h"
#include "TSerie.h"

class TTable: public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(int, tableNumber)
    QML_READONLY_PROPERTY(QObject *, match) //current match being played on this table

public:
    explicit TTable(QObject *parent = nullptr);
    virtual ~TTable();

    static TTable *fromJson(const QJsonObject &obj);
    QJsonObject toJson();

    void setRoundMatch(TSerie *serie, int round, int match);

    void clearMatch();

    Q_INVOKABLE bool isFree();

private:
    TSerie *currentSerie = nullptr;
    int currentRound = -1;
    int currentMatchIndex = -1;
};

#endif // TTABLE_H
