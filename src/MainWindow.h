#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qqmlhelpers.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Tournament;
class QQuickView;
class BroadcastWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QML_READONLY_PROPERTY(bool, tournamentOpened)
    QML_READONLY_PROPERTY(bool, broadcastActive)

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Q_INVOKABLE void newTournament();
    Q_INVOKABLE void openTournament(int idx);
    Q_INVOKABLE void deleteTournament(int idx);
    Q_INVOKABLE void showSerieMenu(int idx);
    Q_INVOKABLE void newSerie();
    Q_INVOKABLE void deleteSerie(int idx);
    Q_INVOKABLE void editSerie(int idx);
    Q_INVOKABLE void selectSerie(int idx);

    Q_INVOKABLE void broadcastStart();
    Q_INVOKABLE void broadcastStop();

private slots:
    void on_actionMettre_jour_la_liste_de_joueur_depuis_le_CDSLS_triggered();
    void on_actionListe_des_joueurs_triggered();
    void on_actionFermer_triggered();
    void on_actionPropri_t_s_triggered();

private:
    Ui::MainWindow *ui;

    QQuickView *view;
    Tournament *currentTournament = nullptr;
    BroadcastWindow *broadcastWin = nullptr;

    virtual void closeEvent(QCloseEvent *event) override;

    void loadQmlApp();
};
#endif // MAINWINDOW_H
