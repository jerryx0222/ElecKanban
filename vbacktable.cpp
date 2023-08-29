#include "vbacktable.h"
#include "ui_vbacktable.h"
#include "helcsignage.h"
#include "dlgbackselect.h"
#include <QFileDialog>
#include "xlsx/xlsxdocument.h"
#include "xlsx/xlsxcellreference.h"

extern HElcSignage* gSystem;

VBackTable::VBackTable(QWidget *parent) :
    HTabBase(parent),
    ui(new Ui::VBackTable)
{
    ui->setupUi(this);
}

VBackTable::~VBackTable()
{
    delete ui;
}

void VBackTable::OnShowTab(bool show)
{
    if(!show)
        return;

    //ui->lblType->setText(gSystem->m_TypeSelect);

    //if(gSystem->m_TypeSelect.size()>0)
    //    RelistProducts();
}

void VBackTable::OnInit()
{
    //if(gSystem->m_TypeSelect.size()>0)
    //    ui->lblType->setText(gSystem->m_TypeSelect);
}

void VBackTable::on_rdoWip1_clicked()
{
    SwitchRadio(0,true);
}

void VBackTable::on_rdoInfo1_clicked()
{
    SwitchRadio(0,false);
}

void VBackTable::on_rdoWip2_clicked()
{
    SwitchRadio(1,true);
}

void VBackTable::on_rdoInfo2_clicked()
{
    SwitchRadio(1,false);
}

void VBackTable::on_btnSelect1_clicked()
{
    DisplayTable(0);
}

void VBackTable::on_btnSelect2_clicked()
{
    DisplayTable(1);
}

void VBackTable::SwitchRadio(int id, bool wip)
{
    if(id==0)
    {
        ui->rdoWip1->setChecked(wip);
        ui->rdoInfo1->setChecked(!wip);
        if(m_mapProTables[0].size()>0)
            DisplayValues(ui->tbInfo1,wip);
    }
    else if(id==1)
    {
        ui->rdoWip2->setChecked(wip);
        ui->rdoInfo2->setChecked(!wip);
        if(m_mapProTables[1].size()>0)
            DisplayValues(ui->tbInfo2,wip);
    }
    else
        return;


}

void VBackTable::DisplayTable(int id)
{
    QTableWidget* pTable;
    bool bWip;
    if(id==0)
    {
        pTable=ui->tbInfo1;
        bWip=ui->rdoWip1->isChecked();
        m_mapProTables[0].clear();
    }
    else if(id==1)
    {
        pTable=ui->tbInfo2;
        bWip=ui->rdoWip2->isChecked();
        m_mapProTables[1].clear();
    }
    else
        return;

    QString strType,strProduct,PName;
    std::vector<QString>::iterator itP;
    std::vector<QString> vProducts;


    dlgBackSelect* pDlg=new dlgBackSelect(&strType,&strProduct,&vProducts,this);
    pDlg->setModal(true);
    pDlg->exec();

    int nCol=0,nRow=0,index=0;
    if(strType.size()>0 && vProducts.size()>0)
    {
        nRow=vProducts.size()+3;
        while(pTable->rowCount()>=nRow)
            pTable->removeRow(pTable->rowCount()-1);

        itP=vProducts.begin();
        std::map<int, ProcessTable> mProcesses;
        gSystem->GetRunningsFromDB(strType,*itP,PName,mProcesses);
        nCol=mProcesses.size()+2;
        while(pTable->columnCount()>=nCol)
            pTable->removeColumn(pTable->columnCount()-1);

        pTable->setColumnCount(nCol);
        pTable->setRowCount(nRow);
        pTable->horizontalHeader()->setVisible(false);
        pTable->verticalHeader()->setVisible(false);
        pTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

        index=0;
        for(itP=vProducts.begin();itP!=vProducts.end();itP++)
        {
            std::map<int, ProcessTable> mProcesses2;
            gSystem->GetRunningsFromDB(strType,*itP,PName,mProcesses2);
            m_mapProTables[id].insert(std::make_pair(index,mProcesses2));
            RelistProducts(pTable,index++,*itP,PName,bWip);
        }

        if(mProcesses.size()>0)
            gSystem->m_TypeSelect=strType;

    }

    delete pDlg;





}


void VBackTable::RelistProducts(QTableWidget* pTable,int index,QString pid,QString pname,bool bWip)
{

    std::map<int, ProcessTable>::iterator itMap;
    QTableWidgetItem* pItem;
    QString strValue;
    std::map<int, ProcessTable>* pPTables=nullptr;
    std::map<int,std::map<int, ProcessTable>>::iterator itTable;

    if(pTable==ui->tbInfo1)
    {
        itTable=m_mapProTables[0].find(index);
        if(itTable!=m_mapProTables[0].end())
        {
            pPTables=&itTable->second;
            if(pPTables->size()<=0)
                return;
        }
        else
            return;
    }
    else if(pTable==ui->tbInfo2)
    {
        itTable=m_mapProTables[1].find(index);
        if(itTable!=m_mapProTables[1].end())
        {
            pPTables=&itTable->second;
            if(pPTables->size()<=0)
                return;
        }
        else
            return;
    }
    else
        return;

    int nItem,nRow,nCol;
    if(index==0)
    {
        nItem=0;
        nRow=0;
        nCol=pPTables->size()+1;
        for(int i=0;i<3;i++)
        {
            for(int j=0;j<2;j++)
            {
                pItem=pTable->item(i,j);
                if(pItem!=nullptr)
                    delete pItem;
            }
        }
        for(itMap=pPTables->begin();itMap!=pPTables->end();itMap++)
        {
            strValue=QString("%1").arg(pPTables->size()-nItem,2,10,QChar('0'));
            pItem=pTable->item(nRow,nCol);
            if(pItem==nullptr)
            {
                pItem=new QTableWidgetItem(strValue);
                pTable->setItem(nRow,nCol,pItem);
            }
            else
                pItem->setText(strValue);
            pItem->setTextAlignment(Qt::AlignCenter);

            strValue=itMap->second.info.ProcessID;
            pItem=pTable->item(nRow+1,nCol);
            if(pItem==nullptr)
            {
                pItem=new QTableWidgetItem(strValue);
                pTable->setItem(nRow+1,nCol,pItem);
            }
            else
                pItem->setText(strValue);
            pItem->setTextAlignment(Qt::AlignCenter);

            strValue=itMap->second.info.ProcessName;
            pItem=pTable->item(nRow+2,nCol);
            if(pItem==nullptr)
            {
                pItem=new QTableWidgetItem(strValue);
                pTable->setItem(nRow+2,nCol,pItem);
            }
            else
                pItem->setText(strValue);
            pItem->setTextAlignment(Qt::AlignCenter);

            nCol--;
            nItem++;
        }
    }

    pItem=pTable->item(3+index,0);
    if(pItem==nullptr)
    {
        pItem=new QTableWidgetItem(pid);
        pTable->setItem(3+index,0,pItem);
    }
    else
        pItem->setText(pid);

    pItem=pTable->item(3+index,1);
    if(pItem==nullptr)
    {
        pItem=new QTableWidgetItem(pname);
        pTable->setItem(3+index,1,pItem);
    }
    else
        pItem->setText(pname);

    bool bRed;
    nItem=0;
    nRow=index+3;
    nCol=pPTables->size()+1;
    for(itMap=pPTables->begin();itMap!=pPTables->end();itMap++)
    {
        if(bWip)
        {
            bRed=(itMap->second.info.Target<0);
            strValue=QString("%1").arg(itMap->second.info.Target);
        }
        else
        {
            bRed=(itMap->second.info.Source<0);
            strValue=QString("%1").arg(itMap->second.info.Source);
        }
        pItem=pTable->item(nRow,nCol);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem(strValue);
            pTable->setItem(nRow,nCol,pItem);
        }
        else
            pItem->setText(strValue);

        if(bRed)
        {
            QBrush brush(Qt::red);
            pItem->setForeground(brush);
        }

        pItem->setTextAlignment(Qt::AlignCenter);

        nCol--;
        nItem++;
    }


    pTable->resizeColumnsToContents();
    pTable->resizeRowsToContents();

}

void VBackTable::ExportExcel(int id)
{
    QTableWidget* pTable;
    if(id==0)
        pTable=ui->tbInfo1;
    else if(id==1)
        pTable=ui->tbInfo2;
    else
        return;

    if(pTable->rowCount()<=3 || pTable->columnCount()<=2)
        return;

    QString strPath;
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Export File"),
        "",
        tr("Excel Files (*.XLSX *.xlsx);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    // 創建一個QXlsx::Document對象
    QXlsx::Document xlsx;

    // 在工作表中寫入數據
    QTableWidgetItem* pItem;
    QXlsx::Format format;
    int nRow=pTable->rowCount();
    int nCol=pTable->columnCount();
    format.setFontColor(Qt::red);
    for(int i=0;i<nRow;i++)
    {
        for(int j=0;j<nCol;j++)
        {
            pItem=pTable->item(i,j);
            if(pItem!=nullptr)
            {
                QXlsx::CellReference cell(i+1,j+1);
                if((i==0 || i>=3) && j>=2)
                {
                    int nValue=pItem->text().toInt();
                    if(nValue<0)
                        xlsx.write(cell,nValue,format);
                    else
                        xlsx.write(cell,nValue);
                }
                else
                    xlsx.write(cell,pItem->text());
            }
        }
    }

    // 保存Excel檔案
    xlsx.saveAs(fileName);
}

void VBackTable::DisplayValues(QTableWidget* pTable,bool bWip)
{
    std::map<int, ProcessTable>::iterator itMap;
    std::map<int, ProcessTable>* pProcess;
    QTableWidgetItem* pItem;
    QString strValue;
    bool bRed;
    int nRow,nCol,nItem=0;
    std::map<int,std::map<int, ProcessTable>>* pMapTables;
    std::map<int,std::map<int, ProcessTable>>::iterator itTable;
    if(pTable==ui->tbInfo1)
        pMapTables=&m_mapProTables[0];
    else if(pTable==ui->tbInfo2)
        pMapTables=&m_mapProTables[1];
    else
        return;

    int index=0;
    for(itTable=pMapTables->begin();itTable!=pMapTables->end();itTable++)
    {
        pProcess=&itTable->second;
        nRow=index+3;
        nCol=pProcess->size()+1;

        for(itMap=pProcess->begin();itMap!=pProcess->end();itMap++)
        {
            if(bWip)
            {
                bRed=(itMap->second.info.Target<0);
                strValue=QString("%1").arg(itMap->second.info.Target);
            }
            else
            {
                bRed=(itMap->second.info.Source<0);
                strValue=QString("%1").arg(itMap->second.info.Source);
            }
            pItem=pTable->item(nRow,nCol);
            if(pItem==nullptr)
            {
                pItem=new QTableWidgetItem(strValue);
                pTable->setItem(nRow,nCol,pItem);
            }
            else
                pItem->setText(strValue);

            if(bRed)
            {
                QBrush brush(Qt::red);
                pItem->setForeground(brush);
            }
            else
            {
                QBrush brush(Qt::black);
                pItem->setForeground(brush);
            }

            pItem->setTextAlignment(Qt::AlignCenter);

            nCol--;
            nItem++;
        }

        index++;
    }

    pTable->resizeRowsToContents();
    pTable->resizeColumnsToContents();
}



void VBackTable::on_btnExport1_clicked()
{
    ExportExcel(0);
}

void VBackTable::on_btnExport2_clicked()
{
    ExportExcel(1);
}
