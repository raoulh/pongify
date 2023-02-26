#include "Utils.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QStandardPaths>
#include <QDateTime>

QString Utils::getBinPath()
{
    //path cache
    static QString staticPath;

    if (!staticPath.isEmpty())
        return staticPath;

    staticPath = QCoreApplication::applicationDirPath();
    QStringList cargs = QApplication::arguments();

    int idx = cargs.indexOf("-bin_path");
    if (idx >= 0 &&
        idx + 1 < cargs.count())
    {
        if (QFile::exists(cargs[idx + 1]))
        {
            staticPath = cargs[idx + 1];
            staticPath.replace('\\', '/');
            if (staticPath.endsWith('/'))
                staticPath.remove(staticPath.length() - 1, 1);
        }
    }

    return staticPath;
}

QString Utils::getCachePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

Utils::Utils()
{
    if (!debugLogFile.isOpen())
    {
        auto logfile = QStringLiteral("%1/lastrun.log")
                .arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));

        //Log rotation
        rotateLogFile(logfile);

        debugLogFile.setFileName(logfile);
        debugLogFile.open(QFile::ReadWrite | QFile::Truncate);
    }
}

void Utils::rotateLogFile(QString logfile)
{
    //keep maximum of 10 log files
    const int maxKept = 10;

    for (int i = maxKept - 2;i >= 0;i--)
    {
        auto fOld = logfile;
        auto fNew = logfile;
        if (i > 0)
            fOld.replace(".log", QStringLiteral(".%1.log").arg(i));
        fNew.replace(".log", QStringLiteral(".%1.log").arg(i + 1));
        if (QFile::exists(fOld))
        {
            QFile::remove(fNew);
            QFile::copy(fOld, fNew);
        }
    }
}

void Utils::messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker lockerFile(&logMutex);

    QString fname = context.file;
    fname = fname.section('\\', -1, -1);
    if (fname == context.file)
        fname = fname.section('/', -1, -1);

    QString mtype;

    switch (type) {
    default:
    case QtDebugMsg: mtype = "D"; break;
    case QtInfoMsg: mtype = "I"; break;
    case QtWarningMsg: mtype = "W"; break;
    case QtCriticalMsg: mtype = "C"; break;
    case QtFatalMsg: mtype = "F"; break;
    }

    QDateTime dt = QDateTime::currentDateTime();

    QString s = QString("%1 %2 [%3:%4] %5\n").arg(mtype, dt.toString("hh:mm:ss.z"), fname, QString::number(context.line), msg);

    printf("%s", qPrintable(s));
    fflush(stdout);

    if (debugLogFile.isOpen())
    {
        debugLogFile.write(s.toLocal8Bit());
        debugLogFile.flush();
    }
}

static void _messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Utils::Instance().messageOutput(type, context, msg);
}

void Utils::installMessageOutputHandler()
{
    qInstallMessageHandler(_messageOutput);
}
