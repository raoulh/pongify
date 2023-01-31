#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionMettre_jour_la_liste_de_joueur_depuis_le_CDSLS_triggered();
    void on_actionNouveau_tournoi_triggered();
    void on_actionListe_des_joueurs_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
