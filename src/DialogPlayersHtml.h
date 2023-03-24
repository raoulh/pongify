#ifndef DIALOGPLAYERSHTML_H
#define DIALOGPLAYERSHTML_H

#include <QDialog>
#include "Tournament.h"

namespace Ui {
class DialogPlayersHtml;
}

class DialogPlayersHtml : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPlayersHtml(Tournament *t, QWidget *parent = nullptr);
    ~DialogPlayersHtml();

private:
    Ui::DialogPlayersHtml *ui;

    Tournament *tournament = nullptr;

    QString readFile(QString f);
    QString buildHtml();
};

#endif // DIALOGPLAYERSHTML_H
