#ifndef DIALOGCHANGEPLAYER_H
#define DIALOGCHANGEPLAYER_H

#include <QDialog>

class Player;
class PlayerModel;
class TMatch;

namespace Ui {
class DialogChangePlayer;
}

class DialogChangePlayer : public QDialog
{
    Q_OBJECT

public:
    explicit DialogChangePlayer(PlayerModel *m, TMatch *ma, bool isDouble, QWidget *parent = nullptr);
    ~DialogChangePlayer();

    Player *getPlayer1();
    Player *getPlayer2();

private:
    Ui::DialogChangePlayer *ui;

    PlayerModel *model = nullptr;
    TMatch *match = nullptr;
};

#endif // DIALOGCHANGEPLAYER_H
