#include <QString>
#include <QVector>
#include "TSerie.h"

class Tournament {
public:
    Tournament(const QString& name, const QVector<TSerie>& series);

    QString getName() const;
    QVector<TSerie> getSeries() const;
    void start();
    void stop();
    bool isStarted() const;
    bool isStopped() const;

private:
    QString mName;
    QVector<TSerie> mSeries;
    bool mIsStarted;
    bool mIsStopped;
};

Tournament::Tournament(const QString& name, const QVector<TSerie>& series)
    : mName(name), mSeries(series), mIsStarted(false), mIsStopped(false)
{}

QString Tournament::getName() const {
    return mName;
}

QVector<TSerie> Tournament::getSeries() const {
    return mSeries;
}

void Tournament::start() {
    mIsStarted = true;
}

void Tournament::stop() {
    mIsStopped = true;
}

bool Tournament::isStarted() const {
    return mIsStarted;
}

bool Tournament::isStopped() const {
    return mIsStopped;
}