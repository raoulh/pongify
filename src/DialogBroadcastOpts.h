#ifndef DIALOGBROADCASTOPTS_H
#define DIALOGBROADCASTOPTS_H

#include <QDialog>

namespace Ui {
class DialogBroadcastOpts;
}

class DialogBroadcastOpts : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBroadcastOpts(QWidget *parent = nullptr);
    ~DialogBroadcastOpts();

    QScreen *getScreen();
    bool getFullscreen();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DialogBroadcastOpts *ui;
};

#endif // DIALOGBROADCASTOPTS_H
