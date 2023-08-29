#include "vshipment.h"
#include "ui_vshipment.h"
#include "helcsignage.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QMessageBox>
#include "dlgtypeselect.h"

extern HElcSignage* gSystem;

vShipment::vShipment(QWidget *parent)
    :HTabBase(parent),
    ui(new Ui::vShipment)
{
    ui->setupUi(this);
}

vShipment::~vShipment()
{
    delete ui;
}

void vShipment::OnInit()
{
    ui->tbProducts->setSortingEnabled(false);

    CreateTable();
    //ui->dateEdit->setDate(QDate::currentDate());
    RelistDates();
    RelistProducts();

}

void vShipment::OnShowTab(bool show)
{
    QString strName;
    if(!show)
        return;

    ui->btnShiipLoad->setEnabled(false);
    ui->btnHistoryLoad->setEnabled(false);
    ui->btnStatusLoad->setEnabled(false);

    if(gSystem->m_TypeSelect.size()>0)
    {
        strName=gSystem->GetTypeFullName(gSystem->m_TypeSelect);
        if(strName.size()>0)
        {
            strName=QString("%2(%1)").arg(strName).arg(gSystem->m_TypeSelect);
            ui->lblType->setText(strName);
        }
        RelistDates();
        RelistProducts();

        while(ui->tbShip2->rowCount()>0)
            ui->tbShip2->removeRow(0);
        while(ui->tbProcess->rowCount()>0)
            ui->tbProcess->removeRow(0);
    }
}

void vShipment::CreateTable()
{
    QStringList strTitles;
    strTitles.push_back(tr("Date"));
    strTitles.push_back(tr("Product"));
    strTitles.push_back(tr("Export"));
    strTitles.push_back(tr("WareHouse"));
    strTitles.push_back(tr("Rate"));
    strTitles.push_back(tr("Missing"));


    ui->tbShip2->setColumnCount(strTitles.size());
    ui->tbShip2->setHorizontalHeaderLabels(strTitles);
    ui->tbShip2->setColumnWidth(0,190);
    ui->tbShip2->setColumnWidth(1,270);
    ui->tbShip2->setColumnWidth(2,200);
    ui->tbShip2->setColumnWidth(3,200);
    ui->tbShip2->setColumnWidth(4,200);
    ui->tbShip2->setSelectionMode(QAbstractItemView::SingleSelection);


    strTitles.clear();
    strTitles.push_back(tr("ProductID"));
    strTitles.push_back(tr("Name"));
    strTitles.push_back(tr("pass"));

    ui->tbProducts->setColumnCount(strTitles.size());
    ui->tbProducts->setHorizontalHeaderLabels(strTitles);
    ui->tbProducts->setColumnWidth(0,250);
    ui->tbProducts->setColumnWidth(1,450);
    ui->tbProducts->setColumnWidth(2,100);
    ui->tbProducts->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tbProducts->setSelectionBehavior(QAbstractItemView::SelectRows);

    strTitles.clear();
    strTitles.push_back(tr("ID"));
    strTitles.push_back(tr("Name"));
    strTitles.push_back(tr("Stock"));
    strTitles.push_back(tr("Target"));
    ui->tbProcess->setColumnCount(strTitles.size());
    ui->tbProcess->setHorizontalHeaderLabels(strTitles);
    ui->tbProcess->setColumnWidth(0,130);
    ui->tbProcess->setColumnWidth(1,230);
    ui->tbProcess->setColumnWidth(2,100);
    ui->tbProcess->setColumnWidth(3,100);
    ui->tbProcess->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tbProcess->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void vShipment::RelistDates()
{
    m_listDates.clear();
    ui->lstDate->clear();

    gSystem->GetDatesFromShipments(gSystem->m_TypeSelect,m_listDates);
    for(int i=0;i<m_listDates.size();i++)
    {
        ui->lstDate->addItem(m_listDates[i]);
    }
}

void vShipment::RelistProducts()
{
    std::map<QString,QString>::iterator itMap;
    std::map<QString,QString> datas;
    QTableWidgetItem* pItem;
    Product* pProduct=nullptr;
    int index=0;

    gSystem->CopyProducts(gSystem->m_TypeSelect,datas);
    while(ui->tbProducts->rowCount()>0)
        ui->tbProducts->removeRow(0);
    for(itMap=datas.begin();itMap!=datas.end();itMap++)
    {
        gSystem->GetProductStruct(itMap->first,&pProduct);

        ui->tbProducts->insertRow(index);
        pItem=new QTableWidgetItem(itMap->first);
        ui->tbProducts->setItem(index,0,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        pItem=new QTableWidgetItem(itMap->second);
        ui->tbProducts->setItem(index,1,pItem);

        if(pProduct!=nullptr)
        {
            pItem=new QTableWidgetItem(QString("%1").arg(pProduct->FirstPass));
            ui->tbProducts->setItem(index,2,pItem);
        }

        index++;
    }

    ui->tbProducts->resizeColumnsToContents();
    ui->tbProducts->resizeRowsToContents();
}




void vShipment::RelistShips()
{
    QTableWidgetItem* pItem;
    QStringList lstProduct;
    QDate   DateSelect;
    QString strDateSelect;
    int nDateSel=ui->lstDate->currentRow();
    int nProduct=ui->tbProducts->rowCount();

    if(nDateSel>=0)
    {
        strDateSelect=ui->lstDate->item(nDateSel)->text();
        DateSelect=QDate::fromString(strDateSelect,"yyyy/MM/dd");
    }

    for(int i=0;i<nProduct;i++)
    {
        pItem=ui->tbProducts->item(i,0);
        if(pItem!=nullptr)
            lstProduct.push_back(pItem->text());
    }

    std::map<QString,Shipment>::iterator itMap;
    std::map<QString,Shipment> shipments1,shipments2;
    Shipment* pShipment;
    gSystem->GetShipMents(lstProduct,shipments1);
    for(itMap=shipments1.begin();itMap!=shipments1.end();itMap++)
    {
        if(DateSelect.isNull() || itMap->second.date.isNull() || DateSelect!=itMap->second.date)
            continue;
        shipments2.insert(std::make_pair(itMap->first,itMap->second));
    }


    int nNow=ui->tbShip2->rowCount();
    int nTarget=static_cast<int>(shipments2.size());
    if(nTarget<nNow)
    {
        while(ui->tbShip2->rowCount()>nTarget)
            ui->tbShip2->removeRow(0);
    }


    QString strValue,strDate,strProduct;
    int st,index,nRow=0;
    double dblValue;
    for(itMap=shipments2.begin();itMap!=shipments2.end();itMap++)
    {
        st=itMap->first.indexOf(":");
        if(st>0)
        {
            strDate=itMap->first.left(st);
            strProduct=itMap->first.right(itMap->first.size()-st-1);
            pShipment=&itMap->second;
            if(strDate.size()>0 && strProduct.size()>0)
            {
                if(nRow>=ui->tbShip2->rowCount())
                    ui->tbShip2->insertRow(nRow);

                //Date
                index=0;
                pItem=ui->tbShip2->item(nRow,index);
                if(pItem==nullptr)
                {
                    pItem=new QTableWidgetItem(strDate);
                    ui->tbShip2->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=ui->tbShip2->item(nRow,index);
                    pItem->setText(strDate);
                }
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

                //Product
                index++;
                pItem=ui->tbShip2->item(nRow,index);
                if(pItem==nullptr)
                {
                    pItem=new QTableWidgetItem(strProduct);
                    ui->tbShip2->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=ui->tbShip2->item(nRow,index);
                    pItem->setText(strProduct);
                }
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

                //Export
                index++;
                pItem=ui->tbShip2->item(nRow,index);
                strValue=QString("%1").arg(pShipment->OutCount);
                if(pItem==nullptr)
                {
                    pItem=new QTableWidgetItem(strValue);
                    ui->tbShip2->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=ui->tbShip2->item(nRow,index);
                    pItem->setText(strValue);
                }
                pItem->setTextAlignment(Qt::AlignCenter);

                //WareHouse
                index++;
                pItem=ui->tbShip2->item(nRow,index);
                strValue=QString("%1").arg(
                            QVariant(pShipment->StockCount).toInt());
                if(pItem==nullptr)
                {
                    pItem=new QTableWidgetItem(strValue);
                    ui->tbShip2->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=ui->tbShip2->item(nRow,index);
                    pItem->setText(strValue);
                }
                pItem->setTextAlignment(Qt::AlignCenter);
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

                //Rate
                index++;
                pItem=ui->tbShip2->item(nRow,index);
                if(pShipment->OutCount<=0)
                    strValue="";
                else
                {
                    dblValue=100.0f * pShipment->StockCount/pShipment->OutCount;
                    strValue=QString::number(dblValue,'f',2);
                    strValue += "%";
                }
                if(pItem==nullptr)
                {
                    pItem=new QTableWidgetItem(strValue);
                    ui->tbShip2->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=ui->tbShip2->item(nRow,index);
                    pItem->setText(strValue);
                }
                pItem->setTextAlignment(Qt::AlignCenter);
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

                //Missing
                index++;
                pItem=ui->tbShip2->item(nRow,index);
                if(pShipment->OutCount<=0)
                    strValue="";
                else
                    strValue=QVariant(pShipment->OutCount-pShipment->StockCount).toString();
                if(pItem==nullptr)
                {
                    pItem=new QTableWidgetItem(strValue);
                    ui->tbShip2->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=ui->tbShip2->item(nRow,index);
                    pItem->setText(strValue);
                }
                pItem->setTextAlignment(Qt::AlignCenter);
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

                nRow++;
            }
        }
    }

    ui->tbShip2->resizeRowsToContents();
    ui->tbShip2->resizeColumnsToContents();
}




void vShipment::on_lstDate_itemClicked(QListWidgetItem *)
{
     RelistShips();
}

void vShipment::on_btnShipRelist_clicked()
{
    RelistShips();
}

void vShipment::on_btnShiipSave_clicked()
{
    QTableWidgetItem* pItem;
    int nRowCount=ui->tbShip2->rowCount();
    QString strDate,strProduct;
    std::map<QString,Shipment> shipments;



    for(int i=0;i<nRowCount;i++)
    {
        Shipment ship;

        //Date
        pItem=ui->tbShip2->item(i,0);
        if(pItem==nullptr)
            return;
        strDate=pItem->text();
        ship.date=QDate::fromString(strDate,"yyyy/MM/dd");

        //Product
        pItem=ui->tbShip2->item(i,1);
        if(pItem==nullptr)
            return;
        strProduct=pItem->text();

        //Export
        pItem=ui->tbShip2->item(i,2);
        if(pItem==nullptr)
            return;
        ship.OutCount=pItem->text().toInt();

        //WareHouse
        pItem=ui->tbShip2->item(i,3);
        if(pItem==nullptr)
            return;
        ship.StockCount=pItem->text().toInt();


        shipments.insert(std::make_pair(QString("%1:%2").arg(strDate).arg(strProduct),ship));
    }
    gSystem->SaveShipMents(shipments);
    RelistShips();
}


/*
void vShipment::on_btnInsertDate_clicked()
{
    if(gSystem->m_TypeSelect.size()<0)
        return;

    QDate selectedDate = ui->calendarWidget->selectedDate();
    gSystem->AddNewShipment(gSystem->m_TypeSelect,selectedDate);
    RelistDates();
}
*/

void vShipment::on_btnRemoveDate_clicked()
{
    QString strDate=ui->lstDate->currentItem()->text();
    QDate selectedDate = QDate::fromString(strDate,"yyyy/MM/dd");//ui->calendarWidget->selectedDate();
    gSystem->DelShipment(selectedDate);
    RelistDates();
}

void vShipment::on_btnInsert_clicked()
{
    QTableWidgetItem* pItem;
    int nRow=ui->tbProducts->rowCount();
    if(nRow>0)
    {
        pItem=ui->tbProducts->item(nRow-1,0);
        if(pItem==nullptr)
            return;
        if(pItem->text()=="New")
            return;
    }
    else if(gSystem->m_TypeSelect.size()<=0)
        return;

    ui->tbProducts->insertRow(nRow);
    pItem=new QTableWidgetItem("New");
    ui->tbProducts->setItem(nRow,0,pItem);
    pItem=new QTableWidgetItem("New");
    ui->tbProducts->setItem(nRow,1,pItem);
    pItem=new QTableWidgetItem("0.9");
    ui->tbProducts->setItem(nRow,2,pItem);

    ui->tbProducts->scrollToItem(pItem);

    QScrollBar* pScrollBar = ui->tbProducts->verticalScrollBar();
    if(pScrollBar!=nullptr)
        pScrollBar->setValue(pScrollBar->maximum());

}
void vShipment::on_btnRemove_clicked()
{
    int nRow=ui->tbProducts->currentRow();
    QTableWidgetItem* pItem=ui->tbProducts->item(nRow,0);
    if(pItem==nullptr)
        return;
    ui->tbProducts->removeRow(nRow);
}
void vShipment::on_btnSave_clicked()
{
    QTableWidgetItem* pItem;
    QTableWidgetItem* pItem2;
    QString strID;
    std::map<QString,Product*>   mapProducts;
    if(gSystem->m_TypeSelect.size()<=0)
        return;

    int rowCount=ui->tbProducts->rowCount();
    for(int i=0;i<rowCount;i++)
    {
        pItem=ui->tbProducts->item(i,0);
        if(pItem!=nullptr)
        {
            strID=pItem->text();
            pItem=ui->tbProducts->item(i,1);
            pItem2=ui->tbProducts->item(i,2);
            if(strID.size()>0 && pItem!=nullptr)
            {
                Product* pNew=new Product();
                pNew->CName=pItem->text();
                pNew->ProductID=strID;
                if(pItem2==nullptr)
                    pNew->FirstPass=0.9;
                else
                    pNew->FirstPass=pItem2->text().toDouble();
                if(pNew->FirstPass>1) pNew->FirstPass=1;
                if(pNew->FirstPass<0) pNew->FirstPass=0;

                pNew->Stock=0;
                pNew->TypeID=gSystem->m_TypeSelect;
                mapProducts.insert(std::make_pair(strID,pNew));
            }
        }
    }
    gSystem->SaveProducts(gSystem->m_TypeSelect,mapProducts);
    std::map<QString,Product*>::iterator itMap;
    for(itMap=mapProducts.begin();itMap!=mapProducts.end();itMap++)
        delete itMap->second;
    RelistProducts();
    RelistShips();
}

void vShipment::on_btnRelist_clicked()
{
    RelistProducts();
    RelistShips();
}



void vShipment::RelistProcess(QString strProductID)
{
    //int nRow=ui->tbProducts->currentRow();
    QTableWidgetItem* pItem;//=ui->tbProducts->item(nRow,0);
    while(ui->tbProcess->rowCount()>0)
        ui->tbProcess->removeRow(0);
    //if(pItem==nullptr)
    //    return;

    std::list<ProcessInfo>::iterator itL;
   //QString strProductID=pItem->text();
   std::list<ProcessInfo> datas;
   ProcessInfo* pInfo;
    int index=0;
    if(!gSystem->GetProductProcess(strProductID,datas))
        return;

    for(itL=datas.begin();itL!=datas.end();itL++)
    {
        pInfo=&(*itL);
        ui->tbProcess->insertRow(index);
        pItem=new QTableWidgetItem(pInfo->ProcessID);
        ui->tbProcess->setItem(index,0,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        pItem=new QTableWidgetItem(pInfo->ProcessName);
        ui->tbProcess->setItem(index,1,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        pItem=new QTableWidgetItem(QString("%1").arg(pInfo->Target));
        ui->tbProcess->setItem(index,2,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        pItem=new QTableWidgetItem(QString("%1").arg(pInfo->Source));
        ui->tbProcess->setItem(index,3,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        index++;
    }

    ui->tbProcess->resizeColumnsToContents();
    ui->tbProcess->resizeRowsToContents();

}

void vShipment::on_btnShiipLoad_clicked()
{
    QString strPath="D:/";
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        strPath,
        tr("Excel Files (*.XLSX *.xlsx);;All Files (*)"));
    if (fileName.isEmpty())
        return;


    if(gSystem->ImportERPFile(gSystem->m_TypeSelect,fileName))
        RelistShips();
    else
        QMessageBox::warning(nullptr,tr("Warm"),tr("Import Failed!"));

}

void vShipment::on_btnStatusLoad_clicked()
{
    QString strPath="D:/";
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        strPath,
        tr("Excel Files (*.XLSX *.xlsx);;All Files (*)"));
    if (fileName.isEmpty())
        return;


    if(gSystem->ImportSFCStatusFile(gSystem->m_TypeSelect,fileName))
    {
        QTableWidgetItem* pItem=ui->tbProcess->item(ui->tbProcess->currentRow(),0);
        if(pItem!=nullptr)
            RelistProcess(pItem->text());
    }
    else
        QMessageBox::warning(nullptr,tr("Warm"),tr("Import Failed!"));
}

void vShipment::on_btnHistoryLoad_clicked()
{
    QString strPath="D:/";
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        strPath,
        tr("Excel Files (*.XLSX *.xlsx);;All Files (*)"));
    if (fileName.isEmpty())
        return;


    if(!gSystem->ImportSFCHistoryFile(gSystem->m_TypeSelect,fileName))
        QMessageBox::warning(nullptr,tr("Warm"),tr("Import Failed!"));

}

void vShipment::on_tbShip2_cellClicked(int row, int)
{
    QTableWidgetItem* pItem=ui->tbShip2->item(row,1);
    if(pItem!=nullptr)
    {
        RelistProcess(pItem->text());
    }
}

void vShipment::on_tbProducts_cellClicked(int row, int)
{
    QTableWidgetItem* pItem=ui->tbProducts->item(row,0);
    if(pItem!=nullptr)
    {
        RelistProcess(pItem->text());
    }
}


void vShipment::on_btnTypeSelect_clicked()
{
    QString strName,strSelect;
    dlgTypeSelect* pNew=new dlgTypeSelect(&strSelect,this);
    pNew->setModal(true);
    pNew->setWindowTitle(tr("Type Select"));

    pNew->exec();
    if(strSelect.size()>0)
    {
        strName=gSystem->GetTypeFullName(strSelect);
        if(strName.size()>0)
        {
            strName=QString("%2(%1)").arg(strName).arg(strSelect);
            ui->lblType->setText(strName);
            gSystem->m_TypeSelect=strSelect;
            RelistDates();
            RelistProducts();

            while(ui->tbShip2->rowCount()>0)
                ui->tbShip2->removeRow(0);
            while(ui->tbProcess->rowCount()>0)
                ui->tbProcess->removeRow(0);
        }
    }
}


void vShipment::on_btnTest_clicked()
{

}

void vShipment::on_btnT1_clicked()
{
    gSystem->GetSFC();
}

void vShipment::on_btnLogin_clicked()
{

    /*
    QString strPws=ui->edtPwd->text();
    SystemData* pData=gSystem->GetSystemData("password");
    if(pData!=nullptr)
        emit OnUserLogin(strPws==pData->strValue);
    else
        emit OnUserLogin(strPws=="1234");
    ui->edtPwd->setText("");


    gSystem->GetSFC();
    */
}
