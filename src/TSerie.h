#ifndef TSERIE_H
#define TSERIE_H

#include <QObject>

class TSerie : public QObject
{
    Q_OBJECT
public:
    explicit TSerie(QObject *parent = nullptr);
};

#endif // TSERIE_H
