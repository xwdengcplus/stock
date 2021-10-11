#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qputenv("QT_MAC_WANTS_LAYER", "1");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
