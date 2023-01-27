#include "PlayerModel.h"
#include <QCollator>

PlayerModel::PlayerModel(QObject *parent):
    QStandardItemModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[RoleFirstName] = "firstName";
    roles[RoleLastName] = "lastName";
    roles[RoleLicense] = "license";
    roles[RoleLicenseValid] = "lic_valid";
    roles[RoleRanking] = "ranking";
    roles[RoleClub] = "club";
    setItemRoleNames(roles);
}

Player::Player(QObject *parent):
    QObject(parent),
    QStandardItem()
{
    update_licenseValid(false);
}

PlayerFilterModel::PlayerFilterModel(QObject *parent):
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0);
}

int PlayerFilterModel::indexToSource(int idx)
{
    return mapToSource(index(idx, 0)).row();
}

int PlayerFilterModel::indexFromSource(int idx)
{
    return mapFromSource(index(idx, 0)).row();
}

void PlayerFilterModel::setSearchTerm(QString s)
{
    terms = s.toLower();
    invalidate();
}

bool PlayerFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)

    if (terms.isEmpty())
        return true;

    PlayerModel *model = dynamic_cast<PlayerModel *>(sourceModel());
    Player *item = dynamic_cast<Player *>(model->item(source_row));
    auto txt = item->get_firstName().toLower() + item->get_lastName().toLower();

    return txt.contains(terms);
}

bool PlayerFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    PlayerModel *model = dynamic_cast<PlayerModel *>(sourceModel());
    Player *itemLeft = dynamic_cast<Player *>(model->item(left.row()));
    Player *itemRight = dynamic_cast<Player *>(model->item(right.row()));

    QCollator sorter;
    sorter.setNumericMode(true);
    return sorter.compare(itemLeft->get_lastName(), itemRight->get_lastName()) < 0;
}
