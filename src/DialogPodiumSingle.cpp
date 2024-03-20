#include "DialogPodiumSingle.h"
#include "ui_DialogPodiumSingle.h"

DialogPodiumSingle::DialogPodiumSingle(bool editable, QList<Player *> _players, bool isDouble, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPodiumSingle),
    players(_players)
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
        ui->labelHelp->setText("Ceci est le résultat définitif");
        ui->pushButtonDown->hide();
        ui->pushButtonUp->hide();
    }

    ui->treeWidget->setHeaderLabels(QStringList()
                                        << "#"
                                        << (isDouble? "Joueurs": "Joueur"));


    for (int i = 0;i < players.count();i++)
    {
        const auto &p = players.at(i);

        auto it = new QTreeWidgetItem(ui->treeWidget);
        it->setText(0, QString::number(i + 1));
        if (isDouble)
            it->setText(1, QStringLiteral("%1 %2 / %3 %4").arg(p->get_firstName(), p->get_lastName(), p->get_firstNameSecond(), p->get_lastNameSecond()));
        else
            it->setText(1, QStringLiteral("%1 %2 [%3]").arg(p->get_firstName(), p->get_lastName(), p->get_ranking()));
    }

    for (int i = 0;i < 6;i++) ui->treeWidget->resizeColumnToContents(i);
}

DialogPodiumSingle::~DialogPodiumSingle()
{
    delete ui;
}

QList<Player *> DialogPodiumSingle::getWinners()
{
    return players;
}

void DialogPodiumSingle::on_pushButtonUp_clicked()
{
    auto item = ui->treeWidget->currentItem();
    auto row = ui->treeWidget->currentIndex().row();

    if (item && row > 0)
    {
        ui->treeWidget->takeTopLevelItem(row);
        ui->treeWidget->insertTopLevelItem(row - 1, item);
        ui->treeWidget->setCurrentItem(item);
        players.swapItemsAt(row, row - 1);
        recalcRank();
    }
}

void DialogPodiumSingle::on_pushButtonDown_clicked()
{
    auto item = ui->treeWidget->currentItem();
    auto row = ui->treeWidget->currentIndex().row();

    if (item && row < ui->treeWidget->topLevelItemCount() - 1)
    {
        ui->treeWidget->takeTopLevelItem(row);
        ui->treeWidget->insertTopLevelItem(row + 1, item);
        ui->treeWidget->setCurrentItem(item);
        players.swapItemsAt(row, row + 1);
        recalcRank();
    }
}

void DialogPodiumSingle::recalcRank()
{
    for (int i = 0;i < ui->treeWidget->topLevelItemCount();i++)
    {
        auto it = ui->treeWidget->topLevelItem(i);
        it->setText(0, QString::number(i + 1));
    }
}
