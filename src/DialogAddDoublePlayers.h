#ifndef DIALOGADDDOUBLEPLAYERS_H
#define DIALOGADDDOUBLEPLAYERS_H

#include <QDialog>

class Player;

namespace Ui {
class DialogAddDoublePlayers;
}

class DialogAddDoublePlayers : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddDoublePlayers(QWidget *parent = nullptr);
    ~DialogAddDoublePlayers();

    Player *getPlayer1() { return player1; }
    Player *getPlayer2() { return player2; }

private slots:
    void on_buttonBox_accepted();
    void on_pushButtonChoose1_clicked();
    void on_pushButtonChoose2_clicked();

private:
    Ui::DialogAddDoublePlayers *ui;

    Player *player1 = nullptr;
    Player *player2 = nullptr;
};

#endif // DIALOGADDDOUBLEPLAYERS_H
