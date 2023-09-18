#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTabWidget>
#include "vdatabase.h"
#include "vshipment.h"
#include "vpartprocess.h"
#include "vbackcalinfo.h"
#include "vwipplot.h"
#include "vwipplotex.h"
#include "htabbase.h"
#include "vwebservice.h"
#include "vbacktable.h"
#include "voutputerex.h"
#include "helcsignage.h"

extern bool gLogin;
extern HElcSignage* gSystem;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //QSize size(1920,1000);
    QSize size(1200,768);

    m_pTabMain = new QTabWidget(this);
    m_pTabMain->setMaximumSize(size);
    m_pTabMain->setMinimumSize(size);

    VBackTable* pBack=new VBackTable(this);
    m_pTabMain->addTab(pBack,  tr("CallBackInfor"));
    //m_pTabMain->addTab(new vWipPlot(this),      tr("WipPlotInfor"));
    m_pTabMain->addTab(new vWipPlotEx(this),      tr("WipPlotInfor"));
    m_pTabMain->addTab(new vOutputerEx(this),     tr("OutputerInfor"));


    vShipment* pShip=new vShipment(this);
    m_pTabMain->addTab(pShip,     tr("Shipment"));
    pShip->setEnabled(false);
    vPartProcess* pPart=new vPartProcess(this);
    m_pTabMain->addTab(pPart,  tr("Process"));
    pPart->setEnabled(false);

    VWebService* pWeb=new VWebService(this);
    m_pTabMain->addTab(pWeb,     tr("WebService"));


    m_Index=0;
    QObject::connect(m_pTabMain, &QTabWidget::currentChanged,this,&MainWindow::OnTableChange);
    QObject::connect(pWeb, SIGNAL(OnUserLogin(bool)),this,SLOT(OnUserLogin(bool)));



    OnError("");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ConnectSlots()
{
    connect(gSystem,SIGNAL(OnError(QString)),this,SLOT(OnError(QString)));
}




void MainWindow::OnTableChange()
{
    HTabBase* pOld = static_cast<HTabBase*>(m_pTabMain->widget(m_Index));
    if(pOld!=nullptr)
        pOld->OnShowTab(false);
    m_Index=m_pTabMain->currentIndex();
    pOld = static_cast<HTabBase*>(m_pTabMain->widget(m_Index));
    if(pOld!=nullptr)
        pOld->OnShowTab(true);
}

void MainWindow::OnUserLogin(bool ok)
{

    HTabBase* pTab;
    QString strTitle;
    int nCount=m_pTabMain->count();
    for(int i=0;i<nCount;i++)
    {
        pTab=static_cast<HTabBase*>(m_pTabMain->widget(i));
        strTitle=m_pTabMain->tabText(i);
        if(strTitle==tr("Shipment"))
            pTab->setEnabled(ok);
        else if(strTitle==tr("Process"))
            pTab->setEnabled(ok);
    }

    gLogin=ok;

}

void MainWindow::OnError(QString strErr)
{
    QString strVer="20230918";
    if(strErr.size()<=0)
        setWindowTitle(QString("%1_%2").arg("Electronic Kanban").arg(strVer));
    else
        setWindowTitle(QString("%1_%2 :Error-%3").arg("Electronic Kanban").arg(strVer).arg(strErr));
}

