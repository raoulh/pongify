#ifndef DIALOGEDITSCORE_H
#define DIALOGEDITSCORE_H

#include <QDialog>

class TMatch;

namespace Ui {
class DialogEditScore;
}

class DialogEditScore : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditScore(TMatch *match, QWidget *parent = nullptr);
    ~DialogEditScore();

    int getScorePlayer1();
    int getScorePlayer2();

private:
    Ui::DialogEditScore *ui;
};

#endif // DIALOGEDITSCORE_H
