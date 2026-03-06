#include "DialogWebPublish.h"
#include "ui_DialogWebPublish.h"
#include "WebPublisher.h"
#include "DialogCloudflareSetup.h"

DialogWebPublish::DialogWebPublish(QWidget *parent):
    QDialog(parent),
    ui(new Ui::DialogWebPublish)
{
    ui->setupUi(this);
    setWindowTitle(tr("Configuration publication web"));

    auto wp = WebPublisher::Instance();
    ui->editWorkerUrl->setText(wp->workerUrl());
    ui->editAdminSecret->setText(wp->adminSecret());
    ui->editAdminSecret->setEchoMode(QLineEdit::Password);
    ui->checkEnabled->setChecked(wp->enabled());
    ui->labelStatus->clear();

    connect(ui->btnTest, &QPushButton::clicked, this, &DialogWebPublish::testConnection);
    connect(ui->btnCloudflareSetup, &QPushButton::clicked, this, &DialogWebPublish::openCloudflareSetup);

    connect(this, &QDialog::accepted, this, [this]() {
        auto wp = WebPublisher::Instance();
        wp->setWorkerUrl(ui->editWorkerUrl->text().trimmed());
        wp->setAdminSecret(ui->editAdminSecret->text().trimmed());
        wp->setEnabled(ui->checkEnabled->isChecked());
        wp->saveSettings();
    });
}

DialogWebPublish::~DialogWebPublish()
{
    delete ui;
}

QString DialogWebPublish::getWorkerUrl() const
{
    return ui->editWorkerUrl->text().trimmed();
}

QString DialogWebPublish::getAdminSecret() const
{
    return ui->editAdminSecret->text().trimmed();
}

bool DialogWebPublish::getEnabled() const
{
    return ui->checkEnabled->isChecked();
}

void DialogWebPublish::openCloudflareSetup()
{
    DialogCloudflareSetup dlg(this);
    if (dlg.exec() == QDialog::Accepted && dlg.setupSucceeded()) {
        ui->editWorkerUrl->setText(dlg.workerUrl());
        ui->editAdminSecret->setText(dlg.adminSecret());
        ui->checkEnabled->setChecked(true);
        ui->labelStatus->setText(tr("\u2714 Configuration Cloudflare appliquée"));
    }
}

void DialogWebPublish::testConnection()
{
    ui->labelStatus->setText(tr("Test en cours..."));

    auto wp = WebPublisher::Instance();
    QString savedUrl = wp->workerUrl();
    wp->setWorkerUrl(ui->editWorkerUrl->text().trimmed());

    bool ok = wp->testConnection();

    wp->setWorkerUrl(savedUrl);

    if (ok)
        ui->labelStatus->setText(tr("✓ Connexion réussie"));
    else
        ui->labelStatus->setText(tr("✗ Erreur de connexion"));
}
