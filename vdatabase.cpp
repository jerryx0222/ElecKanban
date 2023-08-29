#include "vdatabase.h"
#include "ui_vdatabase.h"
#include "helcsignage.h"
#include "dlgdateselect.h"
#include <QDialog>

extern HElcSignage* gSystem;

vDataBase::vDataBase(QWidget *parent) :
    HTabBase(parent),
    ui(new Ui::vDataBase)
{
    ui->setupUi(this);
}

vDataBase::~vDataBase()
{
    delete ui;
}

void vDataBase::OnInit()
{
    /*
        dbProduct,
        dbShipment,
        dbPart,
        dbProductLink,
        dbProcess,
        dbProcessLink,
     */
    ui->cmbDB->addItem(tr("Product"));
    ui->cmbDB->addItem(tr("Shipment"));
    ui->cmbDB->addItem(tr("Part"));
    ui->cmbDB->addItem(tr("ProductLink"));
    ui->cmbDB->addItem(tr("Process"));
    ui->cmbDB->addItem(tr("ProcessLink"));

    CreateProductDB();
    RelistProducts();
}

void vDataBase::CreateProductDB()
{
    QStringList titles;
    titles.push_back("ProductID");
    titles.push_back("CName");
    titles.push_back("FirstPass");
    titles.push_back("Stock");

    ReCreateTable(titles);



}

void vDataBase::CreateShipmentDB()
{
    QStringList titles;
    titles.push_back("ProductID");
    titles.push_back("OutDate");
    titles.push_back("OutCount");


    ReCreateTable(titles);
}

void vDataBase::CreatePartDB()
{
    QStringList titles;
    titles.push_back("PartID");
    titles.push_back("CName");
    titles.push_back("Specification");
    titles.push_back("Stock");


    ReCreateTable(titles);
}

void vDataBase::CreateProductLinkDB()
{
    QStringList titles;
    titles.push_back("ProductID");
    titles.push_back("PartID");
    titles.push_back("PartCount");


    ReCreateTable(titles);
}

void vDataBase::CreateProcessDB()
{
    QStringList titles;
    titles.push_back("ProcessID");
    titles.push_back("CName");


    ReCreateTable(titles);
}

void vDataBase::CreateProcessLinkDB()
{
    QStringList titles;
    titles.push_back("ProductID");
    titles.push_back("ProcessID");
    titles.push_back("PartID");
    titles.push_back("theOrder");
    titles.push_back("DayTarget");
    titles.push_back("Stock");
    titles.push_back("Schedule");


    ReCreateTable(titles);
}

void vDataBase::ReCreateTable(QStringList &datas)
{
    int nColumn=ui->tbDataBase->columnCount();
    int nTarget=static_cast<int>(datas.size());
    if(nColumn<nTarget)
    {
        for(int i=nColumn;i<nTarget;i++)
            ui->tbDataBase->insertColumn(i);
    }
    else if(nColumn>nTarget)
    {
        for(int i=nColumn-1;i>=nTarget;i--)
            ui->tbDataBase->removeColumn(i);
    }

    ui->tbDataBase->setHorizontalHeaderLabels(datas);
    for(int i=0;i<nTarget;i++)
    {
        ui->tbDataBase->setColumnWidth(i,250);
    }

}

void vDataBase::RelistDatas(QString strSQL, std::map<QString,QString> &outDatas)
{
    std::map<QString,QString>::iterator itMap;
    HRecordsetSQLite rs;
    QString strValue;
    if(gSystem==nullptr || !gSystem->m_db.Open())
        return;

    if(!rs.ExcelSQL(strSQL.toStdWString(),&gSystem->m_db))
    {
        gSystem->m_db.Close();
        return;
    }

    while(!rs.isEOF())
    {
        for(itMap=outDatas.begin();itMap!=outDatas.end();itMap++)
        {
            strValue.clear();
            if(rs.GetValue(itMap->first.toStdWString(),strValue))
            {
                itMap->second=strValue;
            }
        }

        rs.MoveNext();
    }

    gSystem->m_db.Close();
}

void vDataBase::RelistProducts()
{
    while(ui->tbDataBase->rowCount()>0)
        ui->tbDataBase->removeRow(0);
    std::map<QString,Product*>::iterator itMap;
    Product* pProduct;
    QTableWidgetItem* pItem;
    int nRow;
    for(itMap=gSystem->m_mapProducts.begin();itMap!=gSystem->m_mapProducts.end();itMap++)
    {
        pProduct=itMap->second;
        nRow=ui->tbDataBase->rowCount();
        ui->tbDataBase->insertRow(nRow);
        pItem=new QTableWidgetItem(pProduct->ProductID);
        ui->tbDataBase->setItem(nRow,0,pItem);
        pItem=new QTableWidgetItem(pProduct->CName);
        ui->tbDataBase->setItem(nRow,1,pItem);
        pItem=new QTableWidgetItem(QString("%1").arg(pProduct->FirstPass));
        ui->tbDataBase->setItem(nRow,2,pItem);
        pItem=new QTableWidgetItem(QString("%1").arg(pProduct->Stock));
        ui->tbDataBase->setItem(nRow,3,pItem);

    }
}


void vDataBase::SaveProducts()
{
    if(ui->cmbDB->currentIndex()!=dbProduct)
        return;

    QTableWidgetItem* pItem;
    Product* pProduct;
    QString strID;
    std::map<QString,Product*>::iterator itMap;
    std::map<QString,Product*> datas;
    int nRow=ui->tbDataBase->rowCount();

    for(int i=0;i<nRow;i++)
    {
        pItem=ui->tbDataBase->item(i,0);
        if(pItem!=nullptr && pItem->text().size()>0)
        {
            strID=pItem->text();
            itMap=datas.find(strID);
            if(itMap!=datas.end())
                pProduct=itMap->second;
            else
            {
                pProduct=new Product();
                pProduct->ProductID=strID;
                datas.insert(std::make_pair(pProduct->ProductID,pProduct));
            }

            pItem=ui->tbDataBase->item(i,1);
            if(pItem!=nullptr)
                pProduct->CName=pItem->text();

            pItem=ui->tbDataBase->item(i,2);
            if(pItem!=nullptr)
                pProduct->FirstPass=pItem->text().toDouble();
            if(pProduct->FirstPass>1) pProduct->FirstPass=1;
            if(pProduct->FirstPass<0) pProduct->FirstPass=0;

            pItem=ui->tbDataBase->item(i,3);
            if(pItem!=nullptr)
                pProduct->Stock=pItem->text().toInt();
        }
    }

    //gSystem->SaveProducts(datas);
    on_btnLoad_clicked();
}


void vDataBase::on_cmbDB_activated(int index)
{
    std::map<QString,QString> outDatas;

    while(ui->tbDataBase->rowCount()>0)
        ui->tbDataBase->removeRow(0);

    switch(index)
    {
    case dbProduct:
        CreateProductDB();
        RelistProducts();
        break;
    case dbShipment:
        CreateShipmentDB();
        RelistDatas("select * from Shipment",outDatas);
        break;
    case dbPart:
        CreatePartDB();
        RelistDatas("select * from Part",outDatas);
        break;
    case dbProductLink:
        CreateProductLinkDB();
        RelistDatas("select * from ProductLink",outDatas);
        break;
    case dbProcess:
        CreateProcessDB();
        RelistDatas("select * from Process",outDatas);
        break;
    case dbProcessLink:
        CreateProcessLinkDB();
        RelistDatas("select * from ProcessLink",outDatas);
        break;
    }
}

void vDataBase::on_btnLoad_clicked()
{
    on_cmbDB_activated(ui->cmbDB->currentIndex());
}

void vDataBase::on_btnNew_clicked()
{
    QTableWidgetItem* pItem;
    int nRow=ui->tbDataBase->rowCount();
    if(nRow>0)
    {
        pItem=ui->tbDataBase->item(nRow-1,0);
        if(pItem==nullptr || pItem->text().size()<=0)
            return;
    }
    ui->tbDataBase->insertRow(nRow);
    pItem=new QTableWidgetItem();
    ui->tbDataBase->setItem(nRow,0,pItem);

    switch(ui->cmbDB->currentIndex())
    {
    case dbProduct:
        break;
    case dbShipment:

        break;
    case dbPart:

        break;
    case dbProductLink:

        break;
    case dbProcess:

        break;
    case dbProcessLink:

        break;
    }

}

void vDataBase::on_btnSave_clicked()
{
    switch(ui->cmbDB->currentIndex())
    {
    case dbProduct:
        SaveProducts();
        break;
    case dbShipment:

        break;
    case dbPart:

        break;
    case dbProductLink:

        break;
    case dbProcess:

        break;
    case dbProcessLink:

        break;
    }
}

void vDataBase::on_btnDelete_clicked()
{
    int nSelect=ui->tbDataBase->currentRow();
    QTableWidgetItem* pItem=ui->tbDataBase->item(nSelect,0);

    switch(ui->cmbDB->currentIndex())
    {
    case dbProduct:
        if(pItem!=nullptr)
        {
            gSystem->DeleteProduct(pItem->text());
            on_btnLoad_clicked();
        }
        break;
    case dbShipment:

        break;
    case dbPart:

        break;
    case dbProductLink:

        break;
    case dbProcess:

        break;
    case dbProcessLink:

        break;
    }
}

void vDataBase::on_tbDataBase_cellClicked(int row, int column)
{
    std::map<QString,Product*>::iterator itP1;
    QTableWidgetItem* pItem;
    QComboBox *pCombo;
    QPushButton *pButton;

    switch(ui->cmbDB->currentIndex())
    {
    case dbProduct:
        break;
    case dbShipment:
        switch(column)
        {
        case 0:
            // Product ID
            pItem=ui->tbDataBase->item(row,column);
            if(pItem!=nullptr)
            {
                pCombo=new QComboBox();
                for(itP1=gSystem->m_mapProducts.begin();itP1!=gSystem->m_mapProducts.end();itP1++)
                {
                    pCombo->addItem(itP1->first);
                }
                QModelIndex index = ui->tbDataBase->model()->index(row,column);
                ui->tbDataBase->setIndexWidget(index,pCombo);
            }
            break;
        case 1:
            // Out Date
            QModelIndex index = ui->tbDataBase->model()->index(row,column);
            pButton = qobject_cast<QPushButton*>(ui->tbDataBase->indexWidget(index));
            if(pButton==nullptr)
            {
                pButton=new QPushButton();
                connect(pButton, &QPushButton::clicked, this, &vDataBase::buttonSelect);
                pButton->setText(tr("select"));
                ui->tbDataBase->setIndexWidget(index,pButton);
            }
            break;
        }
        break;
    case dbPart:

        break;
    case dbProductLink:

        break;
    case dbProcess:

        break;
    case dbProcessLink:

        break;
    }
}

void vDataBase::buttonSelect()
{
    QString strSelect;
    int nRow,nCol;
    dlgDateSelect* pNew=new dlgDateSelect(&strSelect,&nRow,&nCol,this);
    pNew->setModal(true);
    pNew->setWindowTitle(tr("Date select"));


    //QPushButton *button1=dynamic_cast<QPushButton*>(ui->treProcess->itemWidget(pItem,0));
    int result=pNew->exec();
    if(result==QDialog::Accepted)
    {

    }
    delete pNew;
}

