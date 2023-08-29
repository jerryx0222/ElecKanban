#include "vprocesssetup.h"
#include "ui_vprocesssetup.h"
#include "helcsignage.h"
#include "dlgselprocess.h"
#include <QTreeWidgetItem>

extern HElcSignage* gSystem;

vProcessSetup::vProcessSetup(QWidget *parent) :
    HTabBase(parent),
    ui(new Ui::vProcessSetup)
{
    ui->setupUi(this);
    m_pButton=nullptr;

}

vProcessSetup::~vProcessSetup()
{
    delete ui;
}

void vProcessSetup::OnInit()
{
    CreateTable();
    InsertProducts();
    InsertProcess();
}

void vProcessSetup::CreateTable()
{
    QStringList names;
    names.push_back(tr("PartNo"));
    names.push_back(tr("Product"));
    ui->tbFinalProduct->setColumnCount(names.size());
    ui->tbFinalProduct->setHorizontalHeaderLabels(names);
    ui->tbFinalProduct->setColumnWidth(0,200);
    ui->tbFinalProduct->setColumnWidth(1,220);


    names.clear();;
    names.push_back(tr("Process"));
    ui->treProcess->setColumnCount(names.size());
    ui->treProcess->setHeaderLabels(names);
    ui->treProcess->setColumnWidth(0,1000);

    ui->treProcess->setSortingEnabled(false);
}

void vProcessSetup::InsertProducts()
{
    m_mapProducts.clear();
    gSystem->CopyProducts("",m_mapProducts);
    RelistProduct("*");
    ui->edtProduct->setText("*");
}

void vProcessSetup::InsertProcess()
{



}

void vProcessSetup::RelistProduct(QString strKey)
{
    while(ui->tbFinalProduct->rowCount()>0)
        ui->tbFinalProduct->removeRow(0);

    std::map<QString,QString>::iterator itMap;
    int nRow;
    QTableWidgetItem* pItem;
    if(strKey=="*" || strKey.size()<=0)
    {
        for(itMap=m_mapProducts.begin();itMap!=m_mapProducts.end();itMap++)
        {
            nRow=ui->tbFinalProduct->rowCount();
            ui->tbFinalProduct->insertRow(nRow);
            pItem=new QTableWidgetItem(itMap->first);
            ui->tbFinalProduct->setItem(nRow,0,pItem);
            pItem=new QTableWidgetItem(itMap->second);
            ui->tbFinalProduct->setItem(nRow,1,pItem);
        }
    }
    else
    {
        for(itMap=m_mapProducts.begin();itMap!=m_mapProducts.end();itMap++)
        {
            if(itMap->first.indexOf(strKey)>=0)
            {
                nRow=ui->tbFinalProduct->rowCount();
                ui->tbFinalProduct->insertRow(nRow);
                pItem=new QTableWidgetItem(itMap->first);
                ui->tbFinalProduct->setItem(nRow,0,pItem);
                pItem=new QTableWidgetItem(itMap->second);
                ui->tbFinalProduct->setItem(nRow,1,pItem);
            }
        }
    }
}

void vProcessSetup::DisplayProductInfo()
{
    /*
    ui->treProcess->clear();
    ui->btnAdd->setEnabled(false);
    ui->btnNewP->setEnabled(false);
    ui->btnDelete->setEnabled(false);
    if(m_pButton!=nullptr)
    {
        delete m_pButton;
        m_pButton=nullptr;
    }
    QTableWidgetItem* pItem=ui->tbFinalProduct->item(ui->tbFinalProduct->currentRow(),0);
    if(pItem==nullptr)
        return;

    QString strValue,strPartId=pItem->text();
    std::map<QString,QString>::iterator itMap=m_mapProducts.find(strPartId);
    if(itMap!=m_mapProducts.end())
        strValue=QString("%1(%2)").arg(itMap->first).arg(itMap->second);
    else
    {
        strValue="";
        ui->edtProduct2->setText(strValue);
        return;
    }
    ui->edtProduct2->setText(strValue);
    ui->btnAdd->setEnabled(true);
    ui->btnNewP->setEnabled(true);
    ui->btnDelete->setEnabled(true);

    HProduct* pProduct=nullptr;
    if(!gSystem->GetProduct(strPartId,&pProduct) || pProduct==nullptr)
        return;

    QTreeWidgetItem* pTreeTop;
    QTreeWidgetItem* pChild;
    std::map<int,Process*>::iterator itP;
    int nCount;
    QString strID;
    for(itP=pProduct->m_Processes.begin();itP!=pProduct->m_Processes.end();itP++)
    {
        pTreeTop=new QTreeWidgetItem(ui->treProcess);
        strValue=QString("%1:%2").arg(itP->second->id).arg(itP->second->name);
        pTreeTop->setText(0,strValue);
        ui->treProcess->addTopLevelItem(pTreeTop);

        nCount=itP->second->components.size();
        if(nCount>0)
        {
            for(int i=0;i<nCount;i++)
            {
                strID=gSystem->GetPart(pProduct->GetProcessPart(itP->first,i));
                if(strID.size()>0)
                {
                    pChild=new QTreeWidgetItem(pTreeTop);
                    pTreeTop->addChild(pChild);
                    if(i==0)
                        pChild->setText(0,strID);
                    else
                        pChild->setText(0,strID);
                }
            }
        }
    }

     ui->treProcess->expandAll();
     */
}

void vProcessSetup::on_btnSearch_clicked()
{
    QString strKey=ui->edtProduct->text();
    RelistProduct(strKey);
}

void vProcessSetup::on_tbFinalProduct_cellClicked(int row, int )
{
    QTableWidgetItem* pItem=ui->tbFinalProduct->item(row,0);
    if(pItem==nullptr)
    {
        ui->treProcess->clear();
        return;
    }

    DisplayProductInfo();

    QString strValue,strId=pItem->text();
    std::map<QString,QString>::iterator itMap=m_mapProducts.find(strId);
    if(itMap!=m_mapProducts.end())
    {
        strValue=QString("%1:%2").arg(itMap->first).arg(itMap->second);
    }
    else
    {
        strValue="";
    }
    ui->edtProduct2->setText(strValue);

}

void vProcessSetup::on_btnNewP_clicked()
{
    QTreeWidgetItem* pItem=ui->treProcess->currentItem();
    QTreeWidgetItem* pChild;
    if(m_pButton!=nullptr)
        return;
    if(pItem!=nullptr)
    {
        int index=ui->treProcess->indexOfTopLevelItem(pItem);
        if((index%2)==1)
            m_nInsertIndex=1;
        else
            m_nInsertIndex=0;
        pChild=new QTreeWidgetItem(pItem);
        pItem->addChild(pChild);
    }
    else
    {
        m_nInsertIndex=0;
        pChild=new QTreeWidgetItem(ui->treProcess);
        ui->treProcess->addTopLevelItem(pItem);
    }
    pChild->setText(0,"");

    m_pButton = new QPushButton(tr("Select"));
    connect(m_pButton,&QPushButton::clicked,this,&vProcessSetup::OnBtnSelectProcess);
    ui->treProcess->setItemWidget(pChild, 0, m_pButton);

}

void vProcessSetup::on_btnAdd_clicked()
{
    if(m_pButton!=nullptr)
        return;

    QTreeWidgetItem* pChild = new QTreeWidgetItem(ui->treProcess);
    ui->treProcess->addTopLevelItem(pChild);
    pChild->setText(0,"");

    m_nInsertIndex=0;
    m_pButton = new QPushButton(tr("Select"));
    connect(m_pButton,&QPushButton::clicked,this,&vProcessSetup::OnBtnSelectProcess);
    ui->treProcess->setItemWidget(pChild, 0, m_pButton);
}


void vProcessSetup::OnBtnSelectProcess()
{
    QTreeWidgetItem* pItem=ui->treProcess->currentItem();
    QString strID,strName;
    dlgSelProcess* pNew=new dlgSelProcess(&strID,&strName,this);
    pNew->setModal(true);
    if(m_nInsertIndex==1)
        pNew->setWindowTitle(tr("Process select"));
    else
        pNew->setWindowTitle(tr("Product select"));

    QPushButton *button1=dynamic_cast<QPushButton*>(ui->treProcess->itemWidget(pItem,0));
    int result=pNew->exec();
    if(result==QDialog::Accepted)
    {
        QString strPart=pNew->strProcessID;
        if(pItem!=nullptr && button1!=nullptr && strPart.size()>0)
        {
            pItem->setText(0,strPart);
            ui->treProcess->removeItemWidget(pItem,0);
        }
    }
    delete pNew;
    m_pButton=nullptr;
    ui->treProcess->expandAll();
}

void vProcessSetup::on_btnSave_clicked()
{
    /*
    QString strProduct=ui->edtProduct2->text();
    if(strProduct.size()<=0)
        return;
    int sel=strProduct.indexOf(":");
    if(sel<=0)
        return;

    HProduct* pProduct=new HProduct();
    pProduct->m_PartNo=strProduct.left(sel);
    pProduct->m_Name=strProduct.right(strProduct.size()-sel-1);

    int CountProcess=ui->treProcess->topLevelItemCount();
    for(int i=0;i<CountProcess;i++)
    {
        QTreeWidgetItem* pTop=ui->treProcess->topLevelItem(i);  // pTop=Process
        if(pTop!=nullptr)
        {
            Process* pP=new Process();
            pP->PartNo=pProduct->m_PartNo;
            QString strProcess=pTop->text(0);
            sel=strProcess.indexOf(":");
            if(sel<=0)
            {
                delete pP;
                continue;
            }
            pP->id=strProcess.left(sel);
            pP->name=strProcess.right(strProcess.size()-sel-1);
            pProduct->m_Processes.insert(std::make_pair(i,pP));

            int part=pTop->childCount();
            if(part>0)
            {
                for(int i=0;i<part;i++)
                {
                    QTreeWidgetItem* pPart=pTop->child(i);
                    if(pPart!=nullptr)
                    {
                        QString strPart=pPart->text(0);
                        sel=strPart.indexOf(":");
                        if(sel>0)
                            pP->components.push_back(strPart.left(sel));
                    }
                }
            }
        }

    }
    */
    /*
    Shipment *pShip=new Shipment();
    QString strDate="1900-01-01";
    pShip->date=QDate::fromString(strDate,"yyyy-MM-dd");

    pProduct->m_Shipments.insert(std::make_pair(strDate,pShip));

    gSystem->SaveProduct(pProduct);
    delete pProduct;
    */
}

void vProcessSetup::on_btnDelete_clicked()
{
    QTreeWidgetItem* pItem=ui->treProcess->currentItem();
    if(pItem!=nullptr)
    {
        m_pButton=nullptr;
        delete pItem;
    }
}

