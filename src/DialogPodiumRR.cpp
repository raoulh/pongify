#include "DialogPodiumRR.h"
#include "ui_DialogPodiumRR.h"

DialogPodiumRR::DialogPodiumRR(bool editable, QList<ScoreRR> _playerScores, bool isDouble, bool isFinished, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPodiumRR),
    playerScores(_playerScores)
{
    ui->setupUi(this);

    if (editable)
    {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
        ui->labelHelp->setText("Vous pouvez modifier le classement dans le podium avant de valider. Une fois le podium validé, plus aucune modification n'est possible.");
        ui->pushButtonDown->show();
        ui->pushButtonUp->show();
    }
    else
    {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
        if (isFinished)
            ui->labelHelp->setText("Ceci est le résultat définitif");
        else
            ui->labelHelp->setText("Ceci est le résultat provisoire");
        ui->pushButtonDown->hide();
        ui->pushButtonUp->hide();
    }

    ui->treeWidget->setHeaderLabels(QStringList()
                                        << "#"
                                        << (isDouble? "Joueurs": "Joueur")
                                        << "Score"
                                        << "Matchs gagnés"
                                        << "Set gagnés"
                                        << "Set perdus");


    for (int i = 0;i < playerScores.count();i++)
    {
        const auto &p = playerScores.at(i);

        auto it = new QTreeWidgetItem(ui->treeWidget);
        it->setText(0, QString::number(i + 1));
        if (isDouble)
            it->setText(1, QStringLiteral("%1 %2 / %1 %2").arg(p.player->get_firstName(), p.player->get_lastName(), p.player->get_firstNameSecond(), p.player->get_lastNameSecond()));
        else
            it->setText(1, QStringLiteral("%1 %2 [%3]").arg(p.player->get_firstName(), p.player->get_lastName(), p.player->get_ranking()));
        it->setText(2, QString::number(p.score));
        it->setText(3, QString::number(p.winCount));
        it->setText(4, QString::number(p.setWin));
        it->setText(5, QString::number(p.setLoose));
    }

    for (int i = 0;i < 6;i++) ui->treeWidget->resizeColumnToContents(i);
}

DialogPodiumRR::~DialogPodiumRR()
{
    delete ui;
}

QList<ScoreRR> DialogPodiumRR::getWinners()
{
    return playerScores;
}

void DialogPodiumRR::on_pushButtonDown_clicked()
{
    auto item = ui->treeWidget->currentItem();
    auto row = ui->treeWidget->currentIndex().row();

    if (item && row < ui->treeWidget->topLevelItemCount() - 1)
    {
        ui->treeWidget->takeTopLevelItem(row);
        ui->treeWidget->insertTopLevelItem(row + 1, item);
        ui->treeWidget->setCurrentItem(item);
        playerScores.swapItemsAt(row, row + 1);
        recalcRank();
    }
}

void DialogPodiumRR::on_pushButtonUp_clicked()
{
    auto item = ui->treeWidget->currentItem();
    auto row = ui->treeWidget->currentIndex().row();

    if (item && row > 0)
    {
        ui->treeWidget->takeTopLevelItem(row);
        ui->treeWidget->insertTopLevelItem(row - 1, item);
        ui->treeWidget->setCurrentItem(item);
        playerScores.swapItemsAt(row, row - 1);
        recalcRank();
    }
}

void DialogPodiumRR::recalcRank()
{
    for (int i = 0;i < ui->treeWidget->topLevelItemCount();i++)
    {
        auto it = ui->treeWidget->topLevelItem(i);
        it->setText(0, QString::number(i + 1));
    }
}
