#ifndef DIALOGPODIUMSINGLE_H
#define DIALOGPODIUMSINGLE_H

#include <QDialog>
#include <PlayerModel.h>

namespace Ui {
class DialogPodiumSingle;
}

class DialogPodiumSingle : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPodiumSingle(bool editable, QList<Player *> players, bool isDouble, QWidget *parent = nullptr);
    ~DialogPodiumSingle();

    QList<Player *> getWinners();

private slots:
    void on_pushButtonUp_clicked();
    void on_pushButtonDown_clicked();

private:
    Ui::DialogPodiumSingle *ui;

    QList<Player *> players;

    void recalcRank();
};

#endif // DIALOGPODIUMSINGLE_H
