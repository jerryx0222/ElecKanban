#include "vpartprocess.h"
#include "ui_vpartprocess.h"
#include "helcsignage.h"
#include "dlgdatilyinfo.h"
#include "dlgselprocess.h"
#include "dlgpartselect.h"
#include "dlgtypeselect.h"

extern HElcSignage* gSystem;

vPartProcess::vPartProcess(QWidget *parent) :
    HTabBase(parent),
    ui(new Ui::vPartProcess)
{
    ui->setupUi(this);

    m_selProduct=nullptr;
    m_selPart=nullptr;
}

vPartProcess::~vPartProcess()
{
    delete ui;
}

void vPartProcess::OnInit()
{
    CreateTable();
    RelistProducts();
}

void vPartProcess::OnShowTab(bool show)
{
    if(!show)
        return;

    QString strName;
    if(gSystem->m_TypeSelect.size()>0)
    {
        strName=gSystem->GetTypeFullName(gSystem->m_TypeSelect);
        if(strName.size()>0)
        {
            strName=QString("%2(%1)").arg(strName).arg(gSystem->m_TypeSelect);
            ui->lblType->setText(strName);
            gSystem->m_TypeSelect=gSystem->m_TypeSelect;
            RelistProducts();
        }
    }
    RelistChilds();
    RelistProcess();

}

void vPartProcess::RelistProducts()
{
    std::map<QString,QString>::iterator itMap;
    std::map<QString,QString> datas;
    QTableWidgetItem* pItem;
    int index=0;
    if(gSystem->m_TypeSelect.size()<=0)
        return;

    gSystem->CopyProducts(gSystem->m_TypeSelect,datas);
    while(ui->tbProducts->rowCount()>0)
        ui->tbProducts->removeRow(0);
    for(itMap=datas.begin();itMap!=datas.end();itMap++)
    {
        ui->tbProducts->insertRow(index);
        pItem=new QTableWidgetItem(itMap->first);
        ui->tbProducts->setItem(index,0,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        pItem=new QTableWidgetItem(itMap->second);
        ui->tbProducts->setItem(index,1,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        index++;
    }

    ui->tbProducts->resizeRowsToContents();
    ui->tbProducts->resizeColumnsToContents();

}

void vPartProcess::CreateTable()
{
    QStringList strTitles;
    strTitles.push_back(tr("ProductID"));
    strTitles.push_back(tr("Name"));

    ui->tbProducts->setColumnCount(strTitles.size());
    ui->tbProducts->setHorizontalHeaderLabels(strTitles);
    ui->tbProducts->setColumnWidth(0,240);
    ui->tbProducts->setColumnWidth(1,450);
    ui->tbProducts->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tbProducts->setSelectionBehavior(QAbstractItemView::SelectRows);


    strTitles.clear();
    strTitles.push_back(tr("ID"));
    strTitles.push_back(tr("Name"));
    strTitles.push_back(tr("Spec."));
    strTitles.push_back(tr("Count"));
    ui->trProduct->setColumnCount(strTitles.size());
    ui->trProduct->setHeaderLabels(strTitles);
    ui->trProduct->setColumnWidth(0,300);
    ui->trProduct->setColumnWidth(1,340);
    ui->trProduct->setColumnWidth(2,290);
    ui->trProduct->setColumnWidth(3,80);

    strTitles.clear();
    strTitles.push_back(tr("ID"));
    strTitles.push_back(tr("Name"));
    strTitles.push_back(tr("Stock"));
    strTitles.push_back(tr("Target"));
    strTitles.push_back(tr("PartID"));
    ui->tbProcess->setColumnCount(strTitles.size());
    ui->tbProcess->setHorizontalHeaderLabels(strTitles);
    ui->tbProcess->setColumnWidth(0,130);
    ui->tbProcess->setColumnWidth(1,250);
    ui->tbProcess->setColumnWidth(2,100);
    ui->tbProcess->setColumnWidth(3,100);
    ui->tbProcess->setColumnWidth(4,250);
    ui->tbProcess->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tbProcess->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void vPartProcess::on_tbProducts_cellClicked(int , int )
{
    RelistChilds();

    int nRow=ui->tbProducts->currentRow();
    QTableWidgetItem* pItem=ui->tbProducts->item(nRow,0);
    if(pItem==nullptr)
        return;

    QString strName=pItem->text();
    RelistProcess(strName);
}


void vPartProcess::RelistChilds()
{
    int nRow=ui->tbProducts->currentRow();
    QTableWidgetItem* pItem=ui->tbProducts->item(nRow,0);
    ui->trProduct->clear();
    if(pItem==nullptr)
        return;

    QString strProductID=pItem->text();
    Product *pP=nullptr;
    Part *pPart=nullptr;
    Part *pPart2=nullptr;
    if(!gSystem->GetProductStruct(strProductID,&pP))
        return;


    m_selProduct=pP;

    QTreeWidgetItem *pTop;
    QTreeWidgetItem *pChild;
    std::map<QString,int>::iterator itMap,itC;
    for(itMap=pP->parts.begin();itMap!=pP->parts.end();itMap++)
    {
        QStringList dataTop;
        if(!gSystem->GetPartStruct(itMap->first,&pPart))
            continue;
        dataTop.push_back(pPart->PartID);                        // id
        dataTop.push_back(pPart->CName);                              // name
        dataTop.push_back(pPart->Specification);                              // name
        dataTop.push_back(QString("%1").arg(itMap->second));   // count
        pTop=new QTreeWidgetItem(ui->trProduct,dataTop);

        for(itC=pPart->parts.begin();itC!=pPart->parts.end();itC++)
        {
            QStringList dataIn;
            if(!gSystem->GetPartStruct(itC->first,&pPart2))
                continue;
            dataIn.push_back(pPart2->PartID);
            dataIn.push_back(pPart2->CName);
            dataIn.push_back(pPart2->Specification);
            dataIn.push_back(QString("%1").arg(itC->second));
            pChild=new QTreeWidgetItem(pTop,dataIn);

        }

    }

    ui->trProduct->expandAll();
}

void vPartProcess::RelistProcess(QString strProductID,QString strPartID)
{
    QTableWidgetItem* pItem;
    while(ui->tbProcess->rowCount()>0)
        ui->tbProcess->removeRow(0);

    std::list<ProcessInfo>::iterator itL;
    //QString strProductID=pItem->text();
    //std::list<ProcessInfo> datas;
    ProcessInfo* pInfo;
    ProcessInfo* pInfoP=nullptr;
    int index=0;
    m_Processes.clear();

    gSystem->GetPartProcess(strPartID,m_Processes);

    gSystem->GetPartStruct(strPartID,&m_selPart);
    gSystem->GetProductStruct(strProductID,&m_selProduct);

    for(itL=m_Processes.begin();itL!=m_Processes.end();itL++)
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

        if(pInfoP==nullptr || pInfoP->ProductID!=pInfo->ProductID)
            pItem=new QTableWidgetItem(pInfo->ProductID);
        else
            pItem=new QTableWidgetItem("");
        ui->tbProcess->setItem(index,4,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        pInfoP=pInfo;
        index++;
    }

    ui->tbProcess->resizeRowsToContents();
    ui->tbProcess->resizeColumnsToContents();
}

void vPartProcess::RelistProcess(QString strProductID)
{
    while(ui->tbProcess->rowCount()>0)
        ui->tbProcess->removeRow(0);
    m_Processes.clear();
    if(!gSystem->GetProductProcess(strProductID,m_Processes))
        return;
    m_selPart=nullptr;
    gSystem->GetProductStruct(strProductID,&m_selProduct);
    RelistProcess(m_Processes);
}

void vPartProcess::RelistProcess()
{
    int nRow=ui->tbProducts->currentRow();
    QTableWidgetItem* pItem=ui->tbProducts->item(nRow,0);
    if(pItem!=nullptr)
    {
        if(m_selPart!=nullptr)
            RelistProcess(pItem->text(),m_selPart->PartID);
        else
            RelistProcess(pItem->text());
    }
    else
    {
        while(ui->tbProcess->rowCount()>0)
            ui->tbProcess->removeRow(0);
    }
}

void vPartProcess::RelistProcess(std::list<ProcessInfo> &datas)
{
    QTableWidgetItem* pItem;
    int index=0;
    std::list<ProcessInfo>::iterator itL;
    ProcessInfo* pInfo=nullptr;
    ProcessInfo* pInfoP=nullptr;

    while(ui->tbProcess->rowCount()>0)
        ui->tbProcess->removeRow(0);

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

        if(pInfoP==nullptr || pInfoP->ProductID!=pInfo->ProductID)
            pItem=new QTableWidgetItem(pInfo->ProductID);
        else
            pItem=new QTableWidgetItem("");
        ui->tbProcess->setItem(index,4,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        pInfoP=pInfo;
        index++;
    }
    ui->tbProcess->resizeRowsToContents();
    ui->tbProcess->resizeColumnsToContents();
}

void vPartProcess::on_trProduct_itemClicked(QTreeWidgetItem *item, int )
{
    int nRow=ui->tbProducts->currentRow();
    QTableWidgetItem* pItem=ui->tbProducts->item(nRow,0);
    if(pItem==nullptr)
        return;

    QString strName=item->text(0);

    RelistProcess(pItem->text(),strName);

    bool bEnable=(item->parent()==nullptr);
    ui->btnInsert2->setEnabled(bEnable);
    ui->btnSave2->setEnabled(bEnable);
    ui->btnRemove2->setEnabled(bEnable);
}

void vPartProcess::on_btnPlot_clicked()
{
    dlgDatilyInfo* pNew=nullptr;

    if(m_selProduct!=nullptr)
    {
        if(m_selPart!=nullptr)
        {
            pNew=new dlgDatilyInfo(this);
            pNew->m_pPart=m_selPart;
            pNew->m_pProduct=m_selProduct;
        }
        else
        {
            pNew=new dlgDatilyInfo(this);
            pNew->m_pPart=nullptr;
            pNew->m_pProduct=m_selProduct;
        }
    }

    if(pNew!=nullptr)
    {
        pNew->setModal(true);
        pNew->setWindowTitle(tr("Plot"));

        int result=pNew->exec();
        if(result==QDialog::Accepted)
        {

        }

        //delete pNew;
    }

}

void vPartProcess::on_btnInsert_clicked()
{
    QTableWidgetItem* ptbProduct=ui->tbProducts->currentItem();
    QTreeWidgetItem* ptbPart=ui->trProduct->currentItem();
    if(gSystem->m_TypeSelect.size()<=0 || ptbProduct==nullptr)
        return;

    QTreeWidgetItem* pTop;
    QStringList dataTop;
    // new part
    if(ptbPart==nullptr)
    {
        // root
        int nTop=ui->trProduct->topLevelItemCount();
        for(int i=0;i<nTop;i++)
        {
            pTop=ui->trProduct->topLevelItem(i);
            if(pTop->text(0)=="NewID")
                return;
        }
        dataTop.push_back("NewID");
        dataTop.push_back("NewName");
        dataTop.push_back("None");
        dataTop.push_back("1");
        pTop=new QTreeWidgetItem(ui->trProduct,dataTop);
        //pTop->setFlags(pTop->flags() | Qt::ItemIsEditable); // 設定為可編輯
    }
    else
    {
        // child
        dataTop.push_back("NewID");
        dataTop.push_back("NewName");
        dataTop.push_back("None");
        dataTop.push_back("1");
        pTop=new QTreeWidgetItem(ptbPart,dataTop);
        //pTop->setFlags(pTop->flags() | Qt::ItemIsEditable); // 設定為可編輯

    }
}

void vPartProcess::on_btnRemove_clicked()
{
    int nRow=ui->tbProducts->currentRow();
    QTableWidgetItem* ptbProduct=ui->tbProducts->item(nRow,0);
    QTreeWidgetItem* ptbPart=ui->trProduct->currentItem();
    if(gSystem->m_TypeSelect.size()<=0 || ptbProduct==nullptr || ptbPart==nullptr)
        return;

    QString strProduct=ptbProduct->text();
    QTreeWidgetItem* pParent=ptbPart->parent();


    // root
    if(pParent==nullptr)
    {
        gSystem->RemovePart(strProduct,ptbPart->text(0));
    }
    else
    {
        // child
        gSystem->RemovePart(pParent->text(0),ptbPart->text(0));
    }
    RelistChilds();
}

void vPartProcess::on_btnSave_clicked()
{
    QTableWidgetItem* ptbProduct;
    QTreeWidgetItem* ptbPart;
    if(gSystem->m_TypeSelect.size()<=0)
        return;

    int nRow=ui->tbProducts->currentRow();
    ptbProduct=ui->tbProducts->item(nRow,0);
    if(ptbProduct==nullptr)
        return;

    ptbPart=ui->trProduct->currentItem();
    if(ptbPart==nullptr)
        return;

    if(ptbPart->text(0)=="NewID")
        return;

    Part newPart;
    newPart.PartID=ptbPart->text(0);
    newPart.CName=ptbPart->text(1);
    newPart.Specification=ptbPart->text(2);
    int nCount=ptbPart->text(3).toInt();
    newPart.Stock=0;
    newPart.TypeID=gSystem->m_TypeSelect;
    newPart.parts.clear();

    if(ptbPart->parent()==nullptr)
        gSystem->SavePart(gSystem->m_TypeSelect,ptbProduct->text(),&newPart,nCount);
    else
    {
        QTreeWidgetItem* pParent=ptbPart->parent();
        gSystem->SavePart(gSystem->m_TypeSelect,pParent->text(0),&newPart,nCount);
    }
    RelistChilds();
}

void vPartProcess::on_btnRelist_clicked()
{
    RelistChilds();
}

void vPartProcess::on_btnTypeSelect_clicked()
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
            RelistProducts();
        }
    }
    RelistChilds();
    RelistProcess();
}

void vPartProcess::on_trProduct_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Part* pPart=nullptr;
    int nRow=ui->tbProducts->currentRow();
    QTableWidgetItem* pItem=ui->tbProducts->item(nRow,0);
    if(pItem==nullptr)
        return;

    QString strSelect,strName=item->text(0);
    if(strName=="NewID")
    {
        dlgPartSelect* pNew=new dlgPartSelect(gSystem->m_TypeSelect,&strSelect,this);
        pNew->setModal(true);
        pNew->setWindowTitle(tr("Part Select"));

        pNew->exec();
        if(strSelect.size()>0)
        {
            if(gSystem->GetPartStruct(strSelect,&pPart))
            {
                item->setText(0,strSelect);
                item->setText(1,pPart->CName);
                item->setText(2,pPart->Specification);
                item->setText(3,"1");
            }
        }
        delete pNew;
    }
}

void vPartProcess::on_btnInsert2_clicked()
{
    QTableWidgetItem* pItem;
    QTableWidgetItem* ptbProduct=ui->tbProducts->item(ui->tbProducts->currentRow(),0);
    QTreeWidgetItem* ptbPart=ui->trProduct->currentItem();
    if(gSystem->m_TypeSelect.size()<=0 || ptbProduct==nullptr)
        return;

    QString strCopy=ui->edtProduct->text();
    if(strCopy.size()>0)
    {
        if(m_selPart!=nullptr)
        {
            if(ptbPart==nullptr)
                return;
            gSystem->CopyPartProcess(strCopy,ptbPart->text(0));
        }
        else
            gSystem->CopyProductProcess(strCopy,ptbProduct->text());

        RelistProcess();
        ui->edtProduct->setText("");
        return;
    }



    //if(ptbPart==nullptr)
    {
        // product
        int index=ui->tbProcess->rowCount();
        if(index>0)
        {
            if((index-1)==m_Processes.size())
                return;
        }
        ui->tbProcess->insertRow(index);
        pItem=new QTableWidgetItem(tr("NewProcessID"));
        ui->tbProcess->setItem(index,0,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        pItem=new QTableWidgetItem(tr("NewProcessName"));
        ui->tbProcess->setItem(index,1,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        pItem=new QTableWidgetItem(QString("%1").arg("0"));
        ui->tbProcess->setItem(index,2,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        pItem=new QTableWidgetItem(QString("%1").arg("0"));
        ui->tbProcess->setItem(index,3,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

        pItem=new QTableWidgetItem("");
        ui->tbProcess->setItem(index,4,pItem);
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);


        on_tbProcess_cellDoubleClicked(index,0);
    }

    ui->tbProcess->resizeRowsToContents();
    ui->tbProcess->resizeColumnsToContents();
}

void vPartProcess::on_btnRemove2_clicked()
{
    std::list<ProcessInfo>::iterator itL=m_Processes.begin();
    ProcessInfo* pInfo=nullptr;
    QString strProcess,strPart;
    int nRow=ui->tbProcess->currentRow();
    QTableWidgetItem* pItem=ui->tbProcess->item(nRow,0);
    if(pItem!=nullptr)
    {
        strProcess=pItem->text();
        for(int i=0;i<nRow;i++)
            itL++;
        if(itL!=m_Processes.end())
            pInfo=&(*itL);

        if(m_selPart!=nullptr && pInfo!=nullptr)
        {
            strPart=m_selPart->PartID;
            if(gSystem->RemoveProductProcess(strProcess,strPart,pInfo->ProductID))
                RelistProcess();
        }
        else if(m_selProduct!=nullptr)
        {
            strPart=m_selProduct->ProductID;
            if(gSystem->RemoveProductProcess(strProcess,strPart,strPart))
                RelistProcess();
        }


    }
    /*
    std::list<ProcessInfo>::iterator itL;// m_Processes;
    ProcessInfo* pInfo;
    QString strID;
    int diff;
    int nRow=ui->tbProcess->rowCount();
    int nTarget=static_cast<int>(m_Processes.size());
    if(nRow > nTarget)
    {
        diff=nRow-nTarget;
        for(int i=0;i<diff;i++)
            ui->tbProcess->removeRow(nRow-i);
        RelistProcess(m_Processes);
    }
    else if(nRow < nTarget)
    {
        RelistProcess(m_Processes);
    }
    else if(nRow==nTarget)
    {
        nRow=ui->tbProcess->currentRow();
        itL=m_Processes.begin();
        for(int i=0;i<nRow;i++)
            itL++;
        if(itL!=m_Processes.end())
        {
            pInfo=&(*itL);
            strID=pInfo->ProcessID;
            m_Processes.erase(itL);
            ui->tbProcess->removeRow(nRow);
        }
    }
    */
}

void vPartProcess::on_btnSave2_clicked()
{
    QString strPart,strOldPart;
    QTableWidgetItem* pItem;
    int nRow=ui->tbProcess->rowCount();
    m_Processes.clear();
    if(m_selProduct==nullptr)
        return;

    for(int i=0;i<nRow;i++)
    {
        ProcessInfo info;
        pItem=ui->tbProcess->item(i,0);
        if(pItem!=nullptr)
            info.ProcessID=pItem->text();

        pItem=ui->tbProcess->item(i,1);
        if(pItem!=nullptr)
            info.ProcessName=pItem->text();

        pItem=ui->tbProcess->item(i,2);
        if(pItem!=nullptr)
            info.Target=pItem->text().toInt();

        pItem=ui->tbProcess->item(i,3);
        if(pItem!=nullptr)
            info.Source=pItem->text().toInt();

        pItem=ui->tbProcess->item(i,4);
        strPart.clear();
        if(pItem!=nullptr)
        {
            if(m_selPart!=nullptr)
            {
                strPart=pItem->text();
                if(strPart.size()>0)
                    info.ProductID=strPart;
                else if(strOldPart.size()>0)
                    info.ProductID=strOldPart;
                else
                    info.ProductID=m_selPart->PartID;
            }
            else
                info.ProductID=m_selProduct->ProductID;
        }

        if(strPart.size()>0)
            strOldPart=strPart;
        info.order=i;
        info.DayTarget=0;
        m_Processes.push_back(info);



    }
    if(m_selPart!=nullptr)
    {
        gSystem->SavetProductProcess(m_selPart->PartID,m_Processes);
    }
    else
    {
        gSystem->SavetProductProcess(m_selProduct->ProductID,m_Processes);
    }
    RelistProcess(m_Processes);

}

void vPartProcess::on_btnRelist2_clicked()
{
    RelistProcess();
}

void vPartProcess::on_tbProcess_cellDoubleClicked(int row, int col)
{
    QTableWidgetItem* pItem;
    QString strSelect,strValue;
    Part*   pPart=nullptr;
    dlgSelProcess* pNew=nullptr;
    QTableWidgetItem* ptbProduct=ui->tbProducts->item(ui->tbProducts->currentRow(),0);
    //QTreeWidgetItem* ptbPart=ui->trProduct->currentItem();
    if(gSystem->m_TypeSelect.size()<=0 || ptbProduct==nullptr)
        return;


    if(col==0 || col==1)
    {
        pItem=ui->tbProcess->item(row,0);
        if(pItem!=nullptr)
        {
            pNew=new dlgSelProcess(&strSelect,&strValue,this);
            pNew->setModal(true);
            pNew->setWindowTitle(tr("Part Select"));
            pNew->exec();
            if(strSelect.size()>0)
            {
                pItem->setText(strSelect);
                pItem=ui->tbProcess->item(row,1);
                if(pItem!=nullptr)
                    pItem->setText(strValue);
                else
                    pItem->setText("");
            }
            delete pNew;
        }
    }
    else if(col==4)
    {
        pItem=ui->tbProcess->item(row,4);
        if(pItem!=nullptr)
        {
            strValue=pItem->text();
            dlgPartSelect* pNew2=new dlgPartSelect(gSystem->m_TypeSelect,&strSelect,this);
            pNew2->setModal(true);
            pNew2->setWindowTitle(tr("Part Select"));

            pNew2->exec();
            if(strSelect.size()>0 && gSystem->GetPartStruct(strSelect,&pPart))
                pItem->setText(pPart->PartID);
            else
                pItem->setText(strValue);
            delete pNew2;
        }
    }
}


void vPartProcess::on_btnProcessImport_clicked()
{


    dlgSelProcess* pNew=nullptr;
    QString strSelect,strValue;
    pNew=new dlgSelProcess(&strSelect,&strValue,this);
    pNew->setModal(true);
    pNew->setWindowTitle(tr("Part Select"));
    pNew->exec();
    if(strSelect.size()>0)
    {

    }
    delete pNew;
}
