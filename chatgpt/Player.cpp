#include <QString>

class Player {
public:
    Player(const QString& name, const QString& rank);

    QString getName() const;
    QString getRank() const;

private:
    QString mName;
    QString mRank;
};

Player::Player(const QString& name, const QString& rank)
    : mName(name), mRank(rank)
{}

QString Player::getName() const {
    return mName;
}

QString Player::getRank() const {
    return mRank;
}