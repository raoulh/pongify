#include "PlayerSync.h"
#include "Utils.h"
#include "PlayerModel.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QEventLoop>
#include <QMessageBox>

PlayerSync::PlayerSync(QObject *parent)
    : QObject{parent}
{
}

PlayerSync::~PlayerSync()
{
    delete progressDlg;
}

void PlayerSync::start()
{
    if (cdslsProc) return;

    progressDlg = new QProgressDialog("Synchronisation des joueurs depuis le site du CDSLS", "Annuler", 0, 100);
    connect(progressDlg, &QProgressDialog::canceled, this, &PlayerSync::cancel);
    progressDlg->setWindowModality(Qt::ApplicationModal);

    cdslsProc = new QProcess(this);
    connect(cdslsProc, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus exitStatus)
    {
        if (exitStatus != QProcess::NormalExit)
            qWarning() << "Process exits with exit code" << exitCode << "Exit Status:" << exitStatus;
        else
            PlayerModel::Instance()->saveCache();

        eventLoop->quit();
    });

    connect(cdslsProc, &QProcess::readyReadStandardOutput, this, &PlayerSync::readStdOut);

    QString program = QStringLiteral("%1/cdsls_scrapper.exe").arg(Utils::getBinPath());

    cdslsProc->setReadChannel(QProcess::StandardOutput);
    cdslsProc->start(program);

    eventLoop = new QEventLoop(this);
    eventLoop->exec();
}

void PlayerSync::cancel()
{
    eventLoop->quit();
    delete cdslsProc;
    cdslsProc = nullptr;
}

void PlayerSync::readStdOut()
{
    while (cdslsProc->canReadLine())
    {
        QString line = cdslsProc->readLine();
        QJsonParseError err;
        QJsonDocument jdoc = QJsonDocument::fromJson(line.toUtf8(), &err);

        if (err.error != QJsonParseError::NoError)
        {
            qWarning() << "JSON parse error " << err.errorString();
            qWarning() << "Line is: " << line;
            return;
        }

        if (!jdoc.isObject())
            continue;

        auto obj = jdoc.object();

        if (obj.contains("progress"))
        {
            progressDlg->setMaximum(obj["total"].toInt());
            progressDlg->setValue(obj["progress"].toInt());
        }
        else if (obj.contains("error"))
        {
            QMessageBox::warning(nullptr, "Erreur", "Une erreur est survenue.\n\n" + obj["message"].toString());
        }
        else if (obj.contains("lastname"))
        {
            qDebug() << "Loading new player: " << obj["firstname"].toString() << " " << obj["lastname"].toString();
            PlayerModel::Instance()->loadPlayer(obj);
        }
    }
}

