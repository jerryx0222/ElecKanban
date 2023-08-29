#include "dlgbackselect.h"
#include "ui_dlgbackselect.h"
#include "helcsignage.h"

extern HElcSignage* gSystem;

dlgBackSelect::dlgBackSelect(QString *strType,QString *strProduct,std::vector<QString>* vProducts,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgBackSelect)
{
    ui->setupUi(this);

    m_pStrType=strType;
    m_pStrProduct=strProduct;
    m_pVProducts=vProducts;

    QStringList labels;
    labels.push_back("ProductID");
    labels.push_back("ProductName");

    ui->tbProduct->setColumnCount(labels.size());
    ui->tbProduct->setHorizontalHeaderLabels(labels);
    ui->tbProduct->setSelectionMode(QAbstractItemView::SingleSelection);


    connect(&m_Timer,SIGNAL(timeout()),this,SLOT(OnReloadTypes()));
    m_Timer.start(200);
    //ReloadTypes();




}

dlgBackSelect::~dlgBackSelect()
{
    delete ui;
}

void dlgBackSelect::OnReloadTypes()
{
    ReloadTypes();
    m_Timer.stop();
}


void dlgBackSelect::ReloadTypes()
{
    QString strType;
    std::map<QString, QString>::iterator itMap;
    std::map<QString, QString> datas;

    int nSelect=0,nIndex=0;
    ui->cmbType->clear();
    if(gSystem->GetTypeLists(datas))
    {
        for(itMap=datas.begin();itMap!=datas.end();itMap++)
        {
            if(itMap->first==gSystem->m_TypeSelect)
                nSelect=nIndex;
           strType=QString("%1(%2)").arg(itMap->first).arg(itMap->second);
           ui->cmbType->addItem(strType);
           nIndex++;
        }
    }
    ui->cmbType->setCurrentIndex(nSelect);
    RelistProducts();
}

void dlgBackSelect::RelistProducts()
{
    int pos;
    QString strType=ui->cmbType->currentText();
    m_mapTotalProducts.clear();

    pos=strType.indexOf("(");
    if(pos<=0)
    {
        while(ui->tbProduct->rowCount()>0)
            ui->tbProduct->removeRow(0);
        return;
    }


    //m_mapTotalProducts

    std::map<QString,std::map<QString,QString>>::iterator itTotal;
    std::map<QString,QString>::iterator itMap;
    std::map<QString,QString> mapDatas;
    strType=strType.left(pos);
    gSystem->CopyProducts(strType,mapDatas);
    if(mapDatas.size()>0)
    {
        //gSystem->m_TypeSelect=strType;
        itMap=mapDatas.begin();
        m_mapTotalProducts.insert(std::make_pair(itMap->first,mapDatas));
        ReCheckPoints(itMap->first);
    }


    while(ui->tbProduct->rowCount()>m_mapTotalProducts.size())
        ui->tbProduct->removeRow(0);


    int index=0;
    QTableWidgetItem* pItem;
    std::map<QString,QString>* pMembers;
    for(itTotal=m_mapTotalProducts.begin();itTotal!=m_mapTotalProducts.end();itTotal++)
    {
        pMembers=&itTotal->second;
        itMap=pMembers->begin();
        if(index>=ui->tbProduct->rowCount())
            ui->tbProduct->insertRow(index);
        pItem=ui->tbProduct->item(index,0);
        if(pItem!=nullptr)
            pItem->setText(itTotal->first);
        else
        {
            pItem=new QTableWidgetItem(itTotal->first);
            ui->tbProduct->setItem(index,0,pItem);
            pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
        }

        pItem=ui->tbProduct->item(index,1);
        if(pItem!=nullptr)
            pItem->setText(itMap->second);
        else
        {
            pItem=new QTableWidgetItem(itMap->second);
            ui->tbProduct->setItem(index,1,pItem);
            pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
        }
        index++;
    }




    ui->tbProduct->resizeRowsToContents();
    ui->tbProduct->resizeColumnsToContents();
}

void dlgBackSelect::ReCheckPoints(QString )
{
    std::map<QString,int>::iterator itPart1;
    std::map<QString,QString>::iterator itMap,itMap2;
    std::map<QString,std::vector<QString>>::iterator itPart2,itPart3;
    std::map<QString,std::map<QString,QString>>::iterator itTotal;
    std::map<QString,QString> *pMapProduct=nullptr;
    itTotal=m_mapTotalProducts.begin();
    if(itTotal!=m_mapTotalProducts.end())
        pMapProduct=&itTotal->second;
    if(pMapProduct==nullptr)
        return;

    std::map<QString,std::map<QString,QString>> mapTemp;
    Part* pP=nullptr;
    for(itMap=pMapProduct->begin();itMap!=pMapProduct->end();itMap++)
    {
        Product* pProduct=nullptr;
        gSystem->GetProductStruct(itMap->first,&pProduct);
        if(pProduct!=nullptr)
        {
            if(itMap==pMapProduct->begin())
            {
                for(itPart1=pProduct->parts.begin();itPart1!=pProduct->parts.end();itPart1++)
                {
                    std::map<QString,QString> memDatas;
                    pP=nullptr;
                    gSystem->GetPartStruct(itPart1->first,&pP);
                    if(pP!=nullptr)
                    {
                        memDatas.insert(std::make_pair(pP->PartID,pP->CName));
                        mapTemp.insert(std::make_pair(itPart1->first,memDatas));
                    }
                }
            }
            else
            {
                itTotal=mapTemp.begin();
                if(itTotal!=mapTemp.end())
                {
                    for(itPart1=pProduct->parts.begin();itPart1!=pProduct->parts.end();itPart1++)
                    {
                        pP=nullptr;
                        gSystem->GetPartStruct(itPart1->first,&pP);
                        if(pP!=nullptr && itTotal!=mapTemp.end())
                        {
                            std::map<QString,QString> *pMemDatas=&itTotal->second;
                            pMemDatas->insert(std::make_pair(pP->PartID,pP->CName));
                        }
                        else
                            break;
                        itTotal++;
                    }
                }
            }
        }
    }

    for(itTotal=mapTemp.begin();itTotal!=mapTemp.end();itTotal++)
    {
        m_mapTotalProducts.insert(std::make_pair(itTotal->first,itTotal->second));
    }
}

void dlgBackSelect::on_btnCan_clicked()
{
    this->close();
}

void dlgBackSelect::on_btnOK_clicked()
{
    std::map<QString,QString>::iterator itMap;
   if(m_pVProducts!=nullptr)
   {
        QTableWidgetItem* pItem=ui->tbProduct->item(ui->tbProduct->currentRow(),0);
        if(pItem!=nullptr)
        {
            std::map<QString,QString>::iterator itData;
            std::map<QString,std::map<QString,QString>>::iterator itMap=m_mapTotalProducts.find(pItem->text());
            if(itMap!=m_mapTotalProducts.end())
            {
                for(itData=itMap->second.begin();itData!=itMap->second.end();itData++)
                {
                    this->m_pVProducts->push_back(itData->first);
                }
            }

            if(m_pStrProduct!=nullptr)
            {
                pItem=ui->tbProduct->item(ui->tbProduct->currentRow(),0);
                if(pItem!=nullptr)
                    (*m_pStrProduct)=pItem->text();
            }

            if(m_pStrType!=nullptr)
            {
                QString strType=ui->cmbType->currentText();
                int pos=strType.indexOf("(");
                if(pos>0)
                    *m_pStrType=strType.left(pos);
            }
        }

   }


    this->close();
}

void dlgBackSelect::on_tbProduct_itemDoubleClicked(QTableWidgetItem *)
{
    on_btnOK_clicked();
}


void dlgBackSelect::on_cmbType_activated(int index)
{
    RelistProducts();
}




/***********************************************************************************************************/
dlgBackSelectEx::dlgBackSelectEx(QString *strType,QString* strProduct, std::vector<QString> *vProducts, QWidget *parent)
    :dlgBackSelect(strType,strProduct,vProducts,parent)
{

}

dlgBackSelectEx::~dlgBackSelectEx()
{

}

void dlgBackSelectEx::RelistProducts()
{
    int pos;
    QString strType=ui->cmbType->currentText();
    m_mapTotalProducts.clear();

    pos=strType.indexOf("(");
    if(pos<=0)
    {
        while(ui->tbProduct->rowCount()>0)
            ui->tbProduct->removeRow(0);
        return;
    }

    std::map<QString,std::map<QString,QString>>::iterator itTotal;
    std::map<QString,QString>::iterator itMap;
    std::map<QString,QString> mapDatas;
    strType=strType.left(pos);
    gSystem->CopyProducts(strType,mapDatas);
    if(mapDatas.size()>0)
    {
        //gSystem->m_TypeSelect=strType;
        itMap=mapDatas.begin();
        m_mapTotalProducts.insert(std::make_pair(itMap->first,mapDatas));
        ReCheckPoints(itMap->first);
    }


    while(ui->tbProduct->rowCount()>m_mapTotalProducts.size())
        ui->tbProduct->removeRow(0);


    int index=0;
    QTableWidgetItem* pItem;
    std::map<QString,QString>* pMembers;
    for(itTotal=m_mapTotalProducts.begin();itTotal!=m_mapTotalProducts.end();itTotal++)
    {
        pMembers=&itTotal->second;
        for(itMap=pMembers->begin();itMap!=pMembers->end();itMap++)
        {
            if(index>=ui->tbProduct->rowCount())
                ui->tbProduct->insertRow(index);
            pItem=ui->tbProduct->item(index,0);
            if(pItem!=nullptr)
                pItem->setText(itTotal->first);
            else
            {
                pItem=new QTableWidgetItem(itMap->first);
                ui->tbProduct->setItem(index,0,pItem);
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
                //pItem->setSizeHint(QSize(0, 0));

            }

            pItem=ui->tbProduct->item(index,1);
            if(pItem!=nullptr)
                pItem->setText(itMap->second);
            else
            {
                pItem=new QTableWidgetItem(itMap->second);
                ui->tbProduct->setItem(index,1,pItem);
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
                //pItem->setSizeHint(QSize(0, 0));
            }


            index++;
        }
    }

    ui->tbProduct->resizeColumnsToContents();
    ui->tbProduct->resizeRowsToContents();

}
