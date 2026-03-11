#ifndef DIALOGCREATEPOOLS_H
#define DIALOGCREATEPOOLS_H

#include <QDialog>

namespace Ui {
class DialogCreatePools;
}

class Tournament;
class Player;

class DialogCreatePools : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCreatePools(Tournament *tournament, QWidget *parent = nullptr);
    ~DialogCreatePools();

    // Returns true if pools were created
    bool poolsCreated() const { return created; }

private slots:
    void on_buttonBox_accepted();
    void updatePreview();

private:
    Ui::DialogCreatePools *ui;
    Tournament *tournament = nullptr;
    bool created = false;

    QVector<QVector<Player *>> distributePlayersToPool(int poolCount, bool serpentine);
    QVector<Player *> sortedPlayers();
};

#endif // DIALOGCREATEPOOLS_H
