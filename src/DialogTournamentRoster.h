#ifndef DIALOGTOURNAMENTROSTER_H
#define DIALOGTOURNAMENTROSTER_H

#include <QDialog>

namespace Ui {
class DialogTournamentRoster;
}

class PlayerModel;
class PlayerFilterModel;
class Tournament;

class DialogTournamentRoster : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTournamentRoster(Tournament *tournament, QWidget *parent = nullptr);
    ~DialogTournamentRoster();

private slots:
    void on_pushButtonAdd_clicked();
    void on_pushButtonAddManual_clicked();
    void on_pushButtonRemove_clicked();
    void on_pushButtonExport_clicked();
    void on_pushButtonDispatch_clicked();
    void on_buttonBox_accepted();

private:
    Ui::DialogTournamentRoster *ui;
    PlayerModel *playerModel = nullptr;
    PlayerFilterModel *filterModel = nullptr;
    Tournament *tournament = nullptr;
};

#endif // DIALOGTOURNAMENTROSTER_H
