#include "DialogAddPlayer.h"
#include "ui_DialogAddPlayer.h"
#include "PlayerModel.h"

DialogAddPlayer::DialogAddPlayer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddPlayer)
{
    ui->setupUi(this);

    ui->comboBoxClub->addItems(PlayerModel::Instance()->getClubs());

    ui->comboBoxRank->addItem("NC");
    for (int i = 90; i > 0;i -= 5)
        ui->comboBoxRank->addItem(QString::number(i));
}

DialogAddPlayer::~DialogAddPlayer()
{
    delete ui;
}

QJsonObject DialogAddPlayer::getPlayerJson()
{
    return {
        { "firstname", ui->lineEditFirstName->text() },
        { "lastname", ui->lineEditLastName->text() },
        { "license", ui->lineEditLic->text() },
        { "ranking", ui->comboBoxRank->currentText() },
        { "club", ui->comboBoxClub->currentText() },
        { "license_valid", true },
    };
}
