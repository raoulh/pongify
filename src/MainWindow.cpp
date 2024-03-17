#include "MainWindow.h"
#include "Tournament.h"
#include "ui_MainWindow.h"
#include "PlayerSync.h"
#include "PlayerModel.h"
#include "DialogPlayers.h"
#include "DialogNewTournament.h"
#include "DialogNewSerie.h"
#include "TStorage.h"
#include "DialogPlayerList.h"
#include "DialogBroadcastOpts.h"
#include "BroadcastWindow.h"
#include "DialogEditInfo.h"
#include "DialogPlayersHtml.h"
#include "DialogAbout.h"

#include <QQuickStyle>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QTimer>
#include <QLabel>
#include <QSettings>
#include <QMessageBox>
#include <QWidgetAction>

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QIcon::setThemeName("pongify");

    setWindowTitle("Pongify - Gestion de tournoi");

    QLabel *cpyLabel = new QLabel("(c) 2023 Raoul Hecky");
    ui->statusbar->addPermanentWidget(cpyLabel, 1);

    QTimer::singleShot(100, this, []()
    {
        TStorage::Instance()->loadFromDisk();
        PlayerModel::Instance()->loadCache();
    });

    connect(ui->actionQuitter, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionNouveau_tournoi, &QAction::triggered, this, &MainWindow::newTournament);
    connect(this, &MainWindow::tournamentOpenedChanged, this, [this](bool en)
    {
        ui->actionFermer->setEnabled(en);
        ui->actionPropri_t_s->setEnabled(en);
        ui->actionListe_des_joueurs_du_tournoi->setEnabled(en);

        //update QML model
        if (!view) return;
        view->engine()->rootContext()->setContextProperty("currentTournament", currentTournament);
        auto s = currentTournament? currentTournament->getSerie(0): nullptr;
        view->engine()->rootContext()->setContextProperty("selectedSerie", s);
    });

    //Restore window position
    QSettings settings;
    if (settings.contains("MainWindow/geometry"))
        restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    if (settings.contains("MainWindow/windowState"))
        restoreState(settings.value("MainWindow/windowState").toByteArray());

    update_tournamentOpened(false);
    emit tournamentOpenedChanged(false);
    update_broadcastActive(false);
    update_currentBrodcastViewIndex(0);
    update_broadcastViews(nullptr);
    loadQmlApp();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newTournament()
{
    DialogNewTournament d(true);
    if (d.exec() == QDialog::Accepted)
    {
        currentTournament = TStorage::Instance()->createNewTournament(d.getName());
        currentTournament->update_date(QDateTime(d.getDate(), QTime::currentTime()));
        currentTournament->update_timeBroadcastChange(d.getTime());
        update_tournamentOpened(true);
    }
}

void MainWindow::openTournament(int idx)
{
    currentTournament = TStorage::Instance()->item(idx);
    update_tournamentOpened(currentTournament != nullptr);
}

void MainWindow::deleteTournament(int idx)
{
    auto t = TStorage::Instance()->item(idx);
    if (!t) return;
    auto ret = QMessageBox::question(this, "Confirmation",
                                     QStringLiteral("Supprimer le tournoi \"%1\" ?")
                                     .arg(t->get_name()));
    if (ret == QMessageBox::Yes)
        TStorage::Instance()->deleteTournament(t);
}

void MainWindow::showSerieMenu(int idx)
{
    QMenu menu;
    QAction *action = nullptr;
    auto serie = currentTournament->getSerie(idx);
    if (!serie) return;

    if (serie->get_status() == "stopped")
    {
        action = menu.addAction(QIcon::fromTheme("play-button"), tr("Démarrer la série"));
        connect(action, &QAction::triggered, this, [this, serie]()
        {
            auto ret = QMessageBox::question(this, "Confirmation",
                                             QStringLiteral("Démarrer la série \"%1\" ?")
                                             .arg(serie->get_name()));
            if (ret == QMessageBox::Yes)
            {
                serie->startSerie();
            }
        });
    }

    if (serie->get_status() == "playing")
    {
        action = menu.addAction(QIcon::fromTheme("stop-button"), tr("Terminer la série"));
        connect(action, &QAction::triggered, this, [this, serie]()
        {
            auto ret = QMessageBox::question(this, "Confirmation",
                                             QStringLiteral("Terminer la série \"%1\" ?")
                                             .arg(serie->get_name()));
            if (ret == QMessageBox::Yes)
            {
                serie->stopSerie();
            }
        });
    }

    action = menu.addAction(QIcon::fromTheme("athlete"), tr("Joueurs..."));
    connect(action, &QAction::triggered, this, [this, serie]()
    {
        DialogPlayerList d(serie);
        if (d.exec() == QDialog::Accepted)
        {
            TStorage::Instance()->saveToDisk(currentTournament);
        }
    });
    if (serie->get_status() != "stopped")
        action->setDisabled(true);

    if (serie->get_tournamentType() == "single")
    {
        action = menu.addAction(QIcon::fromTheme("casino"), tr("Placer les joueurs automatiquement"));
        connect(action, &QAction::triggered, this, [this, serie]()
        {
            serie->autoSeedPlayers();
            TStorage::Instance()->saveToDisk(currentTournament);
        });
        if (serie->get_status() != "stopped")
            action->setDisabled(true);

        action = menu.addAction(QIcon::fromTheme("trash"), tr("Effacer les joueurs placés"));
        connect(action, &QAction::triggered, this, [this, serie]()
        {
            serie->removeAllPlayers();
            TStorage::Instance()->saveToDisk(currentTournament);
        });
        if (serie->get_status() != "stopped")
            action->setDisabled(true);
    }

    action = menu.addAction(QIcon::fromTheme("filter"), tr("Propriétés"));
    connect(action, &QAction::triggered, this, [this, idx]()
    {
        editSerie(idx);
    });
    if (serie->get_status() != "stopped")
        action->setDisabled(true);

    action = menu.addAction(QIcon::fromTheme("high-score"), tr("Podium"));
    connect(action, &QAction::triggered, this, [serie]()
    {
        serie->showPodium();
    });
    if (serie->get_status() == "playing" && serie->get_tournamentType() == "single" &&
        serie->get_currentRound() < serie->get_rounds() - 1)
        action->setDisabled(true);

    menu.addSeparator();

    action = menu.addAction(QIcon::fromTheme("trash"), tr("Supprimer"));
    connect(action, &QAction::triggered, this, [this, idx]()
    {
        deleteSerie(idx);
    });
    if (serie->get_status() == "playing")
        action->setDisabled(true);

    menu.exec(QCursor::pos());
}

void MainWindow::newSerie()
{
    DialogNewSerie d(true);
    if (d.exec() == QDialog::Accepted)
    {
        auto s = new TSerie();
        s->update_name(d.getName());
        s->update_ranking(d.getRanking());
        s->update_tournamentType(d.getType());
        s->update_status("stopped");
        s->update_isDouble(d.getDouble());
        s->update_isHandicap(d.getHandicap());

        currentTournament->addSerie(s);
        TStorage::Instance()->saveToDisk(currentTournament);
    }
}

void MainWindow::deleteSerie(int idx)
{
    auto s = currentTournament->getSerie(idx);
    if (!s) return;

    auto ret = QMessageBox::question(this, "Confirmation",
                                     QStringLiteral("Supprimer la série \"%1\" ?")
                                     .arg(s->get_name()));
    if (ret == QMessageBox::Yes)
    {
        currentTournament->removeSerie(idx);
        TStorage::Instance()->saveToDisk(currentTournament);
    }
}

void MainWindow::editSerie(int idx)
{
    auto s = currentTournament->getSerie(idx);
    if (!s) return;

    DialogNewSerie d(false);
    d.setName(s->get_name());
    d.setRanking(s->get_ranking());
    d.setType(s->get_tournamentType());
    d.setDouble(s->get_isDouble());
    d.setHandicap(s->get_isHandicap());
    if (d.exec() == QDialog::Accepted)
    {
        s->update_name(d.getName());
        s->update_ranking(d.getRanking());
        s->update_tournamentType(d.getType());
        s->update_isDouble(d.getDouble());
        s->update_isHandicap(d.getHandicap());
        TStorage::Instance()->saveToDisk(currentTournament);
    }
}

void MainWindow::selectSerie(int idx)
{
    auto s = currentTournament->getSerie(idx);
    if (!s) return;
    view->engine()->rootContext()->setContextProperty("selectedSerie", s);
    emit s->matchesUpdated(); //force update matches in QML
}

void MainWindow::broadcastStart()
{
    DialogBroadcastOpts d;
    if (d.exec() == QDialog::Accepted)
    {
        update_broadcastActive(true);
        if (broadcastWin)
            broadcastWin->deleteLater();
        broadcastWin = new BroadcastWindow(d.getScreen(), d.getFullscreen(), currentTournament, this);
        connect(broadcastWin, &BroadcastWindow::windowClosed, this, [=]()
        {
            broadcastStop();
        });
        update_currentBrodcastViewIndex(0);
        update_broadcastViews(broadcastWin->get_views());
        update_currentBrodcastViewIndex(broadcastWin->get_currentViewIndex());
        connect(broadcastWin, &BroadcastWindow::currentViewIndexChanged, this, &MainWindow::update_currentBrodcastViewIndex);
    }
}

void MainWindow::broadcastStop()
{
    update_broadcastActive(false);
    update_currentBrodcastViewIndex(0);
    update_broadcastViews(nullptr);
    if (broadcastWin)
        broadcastWin->deleteLater();
    broadcastWin = nullptr;
}

void MainWindow::broadcastNext()
{
    if (broadcastWin)
        broadcastWin->nextView();
}

void MainWindow::broadcastPrevious()
{
    if (broadcastWin)
        broadcastWin->previousView();
}

void MainWindow::broadcastEditInfo()
{
    DialogEditInfo d;
    d.setText(currentTournament->get_infoText());
    if (d.exec() == QDialog::Accepted)
    {
        currentTournament->update_infoText(d.getText());
        TStorage::Instance()->saveToDisk(currentTournament);
    }
}

void MainWindow::on_actionMettre_jour_la_liste_de_joueur_depuis_le_CDSLS_triggered()
{
    PlayerSync p;
    p.start();
}

void MainWindow::on_actionListe_des_joueurs_triggered()
{
    DialogPlayers d(PlayerModel::Instance(), false);
    d.exec();
}

void MainWindow::on_actionFermer_triggered()
{
    if (currentTournament)
        TStorage::Instance()->saveToDisk(currentTournament);
    currentTournament = nullptr;
    broadcastStop();
    update_tournamentOpened(false);
}

void MainWindow::on_actionPropri_t_s_triggered()
{
    if (!currentTournament) return;
    DialogNewTournament d(false);
    d.setName(currentTournament->get_name());
    d.setDate(currentTournament->get_date().date());
    d.setTime(currentTournament->get_timeBroadcastChange());
    if (d.exec() == QDialog::Accepted)
    {
        currentTournament->update_name(d.getName());
        currentTournament->update_date(QDateTime(d.getDate(), QTime::currentTime()));
        currentTournament->update_timeBroadcastChange(d.getTime());
        TStorage::Instance()->saveToDisk(currentTournament);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/windowState", saveState());

    //do not use broadcastStop() here, as it uses deleteLater which will not happen
    // because the mainloop has already stopped
    delete broadcastWin;

    if (currentTournament)
        TStorage::Instance()->saveToDisk(currentTournament);

    event->accept();
}

void MainWindow::loadQmlApp()
{
    QQuickStyle::setStyle("Imagine");

    view = new QQuickView();
    QWidget *container = QWidget::createWindowContainer(view, this);
    container->setMinimumSize(1024, 600);
    container->setFocusPolicy(Qt::TabFocus);

    view->engine()->rootContext()->setContextProperty("mainWindow", this);
    view->engine()->rootContext()->setContextProperty("storage", TStorage::Instance());
    view->engine()->rootContext()->setContextProperty("playerModel", PlayerModel::Instance());
    view->engine()->rootContext()->setContextProperty("selectedSerie", nullptr);
    view->engine()->rootContext()->setContextProperty("currentTournament", nullptr);

    view->setSource(QUrl("qrc:/qml/main.qml"));
    ui->verticalLayout->addWidget(container);
}

void MainWindow::on_actionListe_des_joueurs_du_tournoi_triggered()
{
    DialogPlayersHtml d(currentTournament);
    d.exec();
}

void MainWindow::on_actionA_propos_triggered()
{
    DialogAbout d(this);
    d.exec();
}
