#include "DialogNewSerie.h"
#include "ui_DialogNewSerie.h"
#include <QTime>

DialogNewSerie::DialogNewSerie(bool isnew, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewSerie)
{
    ui->setupUi(this);

    if (isnew)
        setWindowTitle("Nouvelle série");
    else
        setWindowTitle("Modifier");

    ui->comboBoxType->addItem("Elimination directe");
    ui->comboBoxType->addItem("A la ronde (Round-Robbin)");

    ui->comboBoxRanking->addItem("Open");
    ui->comboBoxRanking->addItem("NC");
    for (int i = 90; i > 0;i -= 5)
        ui->comboBoxRanking->addItem(QString::number(i));

    if (!isnew)
        ui->checkBoxDouble->setDisabled(true);

    ui->timeEditStart->setDisplayFormat("HH:mm");
    ui->timeEditStart->setEnabled(false);
    ui->timeEditStart->setTime(QTime(9, 0));
    connect(ui->checkStartTime, &QCheckBox::toggled, ui->timeEditStart, &QTimeEdit::setEnabled);
}

DialogNewSerie::~DialogNewSerie()
{
    delete ui;
}

void DialogNewSerie::setName(QString name)
{
    ui->lineEditName->setText(name);
}

QString DialogNewSerie::getName()
{
    return ui->lineEditName->text();
}

QString DialogNewSerie::getRanking()
{
    return ui->comboBoxRanking->currentText();
}

void DialogNewSerie::setRanking(QString r)
{
    r = r.trimmed();
    for (int i = 0;i < ui->comboBoxRanking->count();i++)
    {
        if (ui->comboBoxRanking->itemText(i).toLower() == r.toLower())
        {
            ui->comboBoxRanking->setCurrentIndex(i);
            break;
        }
    }
}

QString DialogNewSerie::getType()
{
    if (ui->comboBoxType->currentIndex() == 0)
        return {"single"};

    return {"roundrobin"};
}

void DialogNewSerie::setType(QString t)
{
    t = t.trimmed();
    if (t.toLower() == "single")
        ui->comboBoxType->setCurrentIndex(0);
    else if (t.toLower() == "roundrobin")
        ui->comboBoxType->setCurrentIndex(1);
}

bool DialogNewSerie::getDouble()
{
    return ui->checkBoxDouble->isChecked();
}

void DialogNewSerie::setDouble(bool en)
{
    ui->checkBoxDouble->setChecked(en);
}

bool DialogNewSerie::getHandicap()
{
    if (getDouble())
        return false;
    return ui->checkBoxHandicap->isChecked();
}

void DialogNewSerie::setHandicap(bool en)
{
    if (getDouble())
        return;
    ui->checkBoxHandicap->setChecked(en);
}

QString DialogNewSerie::getStartTime() const
{
    if (!ui->checkStartTime->isChecked())
        return {};
    return ui->timeEditStart->time().toString("HH:mm");
}

void DialogNewSerie::setStartTime(const QString &time)
{
    if (time.isEmpty())
    {
        ui->checkStartTime->setChecked(false);
    }
    else
    {
        ui->checkStartTime->setChecked(true);
        ui->timeEditStart->setTime(QTime::fromString(time, "HH:mm"));
    }
}
