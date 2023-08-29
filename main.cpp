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



    // 获取屏幕的尺寸
    QDesktopWidget *desktop = QApplication::desktop();
    QRect screenGeometry = desktop->screenGeometry();
    QSize screenSize = screenGeometry.size();

    // 设置窗口的固定尺寸为屏幕尺寸
    w.setFixedSize(screenSize);

    // 将窗口显示为最大化状态
    w.showMaximized();


    gSystem=new HElcSignage();




    w.ConnectSlots();

    return a.exec();
}
