#include "TStorage.h"
#include "Tournament.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include "Utils.h"

TStorage::TStorage():
    QAbstractListModel{}
{
    headers = {{ "Nom" },
               { "Date" },
               { "Statut" }};
}

TStorage::~TStorage()
{
    qDeleteAll(tournaments);
}

QVariant TStorage::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return tournaments.at(index.row())->get_name();
        case 1: return tournaments.at(index.row())->get_date();
        case 2: return tournaments.at(index.row())->get_status();
        default: break;
        }

        break;
    case RoleUuid: return tournaments.at(index.row())->get_uuid();
    case RoleName: return tournaments.at(index.row())->get_name();
    case RoleDate: return tournaments.at(index.row())->get_date().toString("dd/MM/yyyy");
    case RoleStatus: return tournaments.at(index.row())->get_status();
    default: break;
    }

    return QVariant();
}

QVariant TStorage::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if (section >= 0 && section < headers.size())
            return headers.at(section);
    }

    return QVariant();
}

int TStorage::rowCount(const QModelIndex &) const
{
    return tournaments.count();
}

int TStorage::columnCount(const QModelIndex &) const
{
    return 3;
}

QHash<int, QByteArray> TStorage::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[RoleName] = "name";
    roles[RoleDate] = "date";
    roles[RoleStatus] = "status";
    roles[RoleUuid] = "uuid";
    return roles;
}

void TStorage::loadTournament(const QJsonObject &obj)
{
    auto tournament = Tournament::fromJson(obj);

    if (!tournament)
    {
        qWarning() << "Bad json for tournament: one of the required param is empty";
        return;
    }

    tournaments.append(tournament);
}

Tournament *TStorage::item(int row)
{
    if (row >= 0 && row < tournaments.count())
        return tournaments.at(row);
    return nullptr;
}

Tournament *TStorage::itemByUuid(const QString &uuid)
{
    for (auto *t : tournaments)
    {
        if (t->get_uuid() == uuid)
            return t;
    }
    return nullptr;
}

int TStorage::indexOfUuid(const QString &uuid) const
{
    for (int i = 0; i < tournaments.size(); i++)
    {
        if (tournaments.at(i)->get_uuid() == uuid)
            return i;
    }
    return -1;
}

void TStorage::loadFromDisk()
{
    beginResetModel();
    qDeleteAll(tournaments);
    tournaments.clear();

    QDir dir(QStringLiteral("%1/tournaments").arg(Utils::getCachePath()));
    auto lst = dir.entryInfoList({{"*.tnm"}}, QDir::Files);

    for (const auto &finfo: lst)
    {
        if (!finfo.isFile()) continue;

        QFile f(finfo.absoluteFilePath());
        if (!f.open(QFile::ReadOnly))
            continue;

        QJsonParseError jerr;
        QJsonDocument jdoc = QJsonDocument::fromJson(f.readAll(), &jerr);
        if (jerr.error != QJsonParseError::NoError)
        {
            qWarning() << "JSON parse error " << jerr.errorString();
            continue;
        }

        if (!jdoc.isObject())
        {
            qWarning() << "JSON is not an object";
            continue;
        }

        loadTournament(jdoc.object());
    }

    endResetModel();
}

void TStorage::saveToDisk(int idx)
{
    auto t = item(idx);
    if (t) saveToDisk(t);
}

void TStorage::saveToDisk(Tournament *t)
{
    createBackup(t);

    QFile f(QStringLiteral("%1/tournaments/t_%2.tnm").arg(Utils::getCachePath(), t->get_uuid()));
    if (!f.open(QFile::ReadWrite | QFile::Truncate))
    {
        qWarning() << "unable to open file " << f.fileName();
        return;
    }

    QJsonDocument jdoc;
    jdoc.setObject(t->toJson());

    f.write(jdoc.toJson());
    f.close();
}

Tournament *TStorage::createNewTournament(QString name)
{
    auto t = new Tournament();
    t->update_name(name);
    t->update_date(QDateTime::currentDateTime());
    t->update_status("open");
    t->update_uuid(QUuid::createUuid().toString(QUuid::WithoutBraces));

    beginInsertRows({}, tournaments.count(), tournaments.count());
    tournaments.append(t);
    endInsertRows();
    saveToDisk(t);

    return t;
}

void TStorage::deleteTournament(Tournament *t)
{
    QString f(QStringLiteral("%1/tournaments/t_%2.tnm").arg(Utils::getCachePath(), t->get_uuid()));
    QFile::remove(f);

    // Remove associated backups
    QString backupDir = QStringLiteral("%1/tournaments/backups").arg(Utils::getCachePath());
    QDir bdir(backupDir);
    auto backupFiles = bdir.entryInfoList({QStringLiteral("t_%1_bak_*.tnm").arg(t->get_uuid())}, QDir::Files);
    for (const auto &bf : backupFiles)
        QFile::remove(bf.absoluteFilePath());

    int idx = -1;
    for (int i = 0;i < tournaments.size();i++)
    {
        if (t->get_uuid() == tournaments.at(i)->get_uuid())
        {
            idx = i;
            break;
        }
    }

    if (idx >= 0)
    {
        beginRemoveRows({}, idx, idx);
        tournaments.removeAt(idx);
        endRemoveRows();
    }

    delete t;
}

void TStorage::createBackup(Tournament *t)
{
    QString tournamentsDir = QStringLiteral("%1/tournaments").arg(Utils::getCachePath());
    QString sourceFile = QStringLiteral("%1/t_%2.tnm").arg(tournamentsDir, t->get_uuid());

    if (!QFile::exists(sourceFile))
        return;

    QString backupDir = QStringLiteral("%1/backups").arg(tournamentsDir);
    QDir().mkpath(backupDir);

    // Rotate existing backups: delete oldest, shift others
    QString backupPath = QStringLiteral("%1/t_%2_bak_%3.tnm");
    QFile::remove(backupPath.arg(backupDir, t->get_uuid(), QString::number(MAX_BACKUPS - 1)));

    for (int i = MAX_BACKUPS - 2; i >= 0; i--)
    {
        QString src = backupPath.arg(backupDir, t->get_uuid(), QString::number(i));
        QString dst = backupPath.arg(backupDir, t->get_uuid(), QString::number(i + 1));
        if (QFile::exists(src))
            QFile::rename(src, dst);
    }

    // Copy current file as backup 0
    QFile::copy(sourceFile, backupPath.arg(backupDir, t->get_uuid(), QStringLiteral("0")));
}

QList<QFileInfo> TStorage::listBackups(const QString &uuid) const
{
    QList<QFileInfo> result;
    QString backupDir = QStringLiteral("%1/tournaments/backups").arg(Utils::getCachePath());
    QDir dir(backupDir);

    if (!dir.exists())
        return result;

    for (int i = 0; i < MAX_BACKUPS; i++)
    {
        QString fname = QStringLiteral("t_%1_bak_%2.tnm").arg(uuid, QString::number(i));
        QFileInfo fi(dir.absoluteFilePath(fname));
        if (fi.exists())
            result.append(fi);
    }

    return result;
}

bool TStorage::restoreBackup(const QString &uuid, int backupIndex)
{
    QString backupDir = QStringLiteral("%1/tournaments/backups").arg(Utils::getCachePath());
    QString backupFile = QStringLiteral("%1/t_%2_bak_%3.tnm").arg(backupDir, uuid, QString::number(backupIndex));

    QFile f(backupFile);
    if (!f.open(QFile::ReadOnly))
        return false;

    QJsonParseError jerr;
    QJsonDocument jdoc = QJsonDocument::fromJson(f.readAll(), &jerr);
    f.close();

    if (jerr.error != QJsonParseError::NoError || !jdoc.isObject())
        return false;

    auto *restored = Tournament::fromJson(jdoc.object());
    if (!restored)
        return false;

    // Backup the current file before restoring
    auto *current = itemByUuid(uuid);
    if (current)
        createBackup(current);

    // Replace in-memory tournament
    int idx = indexOfUuid(uuid);
    if (idx >= 0)
    {
        delete tournaments[idx];
        tournaments[idx] = restored;
        emit dataChanged(index(idx), index(idx));
    }
    else
    {
        beginInsertRows({}, tournaments.count(), tournaments.count());
        tournaments.append(restored);
        endInsertRows();
    }

    // Write restored data to main file
    QString mainFile = QStringLiteral("%1/tournaments/t_%2.tnm").arg(Utils::getCachePath(), uuid);
    QFile out(mainFile);
    if (!out.open(QFile::ReadWrite | QFile::Truncate))
    {
        delete restored;
        return false;
    }

    QJsonDocument outDoc;
    outDoc.setObject(restored->toJson());
    out.write(outDoc.toJson());
    out.close();

    return true;
}

Tournament *TStorage::importTournament(const QJsonObject &obj)
{
    auto *imported = Tournament::fromJson(obj);
    if (!imported)
        return nullptr;

    int idx = indexOfUuid(imported->get_uuid());
    if (idx >= 0)
    {
        // Replace existing tournament
        createBackup(tournaments[idx]);
        delete tournaments[idx];
        tournaments[idx] = imported;
        emit dataChanged(index(idx), index(idx));
    }
    else
    {
        // Add new tournament
        beginInsertRows({}, tournaments.count(), tournaments.count());
        tournaments.append(imported);
        endInsertRows();
    }

    saveToDisk(imported);
    return imported;
}
