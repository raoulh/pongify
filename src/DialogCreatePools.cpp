#include "DialogCreatePools.h"
#include "ui_DialogCreatePools.h"
#include "Tournament.h"
#include "PlayerModel.h"
#include "TSerie.h"
#include "TStorage.h"

#include <QCollator>
#include <QHeaderView>
#include <QMessageBox>
#include <algorithm>
#include <random>

DialogCreatePools::DialogCreatePools(Tournament *t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCreatePools),
    tournament(t)
{
    ui->setupUi(this);

    int playerCount = tournament->getTournamentPlayerModel()->rowCount();
    ui->labelSourceCount->setText(QStringLiteral("%1 joueurs inscrits au tournoi").arg(playerCount));

    // Distribution methods
    ui->comboBoxMethod->addItem("Serpentine par classement", "serpentine");
    ui->comboBoxMethod->addItem("Aléatoire", "random");

    // Ranking options (same as DialogNewSerie)
    ui->comboBoxRanking->addItem("Open");
    ui->comboBoxRanking->addItem("NC");
    QStringList ranks = {"90", "85", "80", "75", "70", "65", "60", "55", "50", "45", "40", "35", "30", "25", "20", "15", "10", "5"};
    ui->comboBoxRanking->addItems(ranks);

    // Update preview when config changes
    connect(ui->spinBoxPoolCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &DialogCreatePools::updatePreview);
    connect(ui->comboBoxMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DialogCreatePools::updatePreview);

    updatePreview();
}

DialogCreatePools::~DialogCreatePools()
{
    delete ui;
}

QVector<Player *> DialogCreatePools::sortedPlayers()
{
    auto *roster = tournament->getTournamentPlayerModel();
    QVector<Player *> allp;
    for (int i = 0; i < roster->rowCount(); i++)
        allp.append(roster->item(i));

    QCollator sorter;
    std::sort(allp.begin(), allp.end(),
              [&sorter](Player *a, Player *b)
    {
        return sorter.compare(a->get_ranking(), b->get_ranking()) < 0;
    });

    return allp;
}

QVector<QVector<Player *>> DialogCreatePools::distributePlayersToPool(int poolCount, bool serpentine)
{
    QVector<QVector<Player *>> pools(poolCount);
    QVector<Player *> allp = sortedPlayers();

    if (!serpentine)
    {
        // Shuffle for random distribution
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(allp.begin(), allp.end(), g);
    }

    // Serpentine distribution:
    // Row 0 (→): Pool 0, 1, 2, ...
    // Row 1 (←): Pool N-1, N-2, ..., 0
    // Row 2 (→): Pool 0, 1, 2, ...
    // etc.
    for (int i = 0; i < allp.count(); i++)
    {
        int row = i / poolCount;
        int col = i % poolCount;

        // Reverse direction on odd rows (serpentine)
        if (serpentine && (row % 2 == 1))
            col = poolCount - 1 - col;

        pools[col].append(allp[i]);
    }

    return pools;
}

void DialogCreatePools::updatePreview()
{
    int poolCount = ui->spinBoxPoolCount->value();
    bool serpentine = ui->comboBoxMethod->currentData().toString() == "serpentine";

    auto pools = distributePlayersToPool(poolCount, serpentine);

    // Find max pool size for row count
    int maxSize = 0;
    for (const auto &pool : pools)
        maxSize = qMax(maxSize, pool.count());

    ui->tableWidgetPreview->clear();
    ui->tableWidgetPreview->setRowCount(maxSize);
    ui->tableWidgetPreview->setColumnCount(poolCount);

    // Pool name headers: A, B, C, ...
    QString prefix = ui->lineEditPrefix->text();
    QStringList headers;
    for (int i = 0; i < poolCount; i++)
    {
        QString poolName = QStringLiteral("%1 %2").arg(prefix, QString(QChar('A' + i)));
        headers << QStringLiteral("%1 (%2)").arg(poolName).arg(pools[i].count());
    }
    ui->tableWidgetPreview->setHorizontalHeaderLabels(headers);

    for (int col = 0; col < poolCount; col++)
    {
        for (int row = 0; row < pools[col].count(); row++)
        {
            auto *p = pools[col][row];
            QString text = QStringLiteral("%1 %2 [%3]")
                               .arg(p->get_firstName(), p->get_lastName(), p->get_ranking());
            auto *item = new QTableWidgetItem(text);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            ui->tableWidgetPreview->setItem(row, col, item);
        }
    }

    ui->tableWidgetPreview->resizeColumnsToContents();
}

void DialogCreatePools::on_buttonBox_accepted()
{
    int poolCount = ui->spinBoxPoolCount->value();
    bool serpentine = ui->comboBoxMethod->currentData().toString() == "serpentine";
    QString prefix = ui->lineEditPrefix->text();
    QString ranking = ui->comboBoxRanking->currentText();

    auto *roster = tournament->getTournamentPlayerModel();
    if (roster->rowCount() < poolCount)
    {
        QMessageBox::warning(this, "Erreur",
                             "Il n'y a pas assez de joueurs inscrits pour créer ce nombre de poules.");
        return;
    }

    auto pools = distributePlayersToPool(poolCount, serpentine);

    // Create round-robin series for each pool
    for (int i = 0; i < poolCount; i++)
    {
        auto *serie = new TSerie();
        serie->update_name(QStringLiteral("%1 %2").arg(prefix, QString(QChar('A' + i))));
        serie->update_ranking(ranking);
        serie->update_tournamentType("roundrobin");
        serie->update_status("stopped");
        serie->update_isDouble(false);
        serie->update_isHandicap(false);

        // Build a temporary PlayerModel to populate the serie
        auto *tempModel = PlayerModel::createEmpty();
        for (auto *p : pools[i])
            tempModel->appendClone(p);

        serie->replaceAllPlayers(tempModel);
        delete tempModel;

        tournament->addSerie(serie);
    }

    TStorage::Instance()->saveToDisk(tournament);
    created = true;
    accept();
}
