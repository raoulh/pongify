#include "Utils.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QStandardPaths>

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
