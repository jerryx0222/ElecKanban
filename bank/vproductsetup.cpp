#include "vproductsetup.h"
#include "ui_vproductsetup.h"
#include "helcsignage.h"


extern HElcSignage* gSystem;

vProductSetup::vProductSetup(QWidget *parent) :
    HTabBase(parent),
    ui(new Ui::vProductSetup)
{
    ui->setupUi(this);


}

vProductSetup::~vProductSetup()
{
    delete ui;
}

void vProductSetup::OnInit()
{
    CreateTable();

    InsertProducts();
    InsertProcess();
    InsertPart();
}

void vProductSetup::CreateTable()
{

    int ColCount=2;
    QStringList names;
    names.push_back(tr("PartNo"));
    names.push_back(tr("Product"));
    ui->tbFinalProduct->setColumnCount(ColCount);
    ui->tbFinalProduct->setHorizontalHeaderLabels(names);
    ui->tbFinalProduct->setColumnWidth(0,200);
    ui->tbFinalProduct->setColumnWidth(1,220);

    names.clear();
    names.push_back(tr("ID"));
    names.push_back(tr("Name"));
    ui->tbProcess->setColumnCount(ColCount);
    ui->tbProcess->setHorizontalHeaderLabels(names);
    ui->tbProcess->setColumnWidth(0,200);
    ui->tbProcess->setColumnWidth(1,220);

    names.clear();
    names.push_back(tr("PartNo"));
    names.push_back(tr("Name"));
    ui->tbPart->setColumnCount(ColCount);
    ui->tbPart->setHorizontalHeaderLabels(names);
    ui->tbPart->setColumnWidth(0,220);
    ui->tbPart->setColumnWidth(1,270);


}

void vProductSetup::InsertProducts()
{
    while(ui->tbFinalProduct->rowCount()>0)
        ui->tbFinalProduct->removeRow(0);
    if(gSystem==nullptr) return;

    int nRow;
    QTableWidgetItem* pItem;
    std::map<QString,QString>::iterator itMap;
    std::map<QString,QString> mapProducts;
    gSystem->CopyProducts("",mapProducts);

    for(itMap=mapProducts.begin();itMap!=mapProducts.end();itMap++)
    {
        nRow=ui->tbFinalProduct->rowCount();
        ui->tbFinalProduct->insertRow(nRow);
        pItem=new QTableWidgetItem(itMap->first);
        ui->tbFinalProduct->setItem(nRow,0,pItem);
        pItem=new QTableWidgetItem(itMap->second);
        ui->tbFinalProduct->setItem(nRow,1,pItem);
    }



}

void vProductSetup::InsertProcess()
{
    while(ui->tbProcess->rowCount()>0)
        ui->tbProcess->removeRow(0);
    if(gSystem==nullptr) return;

    int nRow;
    QTableWidgetItem* pItem;
    std::map<QString,QString>::iterator itMap;
    std::map<QString,QString> mapProcess;
    gSystem->CopyProcess(mapProcess);

    for(itMap=mapProcess.begin();itMap!=mapProcess.end();itMap++)
    {
        nRow=ui->tbProcess->rowCount();
        ui->tbProcess->insertRow(nRow);
        pItem=new QTableWidgetItem(itMap->first);
        ui->tbProcess->setItem(nRow,0,pItem);
        pItem=new QTableWidgetItem(itMap->second);
        ui->tbProcess->setItem(nRow,1,pItem);
    }
}

void vProductSetup::InsertPart()
{
    while(ui->tbPart->rowCount()>0)
        ui->tbPart->removeRow(0);
    if(gSystem==nullptr) return;

    int nRow;
    QTableWidgetItem* pItem;
    std::map<QString,QString>::iterator itMap;
    std::map<QString,QString> mapProducts;
    //gSystem->CopyParts(mapProducts);

    for(itMap=mapProducts.begin();itMap!=mapProducts.end();itMap++)
    {
        nRow=ui->tbPart->rowCount();
        ui->tbPart->insertRow(nRow);
        pItem=new QTableWidgetItem(itMap->first);
        ui->tbPart->setItem(nRow,0,pItem);
        pItem=new QTableWidgetItem(itMap->second);
        ui->tbPart->setItem(nRow,1,pItem);
    }
}

void vProductSetup::on_btnPLoad_clicked()
{
    InsertProducts();
}

void vProductSetup::on_btnPNew_clicked()
{
    QTableWidgetItem* pItem;
    int nCount=ui->tbFinalProduct->rowCount();
    pItem=ui->tbFinalProduct->item(nCount-1,1);
    if(nCount!=0 && pItem==nullptr)
        return;

    ui->tbFinalProduct->insertRow(nCount);
    pItem=new QTableWidgetItem("New");
    ui->tbFinalProduct->setItem(nCount,0,pItem);
}

void vProductSetup::on_btnPSave_clicked()
{
    std::map<QString,QString> datas;
    QTableWidgetItem* pItem[2];
    int count=ui->tbFinalProduct->rowCount();

    for(int i=0;i<count;i++)
    {
        pItem[0]=ui->tbFinalProduct->item(i,0);
        pItem[1]=ui->tbFinalProduct->item(i,1);
        if(pItem[0]!=nullptr && pItem[1]!=nullptr)
            datas.insert(std::make_pair(pItem[0]->text(),pItem[1]->text()));
    }
    gSystem->NewProducts(datas);
    InsertProducts();
}

void vProductSetup::on_btnPDel_clicked()
{
    int nSel=ui->tbFinalProduct->currentRow();
    QTableWidgetItem* pItem=ui->tbFinalProduct->item(nSel,0);
    if(pItem!=nullptr)
    {
        if(gSystem->DeleteProduct(pItem->text()))
            ui->tbFinalProduct->removeRow(nSel);
    }
}


void vProductSetup::on_btnP2New_clicked()
{
    QTableWidgetItem* pItem;
    int nCount=ui->tbProcess->rowCount();
    pItem=ui->tbProcess->item(nCount-1,1);
    if(nCount!=0 && pItem==nullptr)
        return;

    ui->tbProcess->insertRow(nCount);
    pItem=new QTableWidgetItem("New");
    ui->tbProcess->setItem(nCount,0,pItem);
}



void vProductSetup::on_btnP2Del_clicked()
{
    int nSel=ui->tbProcess->currentRow();
    QTableWidgetItem* pItem=ui->tbProcess->item(nSel,0);
    if(pItem!=nullptr)
    {
        //if(gSystem->DeleteProcess(pItem->text()))
        //    ui->tbProcess->removeRow(nSel);
    }
}


void vProductSetup::on_btnP2Load_clicked()
{
    InsertProcess();
}

void vProductSetup::on_btnP2Save_clicked()
{
    std::map<QString,QString> datas;
    QTableWidgetItem* pItem[2];
    int count=ui->tbProcess->rowCount();

    for(int i=0;i<count;i++)
    {
        pItem[0]=ui->tbProcess->item(i,0);
        pItem[1]=ui->tbProcess->item(i,1);
        if(pItem[0]!=nullptr && pItem[1]!=nullptr)
            datas.insert(std::make_pair(pItem[0]->text(),pItem[1]->text()));
    }
    //gSystem->NewProcess(datas);
    InsertProcess();
}

void vProductSetup::on_btnP3Load_clicked()
{
    InsertPart();
}

void vProductSetup::on_btnP3New_clicked()
{
    QTableWidgetItem* pItem;
    int nCount=ui->tbPart->rowCount();
    pItem=ui->tbPart->item(nCount-1,1);
    if(nCount!=0 && pItem==nullptr)
        return;

    ui->tbPart->insertRow(nCount);
    pItem=new QTableWidgetItem("New");
    ui->tbPart->setItem(nCount,0,pItem);
}

void vProductSetup::on_btnP3Save_clicked()
{
    std::map<QString,QString> datas;
    QTableWidgetItem* pItem[2];
    int count=ui->tbPart->rowCount();

    for(int i=0;i<count;i++)
    {
        pItem[0]=ui->tbPart->item(i,0);
        pItem[1]=ui->tbPart->item(i,1);
        if(pItem[0]!=nullptr && pItem[1]!=nullptr)
            datas.insert(std::make_pair(pItem[0]->text(),pItem[1]->text()));
    }
    //gSystem->NewParts(datas);
    InsertProducts();
}

void vProductSetup::on_btnP3Del_clicked()
{
    int nSel=ui->tbPart->currentRow();
    QTableWidgetItem* pItem=ui->tbPart->item(nSel,0);
    if(pItem!=nullptr)
    {
        //if(gSystem->DeletePart(pItem->text()))
        //    ui->tbPart->removeRow(nSel);
    }
}
