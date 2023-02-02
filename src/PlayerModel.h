#ifndef PLAYERMODEL_H
#define PLAYERMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>
#include <QJsonObject>
#include "qqmlhelpers.h"

#include <QAbstractListModel>

class Player;

class PlayerModel: public QAbstractListModel
{
    Q_OBJECT
public:
    static PlayerModel *Instance()
    {
        static PlayerModel m;
        return &m;
    }
    virtual ~PlayerModel() override;

    enum
    {
        RoleFirstName = Qt::UserRole + 1,
        RoleLastName,
        RoleLicense,
        RoleLicenseValid,
        RoleRanking,
        RoleClub,
    };

    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QHash<int,QByteArray> roleNames() const override;

    void loadPlayer(const QJsonObject &obj);
    Player *item(int row);
    QStringList getClubs() { return clubs; }

public slots:
    void loadCache(); //current model is cleared
    void saveCache();

private:
    PlayerModel(QObject *parent = nullptr);

    QStringList headers;
    QVector<Player *> players;

    QStringList clubs;
};

class Player : public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(QString, firstName)
    QML_READONLY_PROPERTY(QString, lastName)
    QML_READONLY_PROPERTY(QString, license)
    QML_READONLY_PROPERTY(bool, licenseValid)
    QML_READONLY_PROPERTY(QString, ranking)
    QML_READONLY_PROPERTY(QString, club)

public:
    explicit Player(QObject *parent = nullptr);

    QJsonObject toJson();
};

class PlayerFilterModel: public QSortFilterProxyModel
{
    Q_OBJECT
    QML_READONLY_PROPERTY(double, totalBalance)
public:
    explicit PlayerFilterModel(QObject *parent = 0);

    Q_INVOKABLE int indexToSource(int idx);
    Q_INVOKABLE int indexFromSource(int idx);

    Q_INVOKABLE void setSearchName(QString s);
    Q_INVOKABLE void setClub(QString s);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    QString terms;
    QString club;
};

#endif // PLAYERMODEL_H
