#ifndef TSTORAGE_H
#define TSTORAGE_H

#include <QObject>
#include <QSortFilterProxyModel>
#include <QAbstractListModel>
#include <QJsonObject>

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

    void loadFromDisk();
    void saveToDisk(int idx);
    void saveToDisk(Tournament *t);

    Tournament *createNewTournament(QString name);
    void deleteTournament(Tournament *t);

private:
    TStorage();

    QStringList headers;
    QVector<Tournament *> tournaments;
};

#endif // TSTORAGE_H
