#include "DialogPlayers.h"
#include "ui_DialogPlayers.h"
#include "PlayerModel.h"

DialogPlayers::DialogPlayers(PlayerModel *m, QWidget *parent) :
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
    ui->comboBoxSearch->addItems(playerModel->getClubs());

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
