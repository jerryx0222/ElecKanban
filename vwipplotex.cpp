#include "vwipplotex.h"
#include "ui_vwipplotex.h"
#include "dlgtypeselect.h"
#include "helcsignage.h"
#include "dlgbackselect.h"

extern HElcSignage* gSystem;
extern bool gLogin;

vWipPlotEx::vWipPlotEx(QWidget *parent) :
    HTabBase(parent),
    ui(new Ui::vWipPlotEx)
{
    ui->setupUi(this);

    m_Color[0]=QColor(0, 128, 255,70);
    m_Color[1]=QColor(255, 128, 0,70);
    m_Color[2]=QColor(0, 192, 0,70);

    m_pProduct=nullptr;
    m_pPart=nullptr;
    m_pBarSum=nullptr;
    m_pBarTarget=nullptr;
    m_pBarWip=nullptr;
    m_pElecTitle=nullptr;
}

vWipPlotEx::~vWipPlotEx()
{
    delete ui;
}

void vWipPlotEx::OnInit()
{

}

void vWipPlotEx::OnShowTab(bool)
{
    if(ui->chkGroup->isChecked())
        ui->btnImport->setEnabled(false);
    else
        ui->btnImport->setEnabled(gLogin);
}

void vWipPlotEx::on_btnSelect_clicked()
{
    QString strType,strProduct,PName;
    std::vector<QString>::iterator itP;
    std::vector<QString> vProducts;

    dlgBackSelectEx* pDlg=new dlgBackSelectEx(&strType,&strProduct,&vProducts,this);
    pDlg->setModal(true);
    pDlg->exec();
    delete pDlg;

    m_pProduct=nullptr;
    m_pPart=nullptr;
    m_pVProducts.clear();
    m_pVParts.clear();


    if(ui->chkGroup->isChecked())
    {
        ui->btnExport->setEnabled(false);
        ui->btnImport->setEnabled(false);

        gSystem->GetProductStruct(strProduct,m_pVProducts);
        if(m_pVProducts.size()<=0)
        {
            gSystem->GetPartStruct(strProduct,m_pVParts);
            if(m_pVParts.size()<=0)
                return;
        }

    }
    else
    {
        gSystem->GetProductStruct(strProduct,&m_pProduct);
        if(m_pProduct==nullptr)
        {
            gSystem->GetPartStruct(strProduct,&m_pPart);
            if(m_pPart==nullptr)
                return;
        }

        ui->btnExport->setEnabled(true);
        ui->btnImport->setEnabled(gLogin);
    }

    QString ID,strCName;
    if(m_pProduct!=nullptr)
        ui->lblType->setText(QString("%1(%2)").arg(m_pProduct->ProductID).arg(m_pProduct->CName));
    else if(m_pPart!=nullptr)
        ui->lblType->setText(QString("%1(%2)").arg(m_pPart->PartID).arg(m_pPart->CName));
    else if(m_pVProducts.size()>0)
    {
        m_pProduct=*m_pVProducts.begin();
        ID=m_pProduct->ProductID;
        strCName=m_pProduct->CName;
        ui->lblType->setText(QString("%1...(%2)").arg(ID).arg(strCName));
        m_pProduct=nullptr;
    }
    else if(m_pVParts.size()>0)
    {
        m_pPart=*m_pVParts.begin();
        ID=m_pPart->PartID;
        strCName=m_pPart->CName;
        ui->lblType->setText(QString("%1...(%2)").arg(ID).arg(strCName));
        m_pPart=nullptr;
    }


    m_ProcessIDs.clear();
    if(m_pProduct!=nullptr)
    {
        DisplayTable(m_pProduct);
        ui->btnSave->setEnabled(gLogin);
    }
    else if(m_pPart!=nullptr)
    {
        DisplayTable(nullptr,m_pPart);
        ui->btnSave->setEnabled(gLogin);
    }
    else if(m_pVProducts.size()>0)
    {
        DisplayProductTables();
        ui->btnSave->setEnabled(false);
    }
    else if(m_pVParts.size()>0)
    {
        DisplayPartTables();
        ui->btnSave->setEnabled(false);
    }

    if(vProducts.size()>0 && strType.size()>0)
        gSystem->m_TypeSelect=strType;
}


void vWipPlotEx::DisplayTable(Product *pP)
{
    std::list<ProcessInfo> infos;
    ui->tbTable->clear();
    m_ProcessIDs.clear();
    if(!gSystem->GetProductProcess(pP->ProductID,infos))
        return;
    DisplayTable(infos);

}

void vWipPlotEx::DisplayProductTables()
{
    std::list<ProcessInfo>::iterator itT,itS;
    std::list<ProcessInfo> infoSum;
    ui->tbTable->clear();
    m_ProcessIDs.clear();
    std::vector<Product*>::iterator itP;
    if(m_pVProducts.size()<=0)
        return;

    for(itP=m_pVProducts.begin();itP!=m_pVProducts.end();itP++)
    {
        std::list<ProcessInfo> info;
        if(itP==m_pVProducts.begin())
        {
            if(!gSystem->GetProductProcess((*itP)->ProductID,infoSum))
                return;
        }
        else
        {
            if(!gSystem->GetProductProcess((*itP)->ProductID,info))
                continue;
            if(info.size()!=infoSum.size())
                continue;
            itT=info.begin();
            for(itS=infoSum.begin();itS!=infoSum.end();itS++)
            {
                itS->sum+=itT->sum;
                itS->Source+=itT->Source;
                itS->Target+=itT->Target;
                itT++;
            }
        }
    }

    DisplayTable(infoSum);
}

void vWipPlotEx::DisplayTable(Product *, Part *pP2)
{
    std::list<ProcessInfo> infos;
    ui->tbTable->clear();
    m_ProcessIDs.clear();
    if(!gSystem->GetPartProcess(pP2->PartID,infos))
        return;
    DisplayTable(infos);
}

void vWipPlotEx::DisplayPartTables()
{
    std::list<ProcessInfo>::iterator itT,itS;
    std::list<ProcessInfo> infoSum;
    ui->tbTable->clear();
    m_ProcessIDs.clear();
    std::vector<Part*>::iterator itP;
    if(m_pVParts.size()<=0)
        return;

    for(itP=m_pVParts.begin();itP!=m_pVParts.end();itP++)
    {
        std::list<ProcessInfo> info;
        if(itP==m_pVParts.begin())
        {
            if(!gSystem->GetPartProcess((*itP)->PartID,infoSum))
                return;
        }
        else
        {
            if(!gSystem->GetPartProcess((*itP)->PartID,info))
                continue;
            if(info.size()!=infoSum.size())
                continue;
            itT=info.begin();
            for(itS=infoSum.begin();itS!=infoSum.end();itS++)
            {
                itS->sum+=itT->sum;
                itS->Source+=itT->Source;
                itS->Target+=itT->Target;
                itT++;
            }
        }
    }

    DisplayTable(infoSum);

}

void vWipPlotEx::DisplayTable(std::list<ProcessInfo> &infos)
{
    QVector<double> vPlot[3];
    ui->tbTable->clear();
    std::list<ProcessInfo>::iterator itV;
    ProcessInfo* pInfo;
    m_ProcessIDs.clear();
    if(infos.size()<=0)
        return;

    QStringList strTitles;
    int nMax=0,nValue,index=0;
    int nCol=static_cast<int>(infos.size())+1;
    ui->tbTable->setRowCount(3);
    ui->tbTable->setColumnCount(nCol);
    ui->tbTable->verticalHeader()->setVisible(false);
    strTitles.push_back(tr("process"));
    //for(itV=infos.begin();itV!=infos.end();itV++)
    itV=infos.end();
    do
    {
        itV--;
        pInfo=&(*itV);
        strTitles.push_back(pInfo->ProcessName);
        ui->tbTable->setColumnWidth(index,100);
        ui->tbTable->setSelectionMode(QAbstractItemView::SingleSelection);
        index++;
    }while(itV!=infos.begin());

    ui->tbTable->setHorizontalHeaderLabels(strTitles);


    QTableWidgetItem* pItem;
    for(int i=0;i<3;i++)
    {
        index=0;
        if(i==0)
        {
            pItem=new QTableWidgetItem(QString(tr("wip")));
            QBrush brush(m_Color[0]);
            pItem->setBackground(brush);
        }
        else if(i==1)
        {
            pItem=new QTableWidgetItem(QString(tr("target")));
            QBrush brush(m_Color[1]);
            pItem->setBackground(brush);
        }
        else
        {
            pItem=new QTableWidgetItem(QString(tr("sum")));
            QBrush brush(m_Color[2]);
            pItem->setBackground(brush);
        }
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
        pItem->setTextAlignment(Qt::AlignCenter);
        ui->tbTable->setItem(i,index++,pItem);

        itV=infos.end();
        //for(itV=infos.begin();itV!=infos.end();itV++)
        do
        {
            itV--;
            pInfo=&(*itV);
            if(i==0)
                nValue=pInfo->Target;
            else if(i==1)
                nValue=pInfo->DayTarget;
            else
                nValue=pInfo->sum;
            vPlot[i].push_back(nValue);
            pItem=new QTableWidgetItem(QString("%1").arg(nValue));
            if(nValue>nMax) nMax=nValue;
            if(i==1)
                m_ProcessIDs.push_back(pInfo->ProcessID);
            else
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
            pItem->setTextAlignment(Qt::AlignCenter);
            ui->tbTable->setItem(i,index,pItem);

            index++;

        }while(itV!=infos.begin());
    }


    setupBarChart(nMax,vPlot[0],vPlot[1],vPlot[2]);

    ui->tbTable->resizeColumnsToContents();
    ui->tbTable->resizeRowsToContents();
}


void vWipPlotEx::setupBarChart(int nMax, QVector<double> &v1, QVector<double> &v2, QVector<double> &v3)
{
    if(v1.size()!=v2.size() || v1.size()!=v3.size() || v2.size()!=v3.size())
        return;

    // 創建 QCustomPlot 對象
    QCustomPlot *customPlot = ui->widget;
    int nCount=v1.size();

    // 設置標題
    QString strTitle;
    int nRow=customPlot->plotLayout()->rowCount();
    if(nRow<=1)
        customPlot->plotLayout()->insertRow(0);

    std::vector<Product*>::iterator itP1;
    std::vector<Part*>::iterator itP2;
    if(m_pProduct!=nullptr)
        strTitle=QString("%1:%2").arg(m_pProduct->ProductID).arg(m_pProduct->CName);
    else if(m_pPart!=nullptr)
        strTitle=QString("%1:%2(%3)").arg(m_pPart->PartID).arg(m_pPart->CName).arg(m_pPart->Specification);
    else if(m_pVProducts.size()>0)
    {
        itP1=m_pVProducts.begin();
        strTitle=QString("%1...").arg((*itP1)->ProductID);
    }
    else if(m_pVParts.size()>0)
    {
        itP2=m_pVParts.begin();
       strTitle=QString("%1...").arg((*itP2)->PartID);
    }


    QCPTextElement* pTitle=static_cast<QCPTextElement*>(customPlot->plotLayout()->element(0,0));
    if(pTitle==nullptr)
    {
        m_pElecTitle=new QCPTextElement(customPlot, strTitle, QFont("sans", 16, QFont::Bold));
        customPlot->plotLayout()->addElement(0, 0,m_pElecTitle);
    }
    else
    {
        pTitle->mText=strTitle;
    }



    // 創建直方圖數據
    if(m_pBarWip!=nullptr)
        customPlot->removePlottable(m_pBarWip);
    m_pBarWip = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    m_pBarWip->setBrush(QColor(0, 128, 255,70));
    m_pBarWip->setPen(QColor(0, 128, 255));
    m_pBarWip->setWidth(1);


    if(m_pBarTarget!=nullptr)
        customPlot->removePlottable(m_pBarTarget);
    m_pBarTarget = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    m_pBarTarget->setBrush(QColor(255, 128, 0,70));
    m_pBarTarget->setPen(QColor(255, 128, 0));
    m_pBarTarget->setWidth(1);


    if(m_pBarSum!=nullptr)
        customPlot->removePlottable(m_pBarSum);
    m_pBarSum = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    m_pBarSum->setBrush(QColor(0, 192, 0,70));
    m_pBarSum->setPen(QColor(0, 192, 0));
    m_pBarSum->setWidth(1);



    for (int i = 0; i < nCount; i++)
    {
        double x = i*4 + 5;
        double y1 = v1[i];
        double y2 = v2[i];
        double y3 = v3[i];

        // 為每個直方圖條添加兩個數值，分別代表條形圖的位置和高度
        m_pBarWip->addData(x, y1);
        m_pBarTarget->addData(x + 1, y2);
        m_pBarSum->addData(x + 2, y3);
    }




    // 顯示圖形
    customPlot->legend->setVisible(true);
    customPlot->rescaleAxes();
    customPlot->xAxis->setRange(0, nCount*4+6);
    customPlot->yAxis->setRange(0, nMax*1.1);

    //if(bFirst)
    {
        customPlot->yAxis->setLabel("count");
        QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
        QTableWidgetItem* pItem;
        QMap<double,QString> mData;
        for(int i=0;i<nCount;i++)
        {
            pItem=ui->tbTable->horizontalHeaderItem(i+1);
            mData.insert(i*4+6,pItem->text());
        }
        textTicker->addTicks(mData);
        customPlot->xAxis->setTicker(textTicker);
        customPlot->xAxis->setSubTicks(false);
        customPlot->xAxis->setTickLabelRotation(60);
    }



    customPlot->replot();
}

void vWipPlotEx::on_btnSave_clicked()
{
    QString strProduct="";
    QTableWidgetItem* pItem;
    int nCol=ui->tbTable->columnCount()-1;
    if(m_pProduct!=nullptr)
        strProduct=m_pProduct->ProductID;
    else if(m_pPart!=nullptr)
        strProduct=m_pPart->PartID;
    else
        return;


    std::map<QString,int> saves;
    for(int i=0;i<nCol;i++)
    {
        pItem=ui->tbTable->item(1,i+1);
        if(pItem!=nullptr && i<m_ProcessIDs.size())
        {
            saves.insert(std::make_pair(m_ProcessIDs[i],pItem->text().toInt()));
        }
    }
    gSystem->SaveProcessDayCount(strProduct,saves);
    //m_timer.start(100);
}

void vWipPlotEx::on_chkGroup_clicked()
{

}

void vWipPlotEx::on_btnExport_clicked()
{
    /*
    QString strPath;
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Export File"),
        "",
        tr("Excel Files (*.XLSX *.xlsx);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    bool ret;
    if(m_pProduct!=nullptr)
    {
        //DisplayTable(m_pProduct);
        ret=gSystem->ExportWipTarget(fileName,m_pProduct,nullptr);
        if(!ret)
            QMessageBox::warning(nullptr,tr("Warm"),tr("Import Failed!"));
    }
    else if(m_pPart!=nullptr)
    {
        ret=gSystem->ExportWipTarget(fileName,nullptr,m_pPart);
        if(!ret)
            QMessageBox::warning(nullptr,tr("Warm"),tr("Import Failed!"));
        //DisplayTable(nullptr,m_pPart);
    }
    */

}

