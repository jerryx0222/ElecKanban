#include "dlgtypeselect.h"
#include "ui_dlgtypeselect.h"
#include "helcsignage.h"
#include <QFileDialog>
#include <QScrollBar>
#include <QMessageBox>

extern HElcSignage* gSystem;

dlgTypeSelect::dlgTypeSelect(QString* pTypeSelect,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgTypeSelect)
{
    ui->setupUi(this);

    m_pStrTypeSelect=pTypeSelect;
    connect(&m_timer,&QTimer::timeout,this,&dlgTypeSelect::OnInit);
    m_timer.start(100);
}

dlgTypeSelect::~dlgTypeSelect()
{
    delete ui;
}

void dlgTypeSelect::OnInit()
{
    if(gSystem==nullptr)
        return;

    m_timer.stop();

    ui->tbType->setColumnCount(2);
    ui->tbType->verticalHeader()->setVisible(false);
    QStringList strTitles;
    strTitles.push_back(tr("type"));
    strTitles.push_back(tr("name"));

    ui->tbType->setColumnWidth(0,150);
    ui->tbType->setColumnWidth(1,300);
    ui->tbType->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->tbType->setHorizontalHeaderLabels(strTitles);


    ReloadTypes();

}

void dlgTypeSelect::on_btnHistoryLoad_2_clicked()
{
    QString strPath="D:/";
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        strPath,
        tr("Excel Files (*.XLSX *.xlsx);;All Files (*)"));
    if (fileName.isEmpty())
        return;


    gSystem->ImportTypeFile(fileName);
}

void dlgTypeSelect::on_btnNew_clicked()
{
    int nRow=ui->tbType->rowCount();
    if(nRow!=static_cast<int>(m_datas.size()))
        return;

    ui->tbType->insertRow(nRow);
    QTableWidgetItem* pItem=new QTableWidgetItem("New");
    ui->tbType->setItem(nRow,0,pItem);

    QScrollBar* verticalScrollBar = ui->tbType->verticalScrollBar();
    if(verticalScrollBar!=nullptr)
    {
        ui->tbType->scrollToItem(pItem);
        nRow=verticalScrollBar->maximum();
        verticalScrollBar->setValue(nRow);
    }
}

void dlgTypeSelect::on_btnSave_clicked()
{
    std::map<QString,QString> datas;
    int nRow=ui->tbType->rowCount();
    QTableWidgetItem* pType,*pName;
    for(int i=0;i<nRow;i++)
    {
        pType=ui->tbType->item(i,0);
        pName=ui->tbType->item(i,1);
        if(pType!=nullptr && pName!=nullptr && pType->text().size()>0 && pName->text().size()>0)
        {
            datas.insert(std::make_pair(pType->text(),pName->text()));
        }
    }
    if(datas.size()>0)
        gSystem->SaveTypeLists(datas);
    ReloadTypes();
}

void dlgTypeSelect::on_btnDel_clicked()
{
    QMessageBox::StandardButton reply;
    int nRow=ui->tbType->currentRow();
    QTableWidgetItem* pItem=ui->tbType->item(nRow,0);
    if(pItem!=nullptr)
    {
        reply = QMessageBox::question(this, tr("Delete File"), tr("Are you sure you want to delete?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            gSystem->DeleteType(pItem->text());
            ReloadTypes();
        }
    }
}


void dlgTypeSelect::on_tbType_cellDoubleClicked(int , int )
{
    int nRow=ui->tbType->currentRow();
    QTableWidgetItem* pItem=ui->tbType->item(nRow,0);
    if(pItem!=nullptr)
    {
        if(pItem->text().indexOf("New")>=0)
        {

        }
        else
        {
            on_btnSelect_clicked();
        }
    }
}

void dlgTypeSelect::on_btnSelect_clicked()
{
    int nRow=ui->tbType->currentRow();
    QTableWidgetItem* pItem=ui->tbType->item(nRow,0);
    if(pItem==nullptr || pItem->text().size()<=0)
        return;
    *m_pStrTypeSelect=pItem->text();
    emit close();
}



void dlgTypeSelect::ReloadTypes()
{
    std::map<QString,QString>::iterator itMap;
    QTableWidgetItem* pType,*pName;
    int nIndex;
    m_datas.clear();
    while(ui->tbType->rowCount()>0)
        ui->tbType->removeRow(0);
    if(gSystem->GetTypeLists(m_datas))
    {
        for(itMap=m_datas.begin();itMap!=m_datas.end();itMap++)
        {
            pType=new QTableWidgetItem(itMap->first);
            pName=new QTableWidgetItem(itMap->second);
            nIndex=ui->tbType->rowCount();
            ui->tbType->insertRow(nIndex);
            ui->tbType->setItem(nIndex,0,pType);
            ui->tbType->setItem(nIndex,1,pName);
            pType->setFlags(pType->flags() & ~Qt::ItemIsEditable);
        }
    }
    ui->tbType->resizeRowsToContents();
    ui->tbType->resizeColumnsToContents();
}
