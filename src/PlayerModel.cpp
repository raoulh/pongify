#include "PlayerModel.h"
#include <QCollator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include "Utils.h"
#include <QMessageBox>

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

void PlayerModel::loadPlayer(const QJsonObject &obj)
{
    Player *player = nullptr;

    if (obj["firstname"].toString().isEmpty() ||
        obj["lastname"].toString().isEmpty() ||
        obj["license"].toString().isEmpty() ||
        obj["ranking"].toString().isEmpty())
    {
        qWarning() << "Bad json for player: one of the required param is empty";
        return;
    }

    //Check if player already exist in model
    for (int i = 0;i < rowCount();i++)
    {
        auto it = dynamic_cast<Player *>(item(i));
        if (it->get_license() == obj["license"].toString())
        {
            player = it;
            break;
        }
    }

    if (!player)
        player = new Player();

    player->update_firstName(obj["firstname"].toString());
    player->update_lastName(obj["lastname"].toString());
    player->update_club(obj["club"].toString());
    player->update_license(obj["license"].toString());
    player->update_licenseValid(obj["license_valid"].toBool());
    player->update_ranking(obj["ranking"].toString());

    appendRow(player);
}

void PlayerModel::loadCache()
{
    clear();

    QFile cacheFile(QStringLiteral("%1/players.cache").arg(Utils::getCachePath()));
    if (!cacheFile.open(QFile::ReadOnly))
    {
        QMessageBox::warning(nullptr, "Attention", "La liste des joueurs est vide.\nVeuillez refaire une synchronisation depuis le site du CDSLS");
        return;
    }

    QJsonParseError jerr;
    QJsonDocument jdoc = QJsonDocument::fromJson(cacheFile.readAll(), &jerr);
    if (jerr.error != QJsonParseError::NoError)
    {
        qWarning() << "JSON parse error " << jerr.errorString();
        QMessageBox::warning(nullptr, "Attention", "La liste des joueurs est corrompue.\nVeuillez refaire une synchronisation depuis le site du CDSLS");
        return;
    }

    auto arr = jdoc.array();
    if (arr.isEmpty())
    {
        QMessageBox::warning(nullptr, "Attention", "La liste des joueurs est vide.\nVeuillez refaire une synchronisation depuis le site du CDSLS");
        return;
    }

    for (int i = 0;i < arr.count();i++)
    {
        auto obj = arr.at(i).toObject();
        loadPlayer(obj);
    }
}

void PlayerModel::saveCache()
{
    QJsonArray arr;

    for (int i = 0;i < rowCount();i++)
    {
        auto p = dynamic_cast<Player *>(item(i));
        arr.append(p->toJson());
    }

    QJsonDocument jdoc;
    jdoc.setArray(arr);

    QFile cacheFile(QStringLiteral("%1/players.cache").arg(Utils::getCachePath()));
    if (!cacheFile.open(QFile::ReadWrite))
    {
        QMessageBox::warning(nullptr, "Erreur", "Impossible d'ecrire dans le fichier de cache!");
        return;
    }
    cacheFile.write(jdoc.toJson());
    cacheFile.close();
}

Player::Player(QObject *parent):
    QObject(parent),
    QStandardItem()
{
    update_licenseValid(false);
}

QJsonObject Player::toJson()
{
    QJsonObject obj;
    obj.insert("firstname", get_firstName());
    obj.insert("lastname", get_lastName());
    obj.insert("license", get_license());
    obj.insert("license_valid", get_licenseValid());
    obj.insert("ranking", get_ranking());
    obj.insert("club", get_club());

    return obj;
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
