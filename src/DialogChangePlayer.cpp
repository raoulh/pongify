#include "DialogChangePlayer.h"
#include "ui_DialogChangePlayer.h"
#include "PlayerModel.h"
#include "TSerie.h"
#include <QStyledItemDelegate>

DialogChangePlayer::DialogChangePlayer(PlayerModel *m, TMatch *ma, bool isDouble, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogChangePlayer),
    model(m),
    match(ma)
{
    ui->setupUi(this);

    for (int i = 0;i < model->rowCount();i++)
    {
        Player *p = model->item(i);
        QString entry = QStringLiteral("%1 %2 [%3] (%4)")
                        .arg(p->get_firstName(), p->get_lastName(), p->get_ranking(), p->get_club());
        ui->comboBoxP1->addItem(entry);
        ui->comboBoxP1_2->addItem(entry);
        ui->comboBoxP2->addItem(entry);
        ui->comboBoxP2_2->addItem(entry);
    }

    ui->comboBoxP1->setCurrentIndex(model->indexFromPlayer(match->get_player1()));
    ui->comboBoxP2->setCurrentIndex(model->indexFromPlayer(match->get_player2()));

    ui->comboBoxP1_2->setCurrentIndex(model->indexFromPlayer(match->get_playerSecond1()));
    ui->comboBoxP2_2->setCurrentIndex(model->indexFromPlayer(match->get_playerSecond2()));

    if (!isDouble)
    {
        ui->comboBoxP1_2->hide();
        ui->comboBoxP2_2->hide();
    }
}

DialogChangePlayer::~DialogChangePlayer()
{
    delete ui;
}

Player *DialogChangePlayer::getPlayer1_1()
{
    if (ui->checkBoxBye1->isChecked())
        return nullptr;
    return model->item(ui->comboBoxP1->currentIndex());
}

Player *DialogChangePlayer::getPlayer1_2()
{
    if (ui->checkBoxBye1->isChecked())
        return nullptr;
    return model->item(ui->comboBoxP1_2->currentIndex());
}

Player *DialogChangePlayer::getPlayer2_1()
{
    if (ui->checkBoxBye2->isChecked())
        return nullptr;
    return model->item(ui->comboBoxP2->currentIndex());
}

Player *DialogChangePlayer::getPlayer2_2()
{
    if (ui->checkBoxBye2->isChecked())
        return nullptr;
    return model->item(ui->comboBoxP2_2->currentIndex());
}
