#ifndef DIALOGNEWSERIE_H
#define DIALOGNEWSERIE_H

#include <QDialog>

namespace Ui {
class DialogNewSerie;
}

class DialogNewSerie : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewSerie(bool isnew, QWidget *parent = nullptr);
    ~DialogNewSerie();

    void setName(QString name);
    QString getName();

    QString getRanking();
    void setRanking(QString r);
    QString getType();
    void setType(QString t);
    bool getDouble();
    void setDouble(bool en);
    bool getHandicap();
    void setHandicap(bool en);
    QString getStartTime() const;
    void setStartTime(const QString &time);

private:
    Ui::DialogNewSerie *ui;
};

#endif // DIALOGNEWSERIE_H
