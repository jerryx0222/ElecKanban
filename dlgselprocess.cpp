#include "dlgselprocess.h"
#include "ui_dlgselprocess.h"
#include "helcsignage.h"
#include <QFileDialog>
#include <QMessageBox>

extern HElcSignage* gSystem;

dlgSelProcess::dlgSelProcess(QString* pID,QString* pName,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgSelProcess)
{
    ui->setupUi(this);

    m_pStrSelID=pID;
    m_pStrSelName=pName;
    ui->edtProcess->setText("*");
    strProcessID="";

    CreateTable();
    RelistProcess();
}

dlgSelProcess::~dlgSelProcess()
{
    delete ui;
}

void dlgSelProcess::CreateTable()
{
    int ColCount=2;
    QStringList names;
    names.push_back(tr("ID"));
    names.push_back(tr("Name"));
    ui->tbProcess->setColumnCount(ColCount);
    ui->tbProcess->setHorizontalHeaderLabels(names);
    ui->tbProcess->setColumnWidth(0,210);
    ui->tbProcess->setColumnWidth(1,400);
}


void dlgSelProcess::on_buttonBox_accepted()
{
    int sel=ui->tbProcess->currentRow();
    QTableWidgetItem* pItem1=ui->tbProcess->item(sel,0);
    QTableWidgetItem* pItem2=ui->tbProcess->item(sel,1);
    if(pItem1!=nullptr && pItem2!=nullptr)
    {
        *m_pStrSelID=pItem1->text();
        *m_pStrSelName=pItem2->text();
        strProcessID=QString("%1:%2").arg(pItem1->text()).arg(pItem2->text());
    }
    else
        strProcessID="";
}

void dlgSelProcess::on_btnSearch_clicked()
{
    RelistProcess(ui->edtProcess->text());

}



void dlgSelProcess::RelistProcess(QString strKey)
{
    while(ui->tbProcess->rowCount()>0)
        ui->tbProcess->removeRow(0);

    std::map<QString,QString>::iterator itMap;
    int nRow;
    QTableWidgetItem* pItem;
    if(strKey=="*" || strKey.size()<=0)
    {
        for(itMap=m_mapProcess.begin();itMap!=m_mapProcess.end();itMap++)
        {
            nRow=ui->tbProcess->rowCount();
            ui->tbProcess->insertRow(nRow);
            pItem=new QTableWidgetItem(itMap->first);
            ui->tbProcess->setItem(nRow,0,pItem);
            //pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

            pItem=new QTableWidgetItem(itMap->second);
            ui->tbProcess->setItem(nRow,1,pItem);
            //pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
        }
    }
    else
    {
        for(itMap=m_mapProcess.begin();itMap!=m_mapProcess.end();itMap++)
        {
            if(itMap->first.indexOf(strKey)>=0)
            {
                nRow=ui->tbProcess->rowCount();
                ui->tbProcess->insertRow(nRow);

                pItem=new QTableWidgetItem(itMap->first);
                ui->tbProcess->setItem(nRow,0,pItem);
                //pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

                pItem=new QTableWidgetItem(itMap->second);
                ui->tbProcess->setItem(nRow,1,pItem);
                //pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
            }
        }
    }

    ui->tbProcess->resizeRowsToContents();
    ui->tbProcess->resizeColumnsToContents();
}

void dlgSelProcess::RelistProcess()
{
    int nRow;
    QTableWidgetItem* pItem;
    std::map<QString,QString>::iterator itMap;

    while(ui->tbProcess->rowCount()>0)
        ui->tbProcess->removeRow(0);
    if(gSystem==nullptr) return;


    m_mapProcess.clear();
    gSystem->CopyProcess(m_mapProcess);


    for(itMap=m_mapProcess.begin();itMap!=m_mapProcess.end();itMap++)
    {
        nRow=ui->tbProcess->rowCount();
        ui->tbProcess->insertRow(nRow);
        pItem=new QTableWidgetItem(itMap->first);
        ui->tbProcess->setItem(nRow,0,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        pItem=new QTableWidgetItem(itMap->second);
        ui->tbProcess->setItem(nRow,1,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);


    }

    ui->tbProcess->resizeRowsToContents();
    ui->tbProcess->resizeColumnsToContents();
}


void dlgSelProcess::on_btnSave_clicked()
{
    QString strID=ui->edtID->text();
    QString strName=ui->edtName->text();
    gSystem->SaveProcess(strID,strName);
    RelistProcess();
    /*
    int nRow=ui->tbProcess->currentRow();
    QTableWidgetItem* pItem=ui->tbProcess->item(nRow,0);
    if(pItem==nullptr)
        return;
    QString strID=pItem->text();

    pItem=ui->tbProcess->item(nRow,1);
    if(pItem==nullptr)
        return;
    QString strName=pItem->text();
    gSystem->SaveProcess(strID,strName);
    RelistProcess();
    */

}

void dlgSelProcess::on_tbProcess_cellDoubleClicked(int row, int column)
{
    if(column!=0)
        return;

    QTableWidgetItem* pItem1=ui->tbProcess->item(row,0);
    QTableWidgetItem* pItem2=ui->tbProcess->item(row,1);
    if(pItem1!=nullptr && pItem2!=nullptr)
    {
        *m_pStrSelID=pItem1->text();
        *m_pStrSelName=pItem2->text();
        emit close();
    }
}

void dlgSelProcess::on_tbProcess_cellClicked(int row, int column)
{
    QTableWidgetItem* pItem1=ui->tbProcess->item(row,0);
    QTableWidgetItem* pItem2=ui->tbProcess->item(row,1);
    if(pItem1!=nullptr && pItem2!=nullptr)
    {
       ui->edtID->setText(pItem1->text());
       ui->edtName->setText(pItem2->text());
    }
}

void dlgSelProcess::on_btnImport_clicked()
{

    QString strPath="D:/";
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        strPath,
        tr("Excel Files (*.XLSX *.xlsx);;All Files (*)"));
    if (fileName.isEmpty())
        return;


    if(!gSystem->LoadProcessIDFromFile(fileName))
        QMessageBox::warning(nullptr,tr("Warm"),tr("Import Failed!"));

}
