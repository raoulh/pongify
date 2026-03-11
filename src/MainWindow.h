#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qqmlhelpers.h"
#include "TableMatchModel.h"

class BroadcastPreviewProvider;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Tournament;
class QQuickView;
class BroadcastWindow;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QML_READONLY_PROPERTY(bool, tournamentOpened)
    QML_READONLY_PROPERTY(bool, broadcastActive)

    //broadcast
    QML_READONLY_PROPERTY(QObject *, broadcastViews)
    QML_READONLY_PROPERTY(int, currentBrodcastViewIndex)

    QML_READONLY_PROPERTY(QObject *, matchTableModel)

    //broadcast preview
    QML_READONLY_PROPERTY(bool, broadcastPreviewActive)
    QML_READONLY_PROPERTY(int, previewUpdateCounter)

    //web publish
    QML_READONLY_PROPERTY(bool, webPublishEnabled)
    QML_READONLY_PROPERTY(QString, webPublishUrl)

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
    Q_INVOKABLE void createPools();

    Q_INVOKABLE void newTable();
    Q_INVOKABLE void deleteTable();
    Q_INVOKABLE void selectTable(int idx);
    Q_INVOKABLE void selectMatchTable(int idx);
    Q_INVOKABLE void startMatchOnTable(int idx, int table);
    Q_INVOKABLE void showTableMenu(int idx);

    Q_INVOKABLE void broadcastStart();
    Q_INVOKABLE void broadcastStop();
    Q_INVOKABLE void broadcastNext();
    Q_INVOKABLE void broadcastPrevious();
    Q_INVOKABLE void broadcastEditInfo();

    Q_INVOKABLE void broadcastTogglePreview();

    Q_INVOKABLE void webPublishConfig();
    Q_INVOKABLE void webPublishToggle();
    Q_INVOKABLE void showTournamentQrCode();

    void exportTournament();
    void importTournament();
    void restoreBackup();

private slots:
    void on_actionMettre_jour_la_liste_de_joueur_depuis_le_CDSLS_triggered();
    void on_actionListe_des_joueurs_triggered();
    void on_actionFermer_triggered();
    void on_actionPropri_t_s_triggered();
    void on_actionA_propos_triggered();
    void on_actionTournamentRoster_triggered();

private:
    Ui::MainWindow *ui;

    QQuickView *view = nullptr;
    Tournament *currentTournament = nullptr;
    BroadcastWindow *broadcastWin = nullptr;
    QQmlObjectListModel<TableMatchItem> *matchTableModel = nullptr;

    QTimer *previewTimer = nullptr;
    BroadcastPreviewProvider *previewProvider = nullptr;
    QLabel *liveStatusLabel = nullptr;

    virtual void closeEvent(QCloseEvent *event) override;

    void loadQmlApp();

    void buildMatchTableModel();
};
#endif // MAINWINDOW_H
