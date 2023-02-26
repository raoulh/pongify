#include "DialogEditScore.h"
#include "ui_DialogEditScore.h"
#include "TSerie.h"

DialogEditScore::DialogEditScore(TMatch *match, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditScore)
{
    ui->setupUi(this);

    auto p = match->get_player1();
    if (p)
        ui->labelPlayer1->setText(QStringLiteral("%1 %2").arg(p->get_firstName(), p->get_lastName()));
    else
        ui->labelPlayer1->setText("<Aucun joueur>");

    p = match->get_player2();
    if (p)
        ui->labelPlayer2->setText(QStringLiteral("%1 %2").arg(p->get_firstName(), p->get_lastName()));
    else
        ui->labelPlayer2->setText("<Aucun joueur>");

    ui->spinBoxPlayer1->setMinimum(0);
    ui->spinBoxPlayer1->setMaximum(5);
    ui->spinBoxPlayer1->setValue(match->get_playerScore1());

    ui->spinBoxPlayer2->setMinimum(0);
    ui->spinBoxPlayer2->setMaximum(5);
    ui->spinBoxPlayer2->setValue(match->get_playerScore2());
}

DialogEditScore::~DialogEditScore()
{
    delete ui;
}

int DialogEditScore::getScorePlayer1()
{
    return ui->spinBoxPlayer1->value();
}

int DialogEditScore::getScorePlayer2()
{
    return ui->spinBoxPlayer2->value();
}
