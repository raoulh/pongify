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

        //update QML model
        view->engine()->rootContext()->setContextProperty("currentTournament", currentTournament);
        auto s = currentTournament->getSerie(0);
        if (s)
            view->engine()->rootContext()->setContextProperty("selectedSerie", s);
    });

    //Restore window position
    QSettings settings;
    if (settings.contains("MainWindow/geometry"))
        restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    if (settings.contains("MainWindow/windowState"))
        restoreState(settings.value("MainWindow/windowState").toByteArray());

    update_tournamentOpened(false);
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

    action = menu.addAction(QIcon::fromTheme("play-button"), tr("Démarrer la série"));
    connect(action, &QAction::triggered, this, [this, idx]()
    {
        auto s = currentTournament->getSerie(idx);
        if (!s) return;
        if (s->get_status() != "stopped") return;
        auto ret = QMessageBox::question(this, "Confirmation",
                                         QStringLiteral("Démarrer la série \"%1\" ?")
                                         .arg(s->get_name()));
        if (ret == QMessageBox::Yes)
        {
            s->startSerie();
        }
    });

    action = menu.addAction(QIcon::fromTheme("athlete"), tr("Joueurs..."));
    connect(action, &QAction::triggered, this, [this, idx]()
    {
        auto s = currentTournament->getSerie(idx);
        if (!s) return;
        DialogPlayerList d(s);
        if (d.exec() == QDialog::Accepted)
        {
            TStorage::Instance()->saveToDisk(currentTournament);
        }
    });

    action = menu.addAction(QIcon::fromTheme("casino"), tr("Placer les joueurs automatiquement"));
    connect(action, &QAction::triggered, this, [this, idx]()
    {
        auto s = currentTournament->getSerie(idx);
        if (!s) return;
        s->autoSeedPlayers();
        TStorage::Instance()->saveToDisk(currentTournament);
    });

    action = menu.addAction(QIcon::fromTheme("trash"), tr("Effacer les joueurs placés"));
    connect(action, &QAction::triggered, this, [this, idx]()
    {
        auto s = currentTournament->getSerie(idx);
        if (!s) return;
        s->removeAllPlayers();
        TStorage::Instance()->saveToDisk(currentTournament);
    });

    action = menu.addAction(QIcon::fromTheme("filter"), tr("Propriétés"));
    connect(action, &QAction::triggered, this, [this, idx]()
    {
        editSerie(idx);
    });

    menu.addSeparator();

    action = menu.addAction(QIcon::fromTheme("trash"), tr("Supprimer"));
    connect(action, &QAction::triggered, this, [this, idx]()
    {
        deleteSerie(idx);
    });

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
    if (d.exec() == QDialog::Accepted)
    {
        s->update_name(d.getName());
        s->update_ranking(d.getRanking());
        s->update_tournamentType(d.getType());
        TStorage::Instance()->saveToDisk(currentTournament);
    }
}

void MainWindow::selectSerie(int idx)
{
    auto s = currentTournament->getSerie(idx);
    if (!s) return;
    view->engine()->rootContext()->setContextProperty("selectedSerie", s);
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
    update_tournamentOpened(false);
}

void MainWindow::on_actionPropri_t_s_triggered()
{
    if (!currentTournament) return;
    DialogNewTournament d(false);
    d.setName(currentTournament->get_name());
    d.setDate(currentTournament->get_date().date());
    if (d.exec() == QDialog::Accepted)
    {
        currentTournament->update_name(d.getName());
        currentTournament->update_date(QDateTime(d.getDate(), QTime::currentTime()));
        TStorage::Instance()->saveToDisk(currentTournament);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/windowState", saveState());

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

    view->setSource(QUrl("qrc:/qml/main.qml"));
    ui->verticalLayout->addWidget(container);
}
