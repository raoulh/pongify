#include "DialogWebPublish.h"
#include "ui_DialogWebPublish.h"
#include "WebPublisher.h"
#include "DialogCloudflareSetup.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>

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

    connect(ui->btnShowSecret, &QPushButton::clicked, this, [this]() {
        if (ui->editAdminSecret->echoMode() == QLineEdit::Password) {
            ui->editAdminSecret->setEchoMode(QLineEdit::Normal);
            ui->btnShowSecret->setText(tr("Masquer"));
        } else {
            ui->editAdminSecret->setEchoMode(QLineEdit::Password);
            ui->btnShowSecret->setText(tr("Afficher"));
        }
    });

    connect(ui->btnCopySecret, &QPushButton::clicked, this, [this]() {
        QApplication::clipboard()->setText(ui->editAdminSecret->text());
        ui->labelStatus->setText(tr("\u2714 Secret copié dans le presse-papier"));
    });

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
    QString savedSecret = wp->adminSecret();
    wp->setWorkerUrl(ui->editWorkerUrl->text().trimmed());
    wp->setAdminSecret(ui->editAdminSecret->text().trimmed());

    bool ok = wp->testConnection();
    if (!ok) {
        ui->labelStatus->setText(tr("\u2718 Erreur de connexion"));
    } else {
        QString authResult = wp->testAuth();
        if (authResult.isEmpty())
            ui->labelStatus->setText(tr("\u2714 Connexion OK, secret admin valid\u00e9"));
        else
            ui->labelStatus->setText(tr("\u2714 Connexion OK, mais secret admin invalide: %1").arg(authResult));
    }

    wp->setWorkerUrl(savedUrl);
    wp->setAdminSecret(savedSecret);
}
