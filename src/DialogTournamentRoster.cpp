#include "DialogTournamentRoster.h"
#include "ui_DialogTournamentRoster.h"
#include "Tournament.h"
#include "PlayerModel.h"
#include "DialogPlayers.h"
#include "DialogAddPlayer.h"
#include "DialogPlayerDispatch.h"
#include "DialogPlayersHtml.h"

#include <QMessageBox>

DialogTournamentRoster::DialogTournamentRoster(Tournament *t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTournamentRoster),
    tournament(t)
{
    ui->setupUi(this);

    // Create a working copy of the tournament roster
    playerModel = PlayerModel::createEmpty();
    for (int i = 0; i < tournament->getTournamentPlayerModel()->rowCount(); i++)
    {
        playerModel->appendClone(tournament->getTournamentPlayerModel()->item(i));
    }

    filterModel = new PlayerFilterModel(this);
    filterModel->setSourceModel(playerModel);

    ui->treeView->setModel(filterModel);
    ui->treeView->setUniformRowHeights(true);

    connect(playerModel, &PlayerModel::playersChanged, this, [this]()
    {
        ui->labelPlayerCount->setText(QStringLiteral("%1 joueurs").arg(playerModel->rowCount()));
    });
    ui->labelPlayerCount->setText(QStringLiteral("%1 joueurs").arg(playerModel->rowCount()));
}

DialogTournamentRoster::~DialogTournamentRoster()
{
    delete playerModel;
    delete ui;
}

void DialogTournamentRoster::on_pushButtonAdd_clicked()
{
    DialogPlayers d(PlayerModel::Instance(), true);
    if (d.exec() == QDialog::Accepted)
    {
        auto selected = d.getSelectedList();
        for (auto p : selected)
        {
            if (p)
            {
                // Check if player is already in roster (by license)
                if (!playerModel->getFromLicense(p->get_license()))
                    playerModel->appendClone(p);
            }
        }
    }
}

void DialogTournamentRoster::on_pushButtonAddManual_clicked()
{
    DialogAddPlayer d;
    if (d.exec() == QDialog::Accepted)
    {
        auto json = d.getPlayerJson();
        // Add to global database
        PlayerModel::Instance()->loadPlayer(json);
        PlayerModel::Instance()->saveCache();
        // Add to roster
        playerModel->loadPlayer(json);
    }
}

void DialogTournamentRoster::on_pushButtonRemove_clicked()
{
    auto indexes = ui->treeView->selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
        return;
    auto idx = filterModel->indexToSource(indexes.at(0).row());

    auto ret = QMessageBox::question(this, "Supprimer", "Supprimer le joueur sélectionné de la liste des inscrits ?");
    if (ret != QMessageBox::Yes)
        return;

    playerModel->removeRow(idx);
}

void DialogTournamentRoster::on_pushButtonExport_clicked()
{
    DialogPlayersHtml d(playerModel, this);
    d.exec();
}

void DialogTournamentRoster::on_pushButtonDispatch_clicked()
{
    // First apply current roster changes so dispatch sees them
    auto *rosterModel = tournament->getTournamentPlayerModel();

    // Temporarily replace the tournament roster for dispatch dialog
    // Save, open dispatch dialog, then let it work directly
    rosterModel->clear();
    for (int i = 0; i < playerModel->rowCount(); i++)
        rosterModel->appendClone(playerModel->item(i));

    DialogPlayerDispatch d(tournament);
    d.exec();
}

void DialogTournamentRoster::on_buttonBox_accepted()
{
    // Replace tournament roster with our working copy
    auto *rosterModel = tournament->getTournamentPlayerModel();
    rosterModel->clear();
    for (int i = 0; i < playerModel->rowCount(); i++)
        rosterModel->appendClone(playerModel->item(i));

    accept();
}
