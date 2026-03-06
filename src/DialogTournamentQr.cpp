#include "DialogTournamentQr.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGuiApplication>
#include <QClipboard>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>

DialogTournamentQr::DialogTournamentQr(const QString &tournamentName,
                                       const QString &tournamentDate,
                                       const QString &publicUrl,
                                       const QImage &qrImage,
                                       QWidget *parent)
    : QDialog(parent),
      m_tournamentName(tournamentName),
      m_tournamentDate(tournamentDate),
      m_publicUrl(publicUrl),
      m_qrImage(qrImage)
{
    setWindowTitle(tr("QR Code du tournoi"));
    setMinimumWidth(420);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    // Tournament name
    auto *nameLabel = new QLabel(m_tournamentName);
    QFont nameFont = nameLabel->font();
    nameFont.setPointSize(14);
    nameFont.setBold(true);
    nameLabel->setFont(nameFont);
    nameLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(nameLabel);

    // Tournament date
    auto *dateLabel = new QLabel(m_tournamentDate);
    dateLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(dateLabel);

    mainLayout->addSpacing(8);

    // QR code image
    auto *qrLabel = new QLabel;
    QPixmap pix = QPixmap::fromImage(m_qrImage).scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    qrLabel->setPixmap(pix);
    qrLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(qrLabel);

    // "Scannez pour suivre en live" text
    auto *scanLabel = new QLabel(tr("Scannez pour suivre en live"));
    QFont scanFont = scanLabel->font();
    scanFont.setPointSize(11);
    scanFont.setBold(true);
    scanLabel->setFont(scanFont);
    scanLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(scanLabel);

    mainLayout->addSpacing(8);

    // URL row: read-only line edit + copy button
    auto *urlLayout = new QHBoxLayout;
    auto *urlEdit = new QLineEdit(m_publicUrl);
    urlEdit->setReadOnly(true);
    urlLayout->addWidget(urlEdit);
    auto *copyBtn = new QPushButton(tr("Copier"));
    urlLayout->addWidget(copyBtn);
    mainLayout->addLayout(urlLayout);

    // Buttons row: print + close
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    auto *printBtn = new QPushButton(tr("Imprimer"));
    btnLayout->addWidget(printBtn);
    auto *closeBtn = new QPushButton(tr("Fermer"));
    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);

    connect(copyBtn, &QPushButton::clicked, this, &DialogTournamentQr::copyUrl);
    connect(printBtn, &QPushButton::clicked, this, &DialogTournamentQr::printQrCode);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void DialogTournamentQr::copyUrl()
{
    QGuiApplication::clipboard()->setText(m_publicUrl);
}

void DialogTournamentQr::printQrCode()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dlg(&printer, this);
    dlg.setWindowTitle(tr("Imprimer le QR Code"));
    if (dlg.exec() != QDialog::Accepted)
        return;

    QPainter painter(&printer);
    QRect pageRect = painter.viewport();

    int margin = pageRect.width() / 20;
    int contentWidth = pageRect.width() - 2 * margin;
    int yPos = margin;

    // Title
    QFont titleFont = painter.font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    QRect titleRect(margin, yPos, contentWidth, pageRect.height() / 10);
    painter.drawText(titleRect, Qt::AlignHCenter | Qt::AlignTop, m_tournamentName);
    yPos += painter.fontMetrics().boundingRect(titleRect, Qt::AlignHCenter | Qt::AlignTop, m_tournamentName).height() + margin / 2;

    // Date
    QFont dateFont = painter.font();
    dateFont.setPointSize(18);
    dateFont.setBold(false);
    painter.setFont(dateFont);
    QRect dateRect(margin, yPos, contentWidth, pageRect.height() / 15);
    painter.drawText(dateRect, Qt::AlignHCenter | Qt::AlignTop, m_tournamentDate);
    yPos += painter.fontMetrics().boundingRect(dateRect, Qt::AlignHCenter | Qt::AlignTop, m_tournamentDate).height() + margin;

    // QR code — 60% of page width, centered
    int qrSize = contentWidth * 60 / 100;
    int qrX = margin + (contentWidth - qrSize) / 2;
    QImage scaledQr = m_qrImage.scaled(qrSize, qrSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter.drawImage(qrX, yPos, scaledQr);
    yPos += qrSize + margin / 2;

    // "Scannez pour suivre en live"
    QFont scanFont = painter.font();
    scanFont.setPointSize(22);
    scanFont.setBold(true);
    painter.setFont(scanFont);
    QRect scanRect(margin, yPos, contentWidth, pageRect.height() / 12);
    painter.drawText(scanRect, Qt::AlignHCenter | Qt::AlignTop, tr("Scannez pour suivre en live"));

    painter.end();
}
