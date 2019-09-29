#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    if( translator.load("ptc_zn_CN.qm",":/translations") ) {
        a.installTranslator(&translator);
        qDebug() << "load translations";
    } else {
        qDebug() << "can not load translations";
    }
    MainWindow w;
    w.show();

    return a.exec();
}
