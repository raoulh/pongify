#include "DialogPlayerDispatch.h"
#include "ui_DialogPlayerDispatch.h"
#include "Tournament.h"
#include "PlayerModel.h"
#include "TSerie.h"
#include "TStorage.h"

#include <QHeaderView>

DialogPlayerDispatch::DialogPlayerDispatch(Tournament *t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPlayerDispatch),
    tournament(t)
{
    ui->setupUi(this);

    auto *roster = tournament->getTournamentPlayerModel();
    int playerCount = roster->rowCount();
    int serieCount = tournament->serieCount();

    if (playerCount == 0 || serieCount == 0)
    {
        ui->labelInfo->setText("Aucun joueur inscrit ou aucune série dans le tournoi.");
        return;
    }

    // Setup table: rows = players, columns = name + ranking + one column per serie
    int fixedCols = 2; // Name, Ranking
    ui->tableWidget->setRowCount(playerCount);
    ui->tableWidget->setColumnCount(fixedCols + serieCount);

    // Headers
    QStringList headers;
    headers << "Joueur" << "Classement";
    for (int s = 0; s < serieCount; s++)
    {
        auto *serie = tournament->getSerie(s);
        headers << serie->get_name();
    }
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    checkboxes.resize(playerCount);

    for (int p = 0; p < playerCount; p++)
    {
        auto *player = roster->item(p);
        QString playerName = QStringLiteral("%1 %2").arg(player->get_firstName(), player->get_lastName());
        QString playerLicense = player->get_license();

        // Name column (read-only)
        auto *nameItem = new QTableWidgetItem(playerName);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(p, 0, nameItem);

        // Ranking column (read-only)
        auto *rankItem = new QTableWidgetItem(player->get_ranking());
        rankItem->setFlags(rankItem->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(p, 1, rankItem);

        checkboxes[p].resize(serieCount);

        for (int s = 0; s < serieCount; s++)
        {
            auto *serie = tournament->getSerie(s);
            auto *cb = new QCheckBox();

            // Check if player is already in this serie
            if (serie->getPlayerModel()->getFromLicense(playerLicense))
                cb->setChecked(true);

            // Disable checkbox for series that are playing or finished
            if (serie->get_status() != "stopped")
                cb->setEnabled(false);

            // Disable for double series (must be managed via DialogPlayerList)
            if (serie->get_isDouble())
            {
                cb->setEnabled(false);
                cb->setToolTip("Les séries doubles se gèrent via le menu Joueurs de la série");
            }

            checkboxes[p][s] = cb;
            ui->tableWidget->setCellWidget(p, fixedCols + s, cb);
        }
    }

    ui->tableWidget->resizeColumnsToContents();
}

DialogPlayerDispatch::~DialogPlayerDispatch()
{
    delete ui;
}

void DialogPlayerDispatch::on_buttonBox_accepted()
{
    auto *roster = tournament->getTournamentPlayerModel();
    int playerCount = roster->rowCount();
    int serieCount = tournament->serieCount();

    for (int s = 0; s < serieCount; s++)
    {
        auto *serie = tournament->getSerie(s);

        // Skip series that cannot be modified
        if (serie->get_status() != "stopped" || serie->get_isDouble())
            continue;

        // Build new player list for this serie
        auto *newModel = PlayerModel::createEmpty();

        for (int p = 0; p < playerCount; p++)
        {
            if (checkboxes[p][s]->isChecked())
                newModel->appendClone(roster->item(p));
        }

        // Also keep existing players that are NOT in the roster
        // (they were added from elsewhere and shouldn't be removed by dispatch)
        auto *existing = serie->getPlayerModel();
        for (int i = 0; i < existing->rowCount(); i++)
        {
            auto *player = existing->item(i);
            bool inRoster = false;
            for (int p = 0; p < playerCount; p++)
            {
                if (roster->item(p)->get_license() == player->get_license())
                {
                    inRoster = true;
                    break;
                }
            }
            if (!inRoster && !newModel->getFromLicense(player->get_license()))
                newModel->appendClone(player);
        }

        serie->replaceAllPlayers(newModel);
        delete newModel;
    }

    TStorage::Instance()->saveToDisk(tournament);
    accept();
}
