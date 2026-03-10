#include "DialogPlayers.h"
#include "ui_DialogPlayers.h"
#include "PlayerModel.h"
#include "DialogAddPlayer.h"

DialogPlayers::DialogPlayers(PlayerModel *m, bool isAddDialog, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPlayers),
    playerModel(m)
{
    filterModel = new PlayerFilterModel(this);
    filterModel->setSourceModel(m);

    ui->setupUi(this);
    ui->treeView->setModel(filterModel);
    ui->treeView->setUniformRowHeights(true);

    ui->comboBoxSearch->addItem("Tous les clubs");
    QStringList clubList = playerModel->getClubs();
    clubList.sort(Qt::CaseInsensitive);
    ui->comboBoxSearch->addItems(clubList);

    connect(ui->comboBoxSearch, &QComboBox::currentIndexChanged, this, [this](int index)
    {
        QString c;
        if (index > 0)
            c = ui->comboBoxSearch->currentText();
        filterModel->setClub(c);
    });

    connect(ui->lineEditSearch, &QLineEdit::textEdited, this, [this](auto txt)
    {
        filterModel->setSearchName(txt);
    });

    if (isAddDialog)
    {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
        ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

        connect(ui->treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex &)
        {
            accept();
        });
    }
    else
    {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
}

DialogPlayers::~DialogPlayers()
{
    delete ui;
}

Player *DialogPlayers::getSelected()
{
    auto indexes = ui->treeView->selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
        return nullptr;
    auto idx = filterModel->indexToSource(indexes.at(0).row());

    return playerModel->item(idx);
}

QList<Player *> DialogPlayers::getSelectedList()
{
    QList<Player *> result;
    QSet<int> seen;
    auto indexes = ui->treeView->selectionModel()->selectedIndexes();
    for (const auto &index : indexes)
    {
        int sourceRow = filterModel->indexToSource(index.row());
        if (seen.contains(sourceRow))
            continue;
        seen.insert(sourceRow);
        auto p = playerModel->item(sourceRow);
        if (p)
            result.append(p);
    }
    return result;
}

void DialogPlayers::on_pushButtonAddManual_clicked()
{
    DialogAddPlayer d;
    if (d.exec() == QDialog::Accepted)
    {
        playerModel->loadPlayer(d.getPlayerJson());
        playerModel->saveCache();
    }
}
