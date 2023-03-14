#ifndef DIALOGEDITINFO_H
#define DIALOGEDITINFO_H

#include <QDialog>

namespace Ui {
class DialogEditInfo;
}

class DialogEditInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditInfo(QWidget *parent = nullptr);
    ~DialogEditInfo();

    QString getText();
    void setText(QString html);

private:
    Ui::DialogEditInfo *ui;
};

#endif // DIALOGEDITINFO_H
