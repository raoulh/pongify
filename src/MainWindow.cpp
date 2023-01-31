#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "PlayerSync.h"
#include "PlayerModel.h"
#include "DialogPlayers.h"

#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QLabel *cpyLabel = new QLabel("(c) 2023 Raoul Hecky");
    ui->statusbar->addPermanentWidget(cpyLabel, 1);

    QTimer::singleShot(100, this, []()
    {
        PlayerModel::Instance()->loadCache();
    });

    connect(ui->actionQuitter, &QAction::triggered, this, [this]()
    {
        close();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionMettre_jour_la_liste_de_joueur_depuis_le_CDSLS_triggered()
{
    PlayerSync p;
    p.start();
}

void MainWindow::on_actionNouveau_tournoi_triggered()
{

}

void MainWindow::on_actionListe_des_joueurs_triggered()
{
    DialogPlayers d(PlayerModel::Instance());
    d.exec();
}
