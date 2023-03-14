#include "DialogEditInfo.h"
#include "ui_DialogEditInfo.h"

DialogEditInfo::DialogEditInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditInfo)
{
    ui->setupUi(this);
}

DialogEditInfo::~DialogEditInfo()
{
    delete ui;
}

QString DialogEditInfo::getText()
{
    return ui->widgetEditor->getHtml();
}

void DialogEditInfo::setText(QString html)
{
    ui->widgetEditor->setHtml(html);
}
