#include "mainwindow.h"
#include <QApplication>
#include <csignal>

void signalHandler(int)
{
   QCoreApplication::instance()->quit();
}

int main(int argc, char *argv[])
{
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

#ifndef Q_OS_WIN
    signal(SIGKILL, signalHandler);
    signal(SIGQUIT, signalHandler);
#endif // Q_OS_WIN


    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(QObject::tr("Connection Tester (v.1.1.0)"));
    w.show();

    return a.exec();
}
