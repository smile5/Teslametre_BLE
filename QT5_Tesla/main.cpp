#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qputenv("QT_LOGGING_TO_CONSOLE",QByteArray("1"));
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
