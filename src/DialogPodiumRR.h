#ifndef DIALOGPODIUMRR_H
#define DIALOGPODIUMRR_H

#include <QDialog>
#include <PlayerModel.h>
#include "Score.h"

namespace Ui {
class DialogPodiumRR;
}

class DialogPodiumRR : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPodiumRR(bool editable, QList<ScoreRR> playerScores, bool isDouble, bool isFinished, QWidget *parent = nullptr);
    ~DialogPodiumRR();

    QList<ScoreRR> getWinners();

private slots:
    void on_pushButtonDown_clicked();
    void on_pushButtonUp_clicked();

private:
    Ui::DialogPodiumRR *ui;

    QList<ScoreRR> playerScores;

    void recalcRank();
};

#endif // DIALOGPODIUMRR_H
