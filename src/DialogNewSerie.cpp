#include "DialogNewSerie.h"
#include "ui_DialogNewSerie.h"

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
