#include "MainWindow.h"
#include "Tournament.h"
#include "ui_MainWindow.h"
#include "PlayerSync.h"
#include "PlayerModel.h"
#include "DialogPlayers.h"
#include "DialogNewTournament.h"
#include "TStorage.h"

#include <QQuickStyle>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QTimer>
#include <QLabel>
#include <QSettings>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QIcon::setThemeName("pongify");

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

void MainWindow::on_actionMettre_jour_la_liste_de_joueur_depuis_le_CDSLS_triggered()
{
    PlayerSync p;
    p.start();
}

void MainWindow::on_actionListe_des_joueurs_triggered()
{
    DialogPlayers d(PlayerModel::Instance());
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

    QQuickView *view = new QQuickView();
    QWidget *container = QWidget::createWindowContainer(view, this);
    container->setMinimumSize(1024, 600);
    container->setFocusPolicy(Qt::TabFocus);

    view->engine()->rootContext()->setContextProperty("mainWindow", this);
    view->engine()->rootContext()->setContextProperty("storage", TStorage::Instance());
    view->engine()->rootContext()->setContextProperty("playerModel", PlayerModel::Instance());

    view->setSource(QUrl("qrc:/qml/main.qml"));
    ui->verticalLayout->addWidget(container);
}
