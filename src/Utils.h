#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QFile>
#include <QMutex>

class Utils
{
public:
    static Utils &Instance()
    {
        static Utils u;
        return u;
    }

    static QString getBinPath();
    static QString getCachePath();

    static void rotateLogFile(QString logfile);

    void installMessageOutputHandler();
    void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    Utils();

    QFile debugLogFile;
    QMutex logMutex;
};

#endif // UTILS_H
