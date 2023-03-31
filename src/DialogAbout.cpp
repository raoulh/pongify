#include "DialogAbout.h"
#include "ui_DialogAbout.h"
#include "../version.h"

DialogAbout::DialogAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAbout)
{
    ui->setupUi(this);

    ui->labelVersion->setText(pongify_version);
}

DialogAbout::~DialogAbout()
{
    delete ui;
}
