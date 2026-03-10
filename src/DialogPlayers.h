#ifndef DIALOGPLAYERS_H
#define DIALOGPLAYERS_H

#include <QDialog>

class Player;
class PlayerModel;
class PlayerFilterModel;

namespace Ui {
class DialogPlayers;
}

class DialogPlayers : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPlayers(PlayerModel *m, bool isAddDialog, QWidget *parent = nullptr);
    ~DialogPlayers();

    Player *getSelected();
    QList<Player *> getSelectedList();

private slots:
    void on_pushButtonAddManual_clicked();

private:
    Ui::DialogPlayers *ui;
    PlayerModel *playerModel = nullptr;
    PlayerFilterModel *filterModel = nullptr;
};

#endif // DIALOGPLAYERS_H
