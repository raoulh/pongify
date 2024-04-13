#ifndef TABLEMATCHMODEL_H
#define TABLEMATCHMODEL_H

#include <QObject>
#include <QQmlObjectListModel.h>
#include "qqmlhelpers.h"
#include "TSerie.h"

class TableMatchItem: public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(QString, player1_firstName)
    QML_READONLY_PROPERTY(QString, player1_lastName)
    QML_READONLY_PROPERTY(QString, player2_firstName)
    QML_READONLY_PROPERTY(QString, player2_lastName)

    QML_READONLY_PROPERTY(QString, player1Second_firstName)
    QML_READONLY_PROPERTY(QString, player1Second_lastName)
    QML_READONLY_PROPERTY(QString, player2Second_firstName)
    QML_READONLY_PROPERTY(QString, player2Second_lastName)

    QML_READONLY_PROPERTY(QString, serieName)

    QML_READONLY_PROPERTY(bool, isDouble)

public:
    explicit TableMatchItem(QObject *parent = nullptr);
    virtual ~TableMatchItem();

    void setRoundMatch(TSerie *serie, int round, int match);

    TSerie *getSerie() { return currentSerie; }
    int getRoundIdx() { return currentRound; }
    int getMatchIdx() { return currentMatchIndex; }

    void clearMatch();

private:
    TSerie *currentSerie = nullptr;
    int currentRound = -1;
    int currentMatchIndex = -1;
};

#endif // TABLEMATCHMODEL_H
