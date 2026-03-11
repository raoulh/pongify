#ifndef DIALOGPLAYERSHTML_H
#define DIALOGPLAYERSHTML_H

#include <QDialog>
#include "Tournament.h"

namespace Ui {
class DialogPlayersHtml;
}

class PlayerModel;

class DialogPlayersHtml : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPlayersHtml(Tournament *t, QWidget *parent = nullptr);
    explicit DialogPlayersHtml(PlayerModel *model, QWidget *parent = nullptr);
    ~DialogPlayersHtml();

private:
    Ui::DialogPlayersHtml *ui;

    Tournament *tournament = nullptr;
    PlayerModel *rosterModel = nullptr;

    QString readFile(QString f);
    QString buildHtml();
};

#endif // DIALOGPLAYERSHTML_H
