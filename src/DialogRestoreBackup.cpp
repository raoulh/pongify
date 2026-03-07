#include "DialogRestoreBackup.h"
#include "ui_DialogRestoreBackup.h"
#include "TStorage.h"
#include <QFileInfo>
#include <QMessageBox>

DialogRestoreBackup::DialogRestoreBackup(const QString &uuid, const QString &tournamentName, QWidget *parent):
    QDialog(parent),
    ui(new Ui::DialogRestoreBackup),
    m_uuid(uuid)
{
    ui->setupUi(this);
    setWindowTitle(tr("Restaurer un backup - %1").arg(tournamentName));

    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setHorizontalHeaderLabels({tr("Version"), tr("Date/Heure"), tr("Taille")});
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    auto backups = TStorage::Instance()->listBackups(uuid);

    if (backups.isEmpty())
    {
        ui->labelInfo->setText(tr("Aucun backup disponible pour ce tournoi."));
        ui->btnRestore->setEnabled(false);
        return;
    }

    ui->labelInfo->setText(tr("%1 backup(s) disponible(s). Sélectionnez un backup à restaurer.").arg(backups.size()));

    ui->tableWidget->setRowCount(backups.size());
    for (int i = 0; i < backups.size(); i++)
    {
        const auto &fi = backups[i];
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(fi.lastModified().toString("dd/MM/yyyy HH:mm:ss")));

        qint64 size = fi.size();
        QString sizeStr;
        if (size < 1024)
            sizeStr = QStringLiteral("%1 o").arg(size);
        else
            sizeStr = QStringLiteral("%1 Ko").arg(size / 1024);
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(sizeStr));
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->selectRow(0);

    connect(ui->btnRestore, &QPushButton::clicked, this, [this]()
    {
        auto selected = ui->tableWidget->selectionModel()->selectedRows();
        if (selected.isEmpty())
            return;

        m_selectedIndex = selected.first().row();

        auto ret = QMessageBox::question(
            this,
            tr("Confirmation"),
            tr("Restaurer le backup version %1 ?\n\n"
               "Le tournoi actuel sera sauvegardé comme backup avant la restauration.")
            .arg(m_selectedIndex));

        if (ret == QMessageBox::Yes)
            accept();
    });

    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

DialogRestoreBackup::~DialogRestoreBackup()
{
    delete ui;
}

int DialogRestoreBackup::selectedBackupIndex() const
{
    return m_selectedIndex;
}
