#include "DialogPlayerList.h"
#include "ui_DialogPlayerList.h"
#include "TSerie.h"
#include "PlayerModel.h"
#include "DialogPlayers.h"

#include <QMessageBox>

DialogPlayerList::DialogPlayerList(TSerie *s, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPlayerList),
    serie(s)
{
    ui->setupUi(this);

    ui->labelSerie->setText(QStringLiteral("Série: %1").arg(serie->get_name()));

    playerModel = PlayerModel::createEmpty();
    for (int i = 0;i < serie->getPlayerModel()->rowCount();i++)
    {
        playerModel->appendClone(serie->getPlayerModel()->item(i));
    }

    filterModel = new PlayerFilterModel(this);
    filterModel->setSourceModel(playerModel);

    ui->treeView->setModel(filterModel);
    ui->treeView->setUniformRowHeights(true);
}

DialogPlayerList::~DialogPlayerList()
{
    delete ui;
}

void DialogPlayerList::on_pushButtonAdd_clicked()
{
    DialogPlayers d(PlayerModel::Instance());
    if (d.exec() == QDialog::Accepted)
    {
        auto p = d.getSelected();
        if (p && checkRanking(p->get_ranking()))
        {
            qDebug() << "add player: " << p->get_firstName() << " " << p->get_lastName() << " " << p->get_club();
            playerModel->appendClone(p);
        }
    }
}

void DialogPlayerList::on_pushButtonRemove_clicked()
{
    auto indexes = ui->treeView->selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
        return;
    auto idx = filterModel->indexToSource(indexes.at(0).row());

    auto ret = QMessageBox::question(this, "Supprimer", "Supprimer le joueur sélectionné ?");
    if (ret != QMessageBox::Yes)
        return;

    playerModel->removeRow(idx);
}

void DialogPlayerList::on_buttonBox_accepted()
{
    //save current player list in serie
    serie->getPlayerModel()->clear();
    for (int i = 0;i < playerModel->rowCount();i++)
    {
        serie->getPlayerModel()->appendClone(playerModel->item(i));
    }

    accept();
}

bool DialogPlayerList::checkRanking(QString rank)
{
    QString maxRank = serie->get_ranking();

    if (maxRank.toLower() == "open")
        return true; //always accept all players for open series

    int max, player_rank;
    if (maxRank.toLower() == "nc")
        max = 100;
    else
    {
        bool v;
        max = maxRank.toInt(&v);
        if (!v)
        {
            qDebug() << "Failed to convert ranking: " << maxRank;
            return true;
        }
    }

    if (rank.toLower() == "nc")
        player_rank = 100;
    else
    {
        bool v;
        player_rank = rank.toInt(&v);
        if (!v)
        {
            qDebug() << "Failed to convert ranking: " << rank;
            return true;
        }
    }

    return player_rank >= max;
}
