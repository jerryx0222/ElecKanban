#include "mainwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include "helcsignage.h"


HElcSignage* gSystem=nullptr;
QTranslator* gTranslator=nullptr;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    gTranslator = new QTranslator;
    gTranslator->load("lang_zh_tw.qm");
    a.installTranslator(gTranslator);


    MainWindow w;


    QDesktopWidget *desktop = QApplication::desktop();
    QRect screenGeometry = desktop->screenGeometry();
    QSize screenSize = screenGeometry.size();

    w.setFixedSize(screenSize);

    w.showMaximized();


    gSystem=new HElcSignage();




    w.ConnectSlots();

    return a.exec();
}
