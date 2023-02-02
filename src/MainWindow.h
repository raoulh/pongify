#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qqmlhelpers.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Tournament;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QML_READONLY_PROPERTY(bool, tournamentOpened)

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Q_INVOKABLE void newTournament();
    Q_INVOKABLE void openTournament(int idx);
    Q_INVOKABLE void deleteTournament(int idx);

private slots:
    void on_actionMettre_jour_la_liste_de_joueur_depuis_le_CDSLS_triggered();
    void on_actionListe_des_joueurs_triggered();
    void on_actionFermer_triggered();
    void on_actionPropri_t_s_triggered();

private:
    Ui::MainWindow *ui;

    Tournament *currentTournament = nullptr;

    virtual void closeEvent(QCloseEvent *event) override;

    void loadQmlApp();
};
#endif // MAINWINDOW_H
