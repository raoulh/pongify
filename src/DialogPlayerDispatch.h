#ifndef DIALOGPLAYERDISPATCH_H
#define DIALOGPLAYERDISPATCH_H

#include <QDialog>
#include <QCheckBox>

namespace Ui {
class DialogPlayerDispatch;
}

class Tournament;

class DialogPlayerDispatch : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPlayerDispatch(Tournament *tournament, QWidget *parent = nullptr);
    ~DialogPlayerDispatch();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DialogPlayerDispatch *ui;
    Tournament *tournament = nullptr;

    // checkboxes[row][col] — row = player index, col = serie index
    QVector<QVector<QCheckBox *>> checkboxes;
};

#endif // DIALOGPLAYERDISPATCH_H
