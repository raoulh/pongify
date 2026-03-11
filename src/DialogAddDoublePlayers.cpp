#include "DialogAddDoublePlayers.h"
#include "ui_DialogAddDoublePlayers.h"
#include "DialogPlayers.h"
#include "PlayerModel.h"

#include <QMessageBox>

DialogAddDoublePlayers::DialogAddDoublePlayers(PlayerModel *source, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddDoublePlayers),
    sourceModel(source)
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
    DialogPlayers d(sourceModel ? sourceModel : PlayerModel::Instance(), true);
    if (d.exec() == QDialog::Accepted)
    {
        auto selected = d.getSelectedList();
        if (selected.count() > 2)
        {
            QMessageBox::warning(this, "Double", "Veuillez sélectionner 1 ou 2 joueurs maximum.");
            return;
        }

        if (selected.count() >= 1 && selected.at(0))
        {
            player1 = selected.at(0);
            ui->lineEditFirstname1->setText(player1->get_firstName());
            ui->lineEditLastname1->setText(player1->get_lastName());
            ui->lineEditClub1->setText(player1->get_club());
            ui->lineEditLicense1->setText(player1->get_license());
            ui->lineEditRank1->setText(player1->get_ranking());
        }

        if (selected.count() == 2 && selected.at(1))
        {
            player2 = selected.at(1);
            ui->lineEditFirstname2->setText(player2->get_firstName());
            ui->lineEditLastname2->setText(player2->get_lastName());
            ui->lineEditClub2->setText(player2->get_club());
            ui->lineEditLicense2->setText(player2->get_license());
            ui->lineEditRank2->setText(player2->get_ranking());
        }
    }
}

void DialogAddDoublePlayers::on_pushButtonChoose2_clicked()
{
    DialogPlayers d(sourceModel ? sourceModel : PlayerModel::Instance(), true);
    if (d.exec() == QDialog::Accepted)
    {
        auto selected = d.getSelectedList();
        if (selected.count() > 2)
        {
            QMessageBox::warning(this, "Double", "Veuillez sélectionner 1 ou 2 joueurs maximum.");
            return;
        }

        if (selected.count() == 1 && selected.at(0))
        {
            player2 = selected.at(0);
            ui->lineEditFirstname2->setText(player2->get_firstName());
            ui->lineEditLastname2->setText(player2->get_lastName());
            ui->lineEditClub2->setText(player2->get_club());
            ui->lineEditLicense2->setText(player2->get_license());
            ui->lineEditRank2->setText(player2->get_ranking());
        }

        if (selected.count() == 2)
        {
            if (selected.at(0))
            {
                player1 = selected.at(0);
                ui->lineEditFirstname1->setText(player1->get_firstName());
                ui->lineEditLastname1->setText(player1->get_lastName());
                ui->lineEditClub1->setText(player1->get_club());
                ui->lineEditLicense1->setText(player1->get_license());
                ui->lineEditRank1->setText(player1->get_ranking());
            }
            if (selected.at(1))
            {
                player2 = selected.at(1);
                ui->lineEditFirstname2->setText(player2->get_firstName());
                ui->lineEditLastname2->setText(player2->get_lastName());
                ui->lineEditClub2->setText(player2->get_club());
                ui->lineEditLicense2->setText(player2->get_license());
                ui->lineEditRank2->setText(player2->get_ranking());
            }
        }
    }
}
