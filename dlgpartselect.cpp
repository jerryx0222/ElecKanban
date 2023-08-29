#include "dlgpartselect.h"
#include "ui_dlgpartselect.h"
#include "helcsignage.h"
#include <QMessageBox>

extern HElcSignage* gSystem;

dlgPartSelect::dlgPartSelect(QString type,QString* pPart,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgPartSelect)
{
    ui->setupUi(this);
    m_pSelectPart=pPart;
    m_strType=type;

    connect(&m_Timer,&QTimer::timeout,this,&dlgPartSelect::OnInit);
    m_Timer.start(100);
}

dlgPartSelect::~dlgPartSelect()
{
    delete ui;
}



void dlgPartSelect::on_btnSearch_clicked()
{
    QTableWidgetItem* pItem;
    QString strKey=ui->edtSearch->text();
    std::map<QString,QString>::iterator itMap;
    std::map<QString,QString> datas;
    int nRow=0;
    while(ui->tbPart->rowCount()>0)
        ui->tbPart->removeRow(0);
    if(strKey=="*" || strKey.size()<=0)
    {
        for(itMap=m_mapParts.begin();itMap!=m_mapParts.end();itMap++)
        {
            nRow=ui->tbPart->rowCount();
            ui->tbPart->insertRow(nRow);
            pItem=new QTableWidgetItem(itMap->first);
            ui->tbPart->setItem(nRow,0,pItem);
            pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
            pItem=new QTableWidgetItem(itMap->second);
            ui->tbPart->setItem(nRow,1,pItem);
            pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
        }
    }
    else
    {
        for(itMap=m_mapParts.begin();itMap!=m_mapParts.end();itMap++)
        {
            if(itMap->first.indexOf(strKey)>=0)
            {
                nRow=ui->tbPart->rowCount();
                ui->tbPart->insertRow(nRow);
                pItem=new QTableWidgetItem(itMap->first);
                ui->tbPart->setItem(nRow,0,pItem);
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
                pItem=new QTableWidgetItem(itMap->second);
                ui->tbPart->setItem(nRow,1,pItem);
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
            }
        }
    }
}

void dlgPartSelect::on_btnOK_clicked()
{
    *m_pSelectPart=ui->edtPart->text();
    emit close();
}

void dlgPartSelect::on_btnCancel_clicked()
{
    m_pSelectPart->clear();
    emit close();
}

void dlgPartSelect::OnInit()
{
    if(gSystem==nullptr)
        return;

    m_Timer.stop();

    int ColCount=2;
    ui->tbPart->setColumnCount(ColCount);
    QStringList names;
    names.push_back(tr("ID"));
    names.push_back(tr("Name"));
    ui->tbPart->setHorizontalHeaderLabels(names);
    ui->tbPart->setColumnWidth(0,220);
    ui->tbPart->setColumnWidth(1,380);

    gSystem->CopyParts(m_strType,m_mapParts);
    on_btnSearch_clicked();
    ui->edtType->setText(m_strType);
}

void dlgPartSelect::on_tbPart_cellClicked(int , int )
{
    int nSelect=ui->tbPart->currentRow();
    QTableWidgetItem* pItem=ui->tbPart->item(nSelect,0);
    if(pItem==nullptr) return;
    QString strID=pItem->text();
    Part* pPart=nullptr;
    gSystem->GetPartStruct(strID,&pPart);
    if(pPart==nullptr)
        return;
    DisplayPart(pPart);
}

void dlgPartSelect::DisplayPart(Part* pPart)
{
    ui->edtPart->setText(pPart->PartID);
    ui->edtName->setText(pPart->CName);
    ui->edtSpec->setText(pPart->Specification);
}

Part *dlgPartSelect::CreatePart()
{
    Part* pNew=new Part();
    pNew->CName=ui->edtName->text();
    pNew->Stock=0;
    pNew->parts.clear();
    pNew->PartID=ui->edtPart->text();
    pNew->TypeID=ui->edtType->text();
    pNew->Specification=ui->edtSpec->text();
    return pNew;
}

void dlgPartSelect::on_btnNew_clicked()
{
    Part* pNew=CreatePart();
    std::map<QString,QString>::iterator itMap=m_mapParts.find(pNew->PartID);
    if(itMap!=m_mapParts.end())
    {
        delete pNew;
        QMessageBox::warning(this, tr("Error"), tr("Duplicate ID"));
        return;
    }
    if(!gSystem->SaveNewPart(pNew))
    {
        QMessageBox::warning(this, tr("Error"), tr("Save Failed"));
        delete pNew;
        return;
    }
    gSystem->CopyParts(m_strType,m_mapParts);
    on_btnSearch_clicked();
}

void dlgPartSelect::on_btnSave_clicked()
{
    Part* pNew=CreatePart();
    std::map<QString,QString>::iterator itMap=m_mapParts.find(pNew->PartID);
    if(!(itMap!=m_mapParts.end()))
    {
        // new
        delete pNew;
        on_btnNew_clicked();
        return;
    }
    if(!gSystem->SavePart(pNew))
    {
        QMessageBox::warning(this, tr("Error"), tr("Save Failed"));
        delete pNew;
        return;
    }
    gSystem->CopyParts(m_strType,m_mapParts);
    on_btnSearch_clicked();
}

void dlgPartSelect::on_btnDel_clicked()
{
    Part* pNew=CreatePart();
    std::map<QString,QString>::iterator itMap=m_mapParts.find(pNew->PartID);
    if(itMap!=m_mapParts.end())
        gSystem->DeletePart(pNew);
    else
         QMessageBox::warning(this, tr("Error"), tr("Unfind"));
    delete pNew;
    gSystem->CopyParts(m_strType,m_mapParts);
    on_btnSearch_clicked();
}
