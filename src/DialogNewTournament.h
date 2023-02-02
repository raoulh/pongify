#ifndef DIALOGNEWTOURNAMENT_H
#define DIALOGNEWTOURNAMENT_H

#include <QDialog>

namespace Ui {
class DialogNewTournament;
}

class DialogNewTournament : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewTournament(bool newTournament, QWidget *parent = nullptr);
    ~DialogNewTournament();

    void setName(QString name);
    QString getName();
    void setDate(QDate date);
    QDate getDate();

private:
    Ui::DialogNewTournament *ui;
};

#endif // DIALOGNEWTOURNAMENT_H
