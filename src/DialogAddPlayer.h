#ifndef DIALOGADDPLAYER_H
#define DIALOGADDPLAYER_H

#include <QDialog>
#include <QJsonObject>

namespace Ui {
class DialogAddPlayer;
}

class DialogAddPlayer : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddPlayer(QWidget *parent = nullptr);
    ~DialogAddPlayer();

    QJsonObject getPlayerJson();

private:
    Ui::DialogAddPlayer *ui;
};

#endif // DIALOGADDPLAYER_H
