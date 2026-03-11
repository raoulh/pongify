#include "DialogQualification.h"
#include "ui_DialogQualification.h"
#include "Tournament.h"
#include "TSerie.h"
#include "PlayerModel.h"
#include "TStorage.h"
#include "Score.h"

#include <QMessageBox>
#include <QTreeWidgetItem>

DialogQualification::DialogQualification(Tournament *t, TSerie *contextSerie,
                                         bool fromSrc, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogQualification),
    tournament(t),
    contextSerie(contextSerie),
    fromSource(fromSrc)
{
    ui->setupUi(this);

    if (fromSource)
    {
        // Mode: from a finished serie, transfer players TO a target
        setWindowTitle(QStringLiteral("Transférer les joueurs depuis \"%1\"").arg(contextSerie->get_name()));

        // Hide source selection (we already know the source)
        ui->labelSource->hide();
        ui->listWidgetSources->hide();

        // Populate target combo with stopped series (excluding self)
        for (int i = 0; i < tournament->serieCount(); i++)
        {
            auto *s = tournament->getSerie(i);
            if (s == contextSerie) continue;
            if (s->get_status() != "stopped") continue;
            ui->comboBoxTarget->addItem(s->get_name(), s->get_serieUid());
        }

        if (ui->comboBoxTarget->count() == 0)
        {
            ui->comboBoxTarget->addItem("(Aucune série cible disponible)");
            ui->comboBoxTarget->setEnabled(false);
        }

        // Build ranked player list from context serie
        buildRankedPlayersFromSerie(contextSerie);
        refreshPlayerList();
    }
    else
    {
        // Mode: from a stopped serie, import players FROM source series
        setWindowTitle(QStringLiteral("Importer des joueurs vers \"%1\"").arg(contextSerie->get_name()));

        // Hide target combo (we already know the target)
        ui->labelTarget->hide();
        ui->comboBoxTarget->hide();

        // Populate source list with finished/playing series (excluding self)
        for (int i = 0; i < tournament->serieCount(); i++)
        {
            auto *s = tournament->getSerie(i);
            if (s == contextSerie) continue;
            if (s->get_status() == "stopped") continue; // Only show series with results

            auto *item = new QListWidgetItem(
                QStringLiteral("%1 (%2)").arg(s->get_name(), s->get_status()));
            item->setData(Qt::UserRole, s->get_serieUid());
            ui->listWidgetSources->addItem(item);
        }

        connect(ui->listWidgetSources, &QListWidget::itemSelectionChanged,
                this, &DialogQualification::onSourceSelectionChanged);
    }
}

DialogQualification::~DialogQualification()
{
    delete ui;
}

void DialogQualification::buildRankedPlayersFromSerie(TSerie *serie)
{
    if (serie->get_tournamentType() == "roundrobin")
    {
        auto scores = serie->getRRWinners();
        for (int i = 0; i < scores.count(); i++)
        {
            if (!scores[i].player) continue;
            QualifiedPlayer qp;
            qp.player = scores[i].player;
            qp.rank = i + 1;
            qp.score = scores[i].score;
            qp.serieName = serie->get_name();
            qp.serieUid = serie->get_serieUid();
            allRankedPlayers.append(qp);
        }
    }
    else if (serie->get_tournamentType() == "single")
    {
        auto winners = serie->getSingleWinners();
        for (int i = 0; i < winners.count(); i++)
        {
            if (!winners[i]) continue;
            QualifiedPlayer qp;
            qp.player = winners[i];
            qp.rank = i + 1;
            qp.score = 0;
            qp.serieName = serie->get_name();
            qp.serieUid = serie->get_serieUid();
            allRankedPlayers.append(qp);
        }
    }
    else
    {
        // Fallback: list all players without ranking
        auto *model = serie->getPlayerModel();
        for (int i = 0; i < model->rowCount(); i++)
        {
            QualifiedPlayer qp;
            qp.player = model->item(i);
            qp.rank = i + 1;
            qp.score = 0;
            qp.serieName = serie->get_name();
            qp.serieUid = serie->get_serieUid();
            allRankedPlayers.append(qp);
        }
    }
}

void DialogQualification::onSourceSelectionChanged()
{
    allRankedPlayers.clear();

    auto selected = ui->listWidgetSources->selectedItems();
    for (auto *item : selected)
    {
        QString uid = item->data(Qt::UserRole).toString();
        auto *serie = tournament->getSerieUid(uid);
        if (serie)
            buildRankedPlayersFromSerie(serie);
    }

    refreshPlayerList();
}

void DialogQualification::refreshPlayerList()
{
    ui->treeWidgetPlayers->clear();

    for (int i = 0; i < allRankedPlayers.count(); i++)
    {
        const auto &qp = allRankedPlayers[i];
        auto *item = new QTreeWidgetItem();

        item->setCheckState(0, Qt::Unchecked);
        item->setText(1, QString::number(qp.rank));
        item->setText(2, QStringLiteral("%1 %2").arg(qp.player->get_firstName(), qp.player->get_lastName()));
        item->setText(3, qp.player->get_ranking());
        item->setText(4, qp.serieName);
        item->setText(5, qp.score > 0 ? QString::number(qp.score) : "-");

        ui->treeWidgetPlayers->addTopLevelItem(item);
    }

    ui->treeWidgetPlayers->resizeColumnToContents(0);
    ui->treeWidgetPlayers->resizeColumnToContents(1);
    ui->treeWidgetPlayers->resizeColumnToContents(2);

    // Auto-apply top N
    on_pushButtonApplyTopN_clicked();
}

void DialogQualification::on_pushButtonApplyTopN_clicked()
{
    int topN = ui->spinBoxTopN->value();

    if (fromSource)
    {
        // Simple: check top N from the single source
        for (int i = 0; i < ui->treeWidgetPlayers->topLevelItemCount(); i++)
        {
            auto *item = ui->treeWidgetPlayers->topLevelItem(i);
            item->setCheckState(0, (i < topN) ? Qt::Checked : Qt::Unchecked);
        }
    }
    else
    {
        // Per-pool top N: check top N from each source serie
        QMap<QString, int> serieCounter; // uid -> count of checked
        for (int i = 0; i < allRankedPlayers.count(); i++)
        {
            const auto &qp = allRankedPlayers[i];
            int &count = serieCounter[qp.serieUid];
            auto *item = ui->treeWidgetPlayers->topLevelItem(i);
            if (count < topN)
            {
                item->setCheckState(0, Qt::Checked);
                count++;
            }
            else
            {
                item->setCheckState(0, Qt::Unchecked);
            }
        }
    }
}

void DialogQualification::on_buttonBox_accepted()
{
    // Determine target serie
    TSerie *targetSerie = nullptr;
    if (fromSource)
    {
        QString targetUid = ui->comboBoxTarget->currentData().toString();
        targetSerie = tournament->getSerieUid(targetUid);
    }
    else
    {
        targetSerie = contextSerie;
    }

    if (!targetSerie)
    {
        QMessageBox::warning(this, "Erreur", "Aucune série cible valide sélectionnée.");
        return;
    }

    if (targetSerie->get_status() != "stopped")
    {
        QMessageBox::warning(this, "Erreur", "La série cible doit être en statut 'stopped'.");
        return;
    }

    // Collect checked players
    QList<Player *> selectedPlayers;
    QStringList sourceUids;
    for (int i = 0; i < ui->treeWidgetPlayers->topLevelItemCount(); i++)
    {
        auto *item = ui->treeWidgetPlayers->topLevelItem(i);
        if (item->checkState(0) == Qt::Checked && i < allRankedPlayers.count())
        {
            selectedPlayers.append(allRankedPlayers[i].player);
            if (!sourceUids.contains(allRankedPlayers[i].serieUid))
                sourceUids.append(allRankedPlayers[i].serieUid);
        }
    }

    if (selectedPlayers.isEmpty())
    {
        QMessageBox::information(this, "Info", "Aucun joueur sélectionné.");
        return;
    }

    // Add selected players to target serie (keep existing players)
    auto *targetModel = PlayerModel::createEmpty();

    // Copy existing players first
    for (int i = 0; i < targetSerie->getPlayerModel()->rowCount(); i++)
        targetModel->appendClone(targetSerie->getPlayerModel()->item(i));

    // Add new qualified players (skip duplicates)
    for (auto *p : selectedPlayers)
    {
        if (!targetModel->getFromLicense(p->get_license()))
            targetModel->appendClone(p);
    }

    targetSerie->replaceAllPlayers(targetModel);
    delete targetModel;

    // Update feedFrom on target serie
    QStringList currentFeed = targetSerie->get_feedFrom();
    for (const auto &uid : sourceUids)
    {
        if (!currentFeed.contains(uid))
            currentFeed.append(uid);
    }
    targetSerie->update_feedFrom(currentFeed);

    TStorage::Instance()->saveToDisk(tournament);
    accept();
}
