#include "DialogBroadcastOpts.h"
#include "ui_DialogBroadcastOpts.h"
#include <QScreen>

DialogBroadcastOpts::DialogBroadcastOpts(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBroadcastOpts)
{
    ui->setupUi(this);

    for (const auto &screen: qApp->screens())
    {
        auto it = new QListWidgetItem(ui->listWidgetScreens);
        it->setText(QStringLiteral("%1 [%2x%3]")
                    .arg(screen->name())
                    .arg(screen->availableGeometry().width())
                    .arg(screen->availableGeometry().height()));
        it->setData(Qt::UserRole, QVariant::fromValue(screen));
    }
}

DialogBroadcastOpts::~DialogBroadcastOpts()
{
    delete ui;
}

QScreen *DialogBroadcastOpts::getScreen()
{
    if (!ui->listWidgetScreens->currentItem())
        return nullptr;

    return ui->listWidgetScreens->currentItem()->data(Qt::UserRole).value<QScreen *>();
}

bool DialogBroadcastOpts::getFullscreen()
{
    return !ui->checkBoxWindow->isChecked();
}

void DialogBroadcastOpts::on_buttonBox_accepted()
{
    if (!ui->listWidgetScreens->selectedItems().isEmpty())
        accept();
}

