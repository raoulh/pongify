#ifndef TSERIE_H
#define TSERIE_H

#include <QObject>
#include <QQmlObjectListModel.h>
#include "qqmlhelpers.h"
#include "PlayerModel.h"

class TSerie : public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(QString, name)
    QML_READONLY_PROPERTY(QString, ranking)
    QML_READONLY_PROPERTY(QString, tournamentType)
    QML_READONLY_PROPERTY(QObject *, players)

public:
    explicit TSerie(QObject *parent = nullptr);
    virtual ~TSerie();

    static TSerie *fromJson(const QJsonObject &obj);
    QJsonObject toJson();

private:
    QQmlObjectListModel<Player> *players;
};

#endif // TSERIE_H
