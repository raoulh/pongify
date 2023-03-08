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
        QString entry;
        if (!isDouble)
        {
            entry = QStringLiteral("%1 %2 [%3] (%4)")
                    .arg(p->get_firstName(), p->get_lastName(), p->get_ranking(), p->get_club());
        }
        else
        {
            entry = QStringLiteral("%1 %2 [%3] (%4) - %5 %6 [%7] (%8)")
                    .arg(p->get_firstName(), p->get_lastName(), p->get_ranking(), p->get_club(),
                         p->get_firstNameSecond(), p->get_lastNameSecond(), p->get_rankingSecond(), p->get_clubSecond());
        }
        ui->comboBoxP1->addItem(entry);
        ui->comboBoxP2->addItem(entry);
    }

    ui->comboBoxP1->setCurrentIndex(model->indexFromPlayer(match->get_player1()));
    ui->comboBoxP2->setCurrentIndex(model->indexFromPlayer(match->get_player2()));
}

DialogChangePlayer::~DialogChangePlayer()
{
    delete ui;
}

Player *DialogChangePlayer::getPlayer1()
{
    if (ui->checkBoxBye1->isChecked())
        return nullptr;
    return model->item(ui->comboBoxP1->currentIndex());
}

Player *DialogChangePlayer::getPlayer2()
{
    if (ui->checkBoxBye2->isChecked())
        return nullptr;
    return model->item(ui->comboBoxP2->currentIndex());
}
