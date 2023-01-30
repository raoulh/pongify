#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "PlayerSync.h"
#include "PlayerModel.h"

#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QTimer::singleShot(100, this, []()
    {
        PlayerModel::Instance()->loadCache();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionQuitter_triggered()
{

}

void MainWindow::on_actionMettre_jour_la_liste_de_joueur_depuis_le_CDSLS_triggered()
{
    PlayerSync p;
    p.start();
}

void MainWindow::on_actionNouveau_tournoi_triggered()
{

}
