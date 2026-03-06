#ifndef DIALOGTOURNAMENTQR_H
#define DIALOGTOURNAMENTQR_H

#include <QDialog>
#include <QImage>

class DialogTournamentQr : public QDialog
{
    Q_OBJECT
public:
    explicit DialogTournamentQr(const QString &tournamentName,
                                const QString &tournamentDate,
                                const QString &publicUrl,
                                const QImage &qrImage,
                                QWidget *parent = nullptr);

private slots:
    void copyUrl();
    void printQrCode();

private:
    QString m_tournamentName;
    QString m_tournamentDate;
    QString m_publicUrl;
    QImage m_qrImage;
};

#endif // DIALOGTOURNAMENTQR_H
