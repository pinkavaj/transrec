#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("TransRec");
    a.setOrganizationName("Pinky");
    a.setOrganizationDomain("web.vscht.cz/pinkavaj");
    MainWindow w;
    w.show();

    return a.exec();
}
