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
