#ifndef PLAYERMODEL_H
#define PLAYERMODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "qqmlhelpers.h"

class PlayerModel: public QStandardItemModel
{
    Q_OBJECT
    QML_READONLY_PROPERTY(double, totalBalance)
public:
    PlayerModel(QObject *parent = nullptr);
    virtual ~PlayerModel() override {}

    enum
    {
        RoleFirstName = Qt::UserRole + 1,
        RoleLastName,
        RoleLicense,
        RoleLicenseValid,
        RoleRanking,
        RoleClub,
    };

private:
};

class Player : public QObject, public QStandardItem
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

};

class PlayerFilterModel: public QSortFilterProxyModel
{
    Q_OBJECT
    QML_READONLY_PROPERTY(double, totalBalance)
public:
    explicit PlayerFilterModel(QObject *parent = 0);

    Q_INVOKABLE int indexToSource(int idx);
    Q_INVOKABLE int indexFromSource(int idx);

    Q_INVOKABLE void setSearchTerm(QString s);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    QString terms;
};

#endif // PLAYERMODEL_H
