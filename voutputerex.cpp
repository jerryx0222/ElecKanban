#include "voutputerex.h"
#include "ui_voutputerex.h"
#include "dlgtypeselect.h"
#include "helcsignage.h"
#include <QFileDialog>
#include <QMessageBox>
#include "xlsx/xlsxdocument.h"
#include "xlsx/xlsxcellreference.h"

extern HElcSignage* gSystem;
extern bool gLogin;

vOutputerEx::vOutputerEx(QWidget *parent) :
    HTabBase(parent),
    ui(new Ui::vOutputerEx)
{
    ui->setupUi(this);
}

vOutputerEx::~vOutputerEx()
{
    delete ui;
}

void vOutputerEx::OnInit()
{
    QStringList strTitles;
    //strTitles.push_back(tr("Date"));
    strTitles.push_back(tr("Product"));
    //strTitles.push_back(tr("Name"));
    strTitles.push_back(tr("Export"));
    strTitles.push_back(tr("WareHouse"));
    strTitles.push_back(tr("Rate"));
    strTitles.push_back(tr("Missing"));

    m_vTables.push_back(ui->tbInfo1);
    m_vTables.push_back(ui->tbInfo2);
    //m_vTables.push_back(ui->tbInfo3);
    //m_vTables.push_back(ui->tbInfo4);

    m_vLabels.push_back(ui->lblDate1);
    m_vLabels.push_back(ui->lblDate2);
    //m_vLabels.push_back(ui->lblDate3);
    //m_vLabels.push_back(ui->lblDate4);

    for(size_t i=0;i<m_vTables.size();i++)
    {
        m_vTables[i]->setColumnCount(strTitles.size());
        m_vTables[i]->setHorizontalHeaderLabels(strTitles);

        m_vTables[i]->setSelectionMode(QAbstractItemView::SingleSelection);

        m_vTables[i]->resizeColumnsToContents();
        m_vTables[i]->resizeRowsToContents();
    }


}

void vOutputerEx::OnShowTab(bool show)
{
    if(!show)
        return;
    ui->btnUpdate->setEnabled(gLogin);
    ui->lblType->setText(gSystem->m_TypeSelect);
    RelistProducts();

}

void vOutputerEx::RelistProducts()
{


    m_Products.clear();
    m_lstDates.clear();
    gSystem->CopyProducts(gSystem->m_TypeSelect,m_Products);
    gSystem->GetDatesFromShipments(gSystem->m_TypeSelect,m_lstDates);   // 取得出貨日期
    int count=m_lstDates.size()/2;
    if(m_lstDates.size()<=2)
    {
        ui->VBar->setEnabled(false);
        ui->VBar->setRange(0,count);
    }
    else
    {
        ui->VBar->setEnabled(true);
        ui->VBar->setRange(0,count);
    }

    if(m_lstDates.size()<=0)
    {
        for(int i=0;i<m_vTables.size();i++)
        {
            while(m_vTables[i]->rowCount()>0)
                m_vTables[i]->removeRow(0);
        }
        return;
    }
    for(int i=0;i<m_lstDates.size();i++)
    {
        if(i<m_lstDates.size())
        {
            //RelistShips(i);
            if(i<m_vLabels.size())
                RelistShipsFromDB(i);
        }
        else
        {
            while(m_vTables[i]->rowCount()>0)
                m_vTables[i]->removeRow(0);

        }
    }
}

void vOutputerEx::RelistProductsByIndex(int sel)
{
    m_Products.clear();
    m_lstDates.clear();
    gSystem->CopyProducts(gSystem->m_TypeSelect,m_Products);
    gSystem->GetDatesFromShipments(gSystem->m_TypeSelect,m_lstDates);   // 取得出貨日期

    for(int i=0;i<m_vLabels.size();i++)
    {
        int index=sel*m_vLabels.size()+i;
        if(index<m_lstDates.size())
        {
            //RelistShips(i);
            RelistShipsFromDB(index);
        }
        else
        {
            m_vLabels[i]->setText("---");
            while(m_vTables[i]->rowCount()>0)
                m_vTables[i]->removeRow(0);

        }
    }
}

void vOutputerEx::on_btnSelect_clicked()
{
    QString strName,strSelect;
    dlgTypeSelect* pNew=new dlgTypeSelect(&strSelect,this);
    pNew->setModal(true);
    pNew->setWindowTitle(tr("Type Select"));
    ui->lblType->setText("");

    pNew->exec();
    if(strSelect.size()>0)
    {
        gSystem->m_TypeSelect=strSelect;
        strName=QString("%1(%2)").arg(strSelect).arg(gSystem->GetTypeFullName(strSelect));
        ui->lblType->setText(strName);
        RelistProducts();
    }
    delete pNew;
}

void vOutputerEx::RelistShips(size_t nDateSel)
{
    std::map<QString,QString>::iterator itProduct;
    QTableWidgetItem* pItem;
    QStringList lstProduct;
    QDate   DateSelect;
    QString strDateSelect;

    m_vLabels[nDateSel]->setText("");
    if(nDateSel>=0)
    {
        strDateSelect=m_lstDates[nDateSel];
        DateSelect=QDate::fromString(strDateSelect,"yyyy/MM/dd");
    }

    for(itProduct=m_Products.begin();itProduct!=m_Products.end();itProduct++)
    {
        lstProduct.push_back(itProduct->first);
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


    int nNow=m_vTables[nDateSel]->rowCount();
    int nTarget=static_cast<int>(shipments2.size());
    if(nTarget<nNow)
    {
        while(m_vTables[nDateSel]->rowCount()>nTarget)
            m_vTables[nDateSel]->removeRow(0);
    }


    QString strValue,strDate,strProduct,strName;
    int st,index,nRow=0;
    double dblValue;
    for(itMap=shipments2.begin();itMap!=shipments2.end();itMap++)
    {
        st=itMap->first.indexOf(":");
        if(st>0)
        {
            strDate=itMap->first.left(st);
            strProduct=itMap->first.right(itMap->first.size()-st-1);
            Product* pProduct=nullptr;
            strName=gSystem->GetProductStruct(strProduct,&pProduct);
            if(pProduct!=nullptr)
                strName=pProduct->CName;
            else
                strName="";
            pShipment=&itMap->second;
            if(strDate.size()>0 && strProduct.size()>0)
            {
                if(m_vLabels[nDateSel]->text().size()<=0)
                    m_vLabels[nDateSel]->setText(strDate);

                if(nRow>=m_vTables[nDateSel]->rowCount())
                    m_vTables[nDateSel]->insertRow(nRow);

                index=0;
                //Date
                /*
                pItem=m_vTables[nDateSel]->item(nRow,index);
                if(pItem==nullptr)
                {
                    if(itMap==shipments2.begin())
                        pItem=new QTableWidgetItem(strDate);
                    else
                        pItem=new QTableWidgetItem("");
                    m_vTables[nDateSel]->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=m_vTables[nDateSel]->item(nRow,index);
                    if(itMap==shipments2.begin())
                        pItem->setText(strDate);
                    else
                        pItem->setText("");
                }
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
                */
                //Product   產品
                //index++;
                pItem=m_vTables[nDateSel]->item(nRow,index);
                if(pItem==nullptr)
                {
                    pItem=new QTableWidgetItem(strProduct);
                    m_vTables[nDateSel]->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=m_vTables[nDateSel]->item(nRow,index);
                    pItem->setText(strProduct);
                }
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

                //Name
                /*
                index++;
                pItem=m_vTables[nDateSel]->item(nRow,index);
                if(pItem==nullptr)
                {
                    pItem=new QTableWidgetItem(strName);
                    m_vTables[nDateSel]->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=m_vTables[nDateSel]->item(nRow,index);
                    pItem->setText(strName);
                }
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
                */
                //Export    出貨
                index++;
                pItem=m_vTables[nDateSel]->item(nRow,index);
                strValue=QString("%1").arg(pShipment->OutCount);
                if(pItem==nullptr)
                {
                    pItem=new QTableWidgetItem(strValue);
                    m_vTables[nDateSel]->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=m_vTables[nDateSel]->item(nRow,index);
                    pItem->setText(strValue);
                }
                pItem->setTextAlignment(Qt::AlignCenter);

                //WareHouse // 庫存
                index++;
                pItem=m_vTables[nDateSel]->item(nRow,index);
                strValue=QString("%1").arg(
                            QVariant(pShipment->StockCount).toInt());
                if(pItem==nullptr)
                {
                    pItem=new QTableWidgetItem(strValue);
                    m_vTables[nDateSel]->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=m_vTables[nDateSel]->item(nRow,index);
                    pItem->setText(strValue);
                }
                pItem->setTextAlignment(Qt::AlignCenter);
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
                if(pShipment->StockCount<0)
                {
                    QBrush brush(Qt::red);
                    pItem->setForeground(brush);
                }
                else
                {
                    QBrush brush(Qt::black);
                    pItem->setForeground(brush);
                }


                //Rate
                index++;
                pItem=m_vTables[nDateSel]->item(nRow,index);
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
                    m_vTables[nDateSel]->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=m_vTables[nDateSel]->item(nRow,index);
                    pItem->setText(strValue);
                }
                pItem->setTextAlignment(Qt::AlignCenter);
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
                if(strValue.size()>0 && dblValue<0)
                {
                    QBrush brush(Qt::red);
                    pItem->setForeground(brush);
                }
                else
                {
                    QBrush brush(Qt::black);
                    pItem->setForeground(brush);
                }

                //Missing
                index++;
                pItem=m_vTables[nDateSel]->item(nRow,index);
                if(pShipment->OutCount<=0)
                    strValue="";
                else
                    strValue=QVariant(pShipment->OutCount-pShipment->StockCount).toString();
                if(pItem==nullptr)
                {
                    pItem=new QTableWidgetItem(strValue);
                    m_vTables[nDateSel]->setItem(nRow,index,pItem);
                }
                else
                {
                    pItem=m_vTables[nDateSel]->item(nRow,index);
                    pItem->setText(strValue);
                }
                pItem->setTextAlignment(Qt::AlignCenter);
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
                if(strValue.size()>0 && (pShipment->OutCount-pShipment->StockCount)<0)
                {
                    QBrush brush(Qt::red);
                    pItem->setForeground(brush);
                }
                else
                {
                    QBrush brush(Qt::black);
                    pItem->setForeground(brush);
                }

                nRow++;
            }
        }
    }

    m_vTables[nDateSel]->resizeColumnsToContents();
    m_vTables[nDateSel]->resizeRowsToContents();
}

void vOutputerEx::RelistShipsFromDB(size_t nDateSel)
{
    std::map<QString,QString>::iterator itProduct;
    QTableWidgetItem* pItem;
    QStringList lstProduct;
    QDate   DateSelect;
    QString strDateSelect;
    int nId=nDateSel % m_vTables.size();
    if(nId>(m_vTables.size()-1))
        return;
    if(nDateSel>(m_lstDates.size()-1))
        return;
    strDateSelect=m_lstDates[nDateSel];
    DateSelect=QDate::fromString(strDateSelect,"yyyy/MM/dd");
    m_vLabels[nId]->setText(strDateSelect);

    std::map<QString,ShipTable>::iterator itMap;
    std::map<QString,ShipTable> shipments;
    ShipTable* pShip;
    gSystem->GetShipMentsFromDB(gSystem->m_TypeSelect,strDateSelect,shipments);


    int nNow=m_vTables[nId]->rowCount();
    int nTarget=static_cast<int>(shipments.size());
    if(nTarget<nNow)
    {
        while(m_vTables[nId]->rowCount()>nTarget)
            m_vTables[nId]->removeRow(0);
    }

    double dblValue;
    QString strValue;
    int index,nRow=0;
    for(itMap=shipments.begin();itMap!=shipments.end();itMap++)
    {
        pShip=&itMap->second;
        if(pShip->ship.date.isNull())
            continue;

        if(nRow>=m_vTables[nId]->rowCount())
            m_vTables[nId]->insertRow(nRow);

        index=0;

        //Product   產品
        pItem=m_vTables[nId]->item(nRow,index);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem(pShip->ProductID);
            m_vTables[nId]->setItem(nRow,index,pItem);
        }
        else
        {
            pItem=m_vTables[nId]->item(nRow,index);
            pItem->setText(pShip->ProductID);
        }
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        //Export    出貨
        index++;
        pItem=m_vTables[nId]->item(nRow,index);
        strValue=QString("%1").arg(pShip->ship.OutCount);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem(strValue);
            m_vTables[nId]->setItem(nRow,index,pItem);
        }
        else
        {
            pItem=m_vTables[nId]->item(nRow,index);
            pItem->setText(strValue);
        }
        pItem->setTextAlignment(Qt::AlignCenter);

        //WareHouse // 庫存
        index++;
        pItem=m_vTables[nId]->item(nRow,index);
        strValue=QString("%1").arg(
                    QVariant(pShip->ship.StockCount).toInt());
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem(strValue);
            m_vTables[nId]->setItem(nRow,index,pItem);
        }
        else
        {
            pItem=m_vTables[nId]->item(nRow,index);
            pItem->setText(strValue);
        }
        pItem->setTextAlignment(Qt::AlignCenter);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
        if(pShip->ship.StockCount<0)
        {
            QBrush brush(Qt::red);
            pItem->setForeground(brush);
        }
        else
        {
            QBrush brush(Qt::black);
            pItem->setForeground(brush);
        }


        //Rate
        index++;
        pItem=m_vTables[nId]->item(nRow,index);
        if(pShip->rate<-100)
            strValue="";
        else
        {
            strValue=QString::number(100.0f * pShip->rate,'f',2);
            strValue += "%";
        }
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem(strValue);
            m_vTables[nId]->setItem(nRow,index,pItem);
        }
        else
        {
            pItem=m_vTables[nId]->item(nRow,index);
            pItem->setText(strValue);
        }
        pItem->setTextAlignment(Qt::AlignCenter);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
        if(strValue.size()>0)
        {
            if(pShip->rate<0)
            {
                QBrush brush(Qt::red);
                pItem->setForeground(brush);
            }
            else
            {
                QBrush brush(Qt::black);
                pItem->setForeground(brush);
            }
        }

        //Missing
        index++;
        pItem=m_vTables[nId]->item(nRow,index);
        if(pShip->diff<-100)
            strValue="";
        else
            strValue=QVariant(pShip->diff).toString();
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem(strValue);
            m_vTables[nId]->setItem(nRow,index,pItem);
        }
        else
        {
            pItem=m_vTables[nId]->item(nRow,index);
            pItem->setText(strValue);
        }
        pItem->setTextAlignment(Qt::AlignCenter);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
        if(strValue.size()>0)
        {
            if(pShip->diff<0)
            {
                QBrush brush(Qt::red);
                pItem->setForeground(brush);
            }
            else
            {
                QBrush brush(Qt::black);
                pItem->setForeground(brush);
            }
        }

        nRow++;


    }




    m_vTables[nId]->resizeColumnsToContents();
    m_vTables[nId]->resizeRowsToContents();
}


void vOutputerEx::on_btnUpdate_clicked()
{
    QString strPath="D:/";
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        strPath,
        tr("Excel Files (*.XLSX *.xlsx);;All Files (*)"));
    if (fileName.isEmpty())
        return;


    if(gSystem->ImportShipmentDateFile(fileName))
        RelistProducts();
    else
        QMessageBox::warning(nullptr,tr("Warm"),tr("Import Failed!"));
}

void vOutputerEx::on_btnExport_clicked()
{
    QString strPath;
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Export File"),
        "",
        tr("Excel Files (*.XLSX *.xlsx);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    std::vector<ShipTable> vShipments;
    ShipTable* pShip;
    //gSystem->GetShipMentsFromDB(gSystem->m_TypeSelect,vShipments);
    gSystem->GetShipMentsFromDB(vShipments);
    if(vShipments.size()<=0)
        return;

    // 創建一個QXlsx::Document對象
    QXlsx::Document xlsx;

    // 在工作表中寫入數據
    xlsx.write(QXlsx::CellReference(1,1),"Date");
    xlsx.write(QXlsx::CellReference(1,2),"Type");
    xlsx.write(QXlsx::CellReference(1,3),"Mes");
    xlsx.write(QXlsx::CellReference(1,4),"Count");
    xlsx.write(QXlsx::CellReference(1,5),"Product");

    Product* pP;
    int nRow=vShipments.size();
    for(int i=0;i<nRow;i++)
    {
        pShip=&vShipments[i];
        xlsx.write(QXlsx::CellReference(2+i,1),pShip->ship.date.toString("yyyy/MM/dd"));
        xlsx.write(QXlsx::CellReference(2+i,2),pShip->TypeID);
        xlsx.write(QXlsx::CellReference(2+i,3),pShip->ProductID);
        xlsx.write(QXlsx::CellReference(2+i,4),pShip->ship.OutCount);
        gSystem->GetProductStruct(pShip->ProductID,&pP);
        if(pP!=nullptr)
            xlsx.write(QXlsx::CellReference(2+i,5),pP->CName);
    }

    // 保存Excel檔案
    xlsx.saveAs(fileName);
}


void vOutputerEx::on_VBar_valueChanged(int value)
{
    this->RelistProductsByIndex(value);
}
