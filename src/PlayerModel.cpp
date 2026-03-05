#include "PlayerModel.h"
#include <QCollator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include "Utils.h"
#include <QMessageBox>
#include <QQmlEngine>

PlayerModel::PlayerModel(QObject *parent):
    QAbstractListModel(parent)
{
    headers = {{ "License" },
               { "Nom" },
               { "Prénom" },
               { "Classement" },
               { "Club" }};
}

PlayerModel::~PlayerModel()
{
    qDeleteAll(players);
}

PlayerModel *PlayerModel::createEmpty()
{
    return new PlayerModel();
}

QVariant PlayerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= players.count())
        return QVariant();

    if (!players.isEmpty() && !players.at(0)->get_licenseSecond().isEmpty() && role < Qt::UserRole)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return players.at(index.row())->get_license();
        case 1: return players.at(index.row())->get_lastName();
        case 2: return players.at(index.row())->get_firstName();
        case 3: return players.at(index.row())->get_ranking();
        case 4: return players.at(index.row())->get_club();
        default: break;
        }

        break;
    case RoleFirstName: return players.at(index.row())->get_firstName();
    case RoleLastName: return players.at(index.row())->get_lastName();
    case RoleLicense: return players.at(index.row())->get_license();
    case RoleLicenseValid: return players.at(index.row())->get_licenseValid();
    case RoleRanking: return players.at(index.row())->get_ranking();
    case RoleClub: return players.at(index.row())->get_club();
    case RoleFirstNameSecond: return players.at(index.row())->get_firstNameSecond();
    case RoleLastNameSecond: return players.at(index.row())->get_lastNameSecond();
    case RoleLicenseSecond: return players.at(index.row())->get_licenseSecond();
    case RoleLicenseValidSecond: return players.at(index.row())->get_licenseValidSecond();
    case RoleRankingSecond: return players.at(index.row())->get_rankingSecond();
    case RoleClubSecond: return players.at(index.row())->get_clubSecond();
    default: break;
    }

    return QVariant();
}

QVariant PlayerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if (!players.isEmpty() && !players.at(0)->get_licenseSecond().isEmpty())
            return "Equipes";
        if (section >= 0 && section < headers.size())
            return headers.at(section);
    }

    return QVariant();
}

int PlayerModel::rowCount(const QModelIndex &) const
{
    return players.count();
}

int PlayerModel::columnCount(const QModelIndex &) const
{
    if (players.isEmpty())
        return 5;
    else
        return players.at(0)->get_licenseSecond().isEmpty()? 5: 1;
}

QHash<int, QByteArray> PlayerModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[RoleFirstName] = "firstName";
    roles[RoleLastName] = "lastName";
    roles[RoleLicense] = "license";
    roles[RoleLicenseValid] = "lic_valid";
    roles[RoleRanking] = "ranking";
    roles[RoleClub] = "club";
    roles[RoleFirstNameSecond] = "firstNameSecond";
    roles[RoleLastNameSecond] = "lastNameSecond";
    roles[RoleLicenseSecond] = "licenseSecond";
    roles[RoleLicenseValidSecond] = "lic_validSecond";
    roles[RoleRankingSecond] = "rankingSecond";
    roles[RoleClubSecond] = "clubSecond";
    return roles;
}

bool PlayerModel::removeRows(int row, int count, const QModelIndex &)
{
    // When removing all remaining double players, column count changes from 1 to 5.
    // Use beginResetModel to notify the view of the structural change.
    bool columnChange = (players.count() - count == 0) &&
                        !players.isEmpty() && !players.at(0)->get_licenseSecond().isEmpty();

    if (columnChange)
        beginResetModel();
    else
        beginRemoveRows({}, row, row + count - 1);

    for (int i = 0; i < count; ++i)
        delete players.at(row + i);
    players.remove(row, count);

    if (columnChange)
        endResetModel();
    else
        endRemoveRows();

    emit playersChanged();
    return true;
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
    auto idx = -1;
    for (int i = 0;i < players.count();i++)
    {
        auto it = players.at(i);
        if (it->get_license() == obj["license"].toString() ||
            it->get_license() == obj["licenseSecond"].toString())
        {
            player = it;
            idx = i;
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

    player->update_firstNameSecond(obj["firstnameSecond"].toString());
    player->update_lastNameSecond(obj["lastnameSecond"].toString());
    player->update_clubSecond(obj["clubSecond"].toString());
    player->update_licenseSecond(obj["licenseSecond"].toString());
    player->update_licenseValidSecond(obj["license_validSecond"].toBool());
    player->update_rankingSecond(obj["rankingSecond"].toString());

    clubs.append(player->get_club());
    if (!player->get_clubSecond().isEmpty())
        clubs.append(player->get_clubSecond());
    clubs.removeDuplicates();

    if (idx < 0)
    {
        // When inserting a double player into an empty model, the column count
        // changes from 5 to 1. Use beginResetModel to notify the view.
        bool columnChange = players.isEmpty() && !obj["licenseSecond"].toString().isEmpty();
        if (columnChange)
            beginResetModel();
        else
            beginInsertRows({}, players.count(), players.count());

        players.append(player);

        if (columnChange)
            endResetModel();
        else
            endInsertRows();

        emit playersChanged();
    }
}

Player *PlayerModel::item(int row)
{
    if (row >= 0 && row < players.count())
        return players.at(row);
    return nullptr;
}

int PlayerModel::indexFromPlayer(Player *p)
{
    for (int i = 0;i < players.count() && p;i++)
    {
        auto player = players.at(i);
        if (player->get_license() == p->get_license())
            return i;
    }

    return -1;
}

void PlayerModel::appendClone(Player *p)
{
    loadPlayer(p->toJson());
}

void PlayerModel::clear()
{
    beginResetModel();
    qDeleteAll(players);
    players.clear();
    clubs.clear();
    endResetModel();
}

Player *PlayerModel::getFromLicense(QString lic)
{
    for (int i = 0;i < players.count();i++)
    {
        auto p = players.at(i);
        if (lic == p->get_license())
        {
            QQmlEngine::setObjectOwnership(p, QQmlEngine::CppOwnership);
            return p;
        }
    }

    return nullptr;
}

void PlayerModel::loadCache()
{
    qDeleteAll(players);
    players.clear();
    clubs.clear();

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
        auto p = players.at(i);
        arr.append(p->toJson());
    }

    QJsonDocument jdoc;
    jdoc.setArray(arr);

    QFile cacheFile(QStringLiteral("%1/players.cache").arg(Utils::getCachePath()));
    if (!cacheFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        QMessageBox::warning(nullptr, "Erreur", "Impossible d'ecrire dans le fichier de cache!");
        return;
    }
    cacheFile.write(jdoc.toJson());
    cacheFile.close();
}

Player::Player(QObject *parent):
    QObject(parent)
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

    if (get_licenseSecond() != "")
    {
        obj.insert("firstnameSecond", get_firstNameSecond());
        obj.insert("lastnameSecond", get_lastNameSecond());
        obj.insert("licenseSecond", get_licenseSecond());
        obj.insert("license_validSecond", get_licenseValidSecond());
        obj.insert("rankingSecond", get_rankingSecond());
        obj.insert("clubSecond", get_clubSecond());
    }

    return obj;
}

Player *Player::fromJson(const QJsonObject &obj)
{
    if (obj["firstname"].toString().isEmpty() ||
        obj["lastname"].toString().isEmpty() ||
        obj["license"].toString().isEmpty() ||
        obj["ranking"].toString().isEmpty())
    {
        qWarning() << "Bad json for player: one of the required param is empty";
        return nullptr;
    }

    Player *player = new Player();
    player->update_firstName(obj["firstname"].toString());
    player->update_lastName(obj["lastname"].toString());
    player->update_club(obj["club"].toString());
    player->update_license(obj["license"].toString());
    player->update_licenseValid(obj["license_valid"].toBool());
    player->update_ranking(obj["ranking"].toString());

    player->update_firstNameSecond(obj["firstnameSecond"].toString());
    player->update_lastNameSecond(obj["lastnameSecond"].toString());
    player->update_clubSecond(obj["clubSecond"].toString());
    player->update_licenseSecond(obj["licenseSecond"].toString());
    player->update_licenseValidSecond(obj["license_validSecond"].toBool());
    player->update_rankingSecond(obj["rankingSecond"].toString());

    return player;
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

void PlayerFilterModel::setClub(QString s)
{
    club = s.toLower();
    invalidate();
}

void PlayerFilterModel::setLicenseList(QStringList lics)
{
    licenseList = lics;
    invalidate();
}

void PlayerFilterModel::setSearchName(QString s)
{
    terms = s.toLower();
    invalidate();
}

bool PlayerFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)

    if (terms.isEmpty() && club.isEmpty() && licenseList.isEmpty())
        return true;

    PlayerModel *model = dynamic_cast<PlayerModel *>(sourceModel());
    Player *item = model->item(source_row);
    auto txt = item->get_firstName().toLower() + item->get_lastName().toLower();

    if (!licenseList.isEmpty())
    {
        if (!licenseList.contains(item->get_license()))
            return false;
    }

    if (!terms.isEmpty() && !club.isEmpty())
        return txt.contains(terms.toLower()) && item->get_club().toLower() == club.toLower();
    else if (terms.isEmpty() && !club.isEmpty())
        return item->get_club().toLower() == club.toLower();
    else if (!terms.isEmpty() && club.isEmpty())
        return txt.contains(terms.toLower());

    return true;
}

bool PlayerFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    PlayerModel *model = dynamic_cast<PlayerModel *>(sourceModel());
    Player *itemLeft = model->item(left.row());
    Player *itemRight = model->item(right.row());

    QCollator sorter;
    sorter.setNumericMode(true);

    if (model->rowCount() > 0 &&
        !model->item(0)->get_licenseSecond().isEmpty())
    {
        return sorter.compare(itemLeft->get_lastName(), itemRight->get_lastName());
    }

    switch (left.column())
    {
    default:
    case 0: return sorter.compare(itemLeft->get_license(), itemRight->get_license()) < 0;
    case 1: return sorter.compare(itemLeft->get_lastName(), itemRight->get_lastName()) < 0;
    case 2: return sorter.compare(itemLeft->get_firstName(), itemRight->get_firstName()) < 0;
    case 3: return sorter.compare(itemLeft->get_ranking(), itemRight->get_ranking()) < 0;
    case 4: return sorter.compare(itemLeft->get_club(), itemRight->get_club()) < 0;
    }
}
