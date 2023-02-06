#ifndef DIALOGPLAYERLIST_H
#define DIALOGPLAYERLIST_H

#include <QDialog>

namespace Ui {
class DialogPlayerList;
}

class PlayerModel;
class PlayerFilterModel;
class TSerie;

class DialogPlayerList : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPlayerList(TSerie *serie, QWidget *parent = nullptr);
    ~DialogPlayerList();

private slots:
    void on_pushButtonAdd_clicked();
    void on_pushButtonRemove_clicked();

    void on_buttonBox_accepted();

private:
    Ui::DialogPlayerList *ui;
    PlayerModel *playerModel = nullptr;
    PlayerFilterModel *filterModel = nullptr;
    TSerie *serie = nullptr;

    bool checkRanking(QString rank);
};

#endif // DIALOGPLAYERLIST_H
