#include "MainWindow.h"
#include "Utils.h"

#include <QDir>
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("raoulhecky");
    QCoreApplication::setOrganizationDomain("raoulhecky.com");
    QCoreApplication::setApplicationName("Pongify");

    //Force create the required user data path
    {
        QDir dir(Utils::getCachePath());
        if (!dir.exists())
            dir.mkpath(".");
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
