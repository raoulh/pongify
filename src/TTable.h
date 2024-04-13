#ifndef TTABLE_H
#define TTABLE_H

#include <QObject>
#include <QQmlObjectListModel.h>
#include "qqmlhelpers.h"
#include "TSerie.h"

class Tournament;

class TTable: public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(int, tableNumber)
    QML_READONLY_PROPERTY(QObject *, match) //current match being played on this table of type TMatch*
    QML_READONLY_PROPERTY(bool, free)

    QML_READONLY_PROPERTY(Player *, player1)
    QML_READONLY_PROPERTY(Player *, player2)

    QML_READONLY_PROPERTY(QString, player1_firstName)
    QML_READONLY_PROPERTY(QString, player1_lastName)
    QML_READONLY_PROPERTY(QString, player2_firstName)
    QML_READONLY_PROPERTY(QString, player2_lastName)

    QML_READONLY_PROPERTY(QString, player1Second_firstName)
    QML_READONLY_PROPERTY(QString, player1Second_lastName)
    QML_READONLY_PROPERTY(QString, player2Second_firstName)
    QML_READONLY_PROPERTY(QString, player2Second_lastName)

    QML_READONLY_PROPERTY(bool, isDouble)

    QML_READONLY_PROPERTY(QString, serieName)

public:
    explicit TTable(QObject *parent = nullptr);
    virtual ~TTable();

    static TTable *fromJson(Tournament *tn, const QJsonObject &obj);
    QJsonObject toJson();

    void setRoundMatch(TSerie *serie, int round, int match);

    TSerie *getSerie() { return currentSerie; }
    int getRoundIdx() { return currentRound; }
    int getMatchIdx() { return currentMatchIndex; }

    void clearTable();

private:
    Tournament *tournament = nullptr;

    TSerie *currentSerie = nullptr;
    int currentRound = -1;
    int currentMatchIndex = -1;

    QVector<QMetaObject::Connection> connections;
};

#endif // TTABLE_H
