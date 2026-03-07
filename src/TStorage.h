#ifndef TSTORAGE_H
#define TSTORAGE_H

#include <QObject>
#include <QSortFilterProxyModel>
#include <QAbstractListModel>
#include <QJsonObject>
#include <QFileInfo>

class Tournament;

class TStorage: public QAbstractListModel
{
    Q_OBJECT
public:
    static TStorage *Instance()
    {
        static TStorage m;
        return &m;
    }
    virtual ~TStorage() override;

    enum
    {
        RoleUuid = Qt::UserRole + 1,
        RoleName,
        RoleDate,
        RoleSerieCount,
        RoleStatus,
    };

    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QHash<int,QByteArray> roleNames() const override;

    void loadTournament(const QJsonObject &obj);
    Tournament *item(int row);
    Tournament *itemByUuid(const QString &uuid);
    int indexOfUuid(const QString &uuid) const;

    void loadFromDisk();
    void saveToDisk(int idx);
    void saveToDisk(Tournament *t);

    Tournament *createNewTournament(QString name);
    void deleteTournament(Tournament *t);

    // Backup
    static const int MAX_BACKUPS = 50;
    void createBackup(Tournament *t);
    QList<QFileInfo> listBackups(const QString &uuid) const;
    bool restoreBackup(const QString &uuid, int backupIndex);

    // Import
    Tournament *importTournament(const QJsonObject &obj);

private:
    TStorage();

    QStringList headers;
    QVector<Tournament *> tournaments;
};

#endif // TSTORAGE_H
