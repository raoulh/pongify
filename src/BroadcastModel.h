#ifndef BROADCASTMODEL_H
#define BROADCASTMODEL_H

#include <QObject>
#include "qqmlhelpers.h"

class BroadcastView: public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(QObject *, viewSerie)
    QML_READONLY_PROPERTY(int, viewSerieIndex)
    QML_READONLY_PROPERTY(QString, viewUrl)
    QML_READONLY_PROPERTY(QString, name)
    QML_WRITABLE_PROPERTY(bool, viewVisible)
    QML_WRITABLE_PROPERTY(int, viewTime)

public:
    explicit BroadcastView(QObject *parent = nullptr);
};

#endif // BROADCASTMODEL_H
