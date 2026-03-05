#include "DialogNewTournament.h"
#include "ui_DialogNewTournament.h"

DialogNewTournament::DialogNewTournament(bool newTournament, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewTournament)
{
    ui->setupUi(this);
    if (newTournament)
        setWindowTitle("Nouveau");
    else
        setWindowTitle("Modifier");

    ui->dateEdit->setDate(QDate::currentDate());
    ui->spinBoxTime->setValue(15000);
    ui->spinBoxTime->setMinimum(2000);
    ui->spinBoxTime->setMaximum(99999999);

    ui->spinBoxScrollSpeed->setValue(80);
    ui->spinBoxScrollSpeed->setMinimum(20);
    ui->spinBoxScrollSpeed->setMaximum(500);
    ui->spinBoxScrollSpeed->setSuffix(" px/s");
}

DialogNewTournament::~DialogNewTournament()
{
    delete ui;
}

void DialogNewTournament::setName(QString name)
{
    ui->lineEditName->setText(name);
}

QString DialogNewTournament::getName()
{
    return ui->lineEditName->text();
}

void DialogNewTournament::setDate(QDate date)
{
    ui->dateEdit->setDate(date);
}

QDate DialogNewTournament::getDate()
{
    return ui->dateEdit->date();
}

void DialogNewTournament::setTime(int time)
{
    ui->spinBoxTime->setValue(time);
}

int DialogNewTournament::getTime()
{
    return ui->spinBoxTime->value();
}

void DialogNewTournament::setScrollSpeed(int speed)
{
    ui->spinBoxScrollSpeed->setValue(speed);
}

int DialogNewTournament::getScrollSpeed()
{
    return ui->spinBoxScrollSpeed->value();
}
