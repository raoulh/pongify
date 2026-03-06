#include "DialogCloudflareSetup.h"
#include "ui_DialogCloudflareSetup.h"
#include "CloudflareSetup.h"

DialogCloudflareSetup::DialogCloudflareSetup(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::DialogCloudflareSetup),
      m_setup(new CloudflareSetup(this))
{
    ui->setupUi(this);
    setWindowTitle(tr("Configuration Cloudflare"));

    ui->progressBar->setValue(0);
    ui->textLog->clear();

    connect(ui->btnDeploy, &QPushButton::clicked, this, &DialogCloudflareSetup::onDeploy);
    connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::reject);

    connect(m_setup, &CloudflareSetup::stepChanged, this,
            [this](int step, int /*totalSteps*/, const QString &description) {
        ui->progressBar->setValue(step);
        appendLog(QString::fromUtf8("\u2714 ") + description);
    });

    connect(m_setup, &CloudflareSetup::errorOccurred, this, [this](const QString &message) {
        appendLog(QString::fromUtf8("\u2718 Erreur: ") + message);
    });

    connect(m_setup, &CloudflareSetup::finished, this, [this](bool success) {
        m_success = success;
        setRunning(false);

        if (success) {
            appendLog("");
            appendLog(tr("============================================"));
            appendLog(QString::fromUtf8("\u2714 ") + tr("Worker déployé avec succès !"));
            appendLog(tr("============================================"));
            appendLog(tr("URL: %1").arg(m_setup->workerUrl()));
            appendLog(tr("Secret admin: %1").arg(m_setup->adminSecret()));
            appendLog("");
            appendLog(tr("Cliquez sur Fermer pour appliquer automatiquement la configuration."));

            // Change close button to accept
            disconnect(ui->btnClose, nullptr, this, nullptr);
            connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::accept);
            ui->btnClose->setText(tr("Appliquer et fermer"));
        }
    });
}

DialogCloudflareSetup::~DialogCloudflareSetup()
{
    delete ui;
}

QString DialogCloudflareSetup::workerUrl() const
{
    return m_setup->workerUrl();
}

QString DialogCloudflareSetup::adminSecret() const
{
    return m_setup->adminSecret();
}

void DialogCloudflareSetup::onDeploy()
{
    QString token = ui->editToken->text().trimmed();
    if (token.isEmpty()) {
        appendLog(tr("Veuillez saisir un token API Cloudflare."));
        return;
    }

    ui->textLog->clear();
    ui->progressBar->setValue(0);
    m_success = false;
    setRunning(true);

    appendLog(tr("Démarrage du déploiement..."));
    m_setup->start(token);
}

void DialogCloudflareSetup::appendLog(const QString &text)
{
    ui->textLog->append(text);
}

void DialogCloudflareSetup::setRunning(bool running)
{
    ui->btnDeploy->setEnabled(!running);
    ui->editToken->setEnabled(!running);
    if (running)
        ui->btnClose->setEnabled(false);
    else
        ui->btnClose->setEnabled(true);
}
