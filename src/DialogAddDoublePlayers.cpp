#include "DialogAddDoublePlayers.h"
#include "ui_DialogAddDoublePlayers.h"
#include "DialogPlayers.h"
#include "PlayerModel.h"

DialogAddDoublePlayers::DialogAddDoublePlayers(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddDoublePlayers)
{
    ui->setupUi(this);

    ui->lineEditFirstname1->setDisabled(true);
    ui->lineEditLastname1->setDisabled(true);
    ui->lineEditClub1->setDisabled(true);
    ui->lineEditLicense1->setDisabled(true);
    ui->lineEditRank1->setDisabled(true);

    ui->lineEditFirstname2->setDisabled(true);
    ui->lineEditLastname2->setDisabled(true);
    ui->lineEditClub2->setDisabled(true);
    ui->lineEditLicense2->setDisabled(true);
    ui->lineEditRank2->setDisabled(true);
}

DialogAddDoublePlayers::~DialogAddDoublePlayers()
{
    delete ui;
}

void DialogAddDoublePlayers::on_buttonBox_accepted()
{
    if (player1 && player2)
        accept();
}

void DialogAddDoublePlayers::on_pushButtonChoose1_clicked()
{
    DialogPlayers d(PlayerModel::Instance(), true);
    if (d.exec() == QDialog::Accepted)
    {
        player1 = d.getSelected();
        ui->lineEditFirstname1->setText(player1->get_firstName());
        ui->lineEditLastname1->setText(player1->get_lastName());
        ui->lineEditClub1->setText(player1->get_club());
        ui->lineEditLicense1->setText(player1->get_license());
        ui->lineEditRank1->setText(player1->get_ranking());
    }
}

void DialogAddDoublePlayers::on_pushButtonChoose2_clicked()
{
    DialogPlayers d(PlayerModel::Instance(), true);
    if (d.exec() == QDialog::Accepted)
    {
        player2 = d.getSelected();
        ui->lineEditFirstname2->setText(player2->get_firstName());
        ui->lineEditLastname2->setText(player2->get_lastName());
        ui->lineEditClub2->setText(player2->get_club());
        ui->lineEditLicense2->setText(player2->get_license());
        ui->lineEditRank2->setText(player2->get_ranking());
    }
}
