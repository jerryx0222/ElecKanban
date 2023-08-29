#include "vwipplot.h"
#include "ui_vwipplot.h"
#include "dlgtypeselect.h"
#include "helcsignage.h"

extern HElcSignage* gSystem;

vWipPlot::vWipPlot(QWidget *parent) :
    HTabBase(parent),
    ui(new Ui::vWipPlot)
{
    ui->setupUi(this);
    ui->cmdPart->setInsertPolicy(QComboBox::NoInsert);
    ui->cmdProducts->setInsertPolicy(QComboBox::NoInsert);

    connect(&m_timer,&QTimer::timeout,this,&vWipPlot::OnReflash);
}

vWipPlot::~vWipPlot()
{
    delete ui;
}

void vWipPlot::OnInit()
{
    m_Color[0]=QColor(0, 128, 255,70);
    m_Color[1]=QColor(255, 128, 0,70);
    m_Color[2]=QColor(0, 192, 0,70);

    m_pProduct=nullptr;
    m_pPart=nullptr;
    m_pElecTitle=nullptr;
    m_pBarWip = m_pBarTarget = m_pBarSum = nullptr;

}

void vWipPlot::OnShowTab(bool show)
{
    if(show)
    {
        ui->lblType->setText(gSystem->m_TypeSelect);
        RelistProducts();
    }
}

void vWipPlot::RelistProducts()
{
    std::map<QString,QString>::iterator itMap;
    m_mapProducts.clear();
    ui->cmdProducts->clear();
    ui->cmdPart->clear();
    gSystem->CopyProducts(gSystem->m_TypeSelect,m_mapProducts);
    RelistParts2ComboBox();

    for(itMap=m_mapProducts.begin();itMap!=m_mapProducts.end();itMap++)
    {
        if(itMap==m_mapProducts.begin())
        {
            m_pProduct=nullptr;
            gSystem->GetProductStruct(itMap->first,&m_pProduct);
            if(m_pProduct!=nullptr)
                DisplayTable(m_pProduct);
        }
        ui->cmdProducts->addItem(QString("%1(%2)").arg(itMap->first).arg(itMap->second));
    }

}

void vWipPlot::RelistParts()
{
    if(m_pPart!=nullptr)
        DisplayTable(m_pProduct,m_pPart);
}

void vWipPlot::DisplayTable(Product *pP)
{
    std::list<ProcessInfo> infos;
    ui->tbTable->clear();
    m_ProcessIDs.clear();
    if(!gSystem->GetProductProcess(pP->ProductID,infos))
        return;
    DisplayTable(infos);

}

void vWipPlot::DisplayTable(Product *, Part *pP2)
{
    std::list<ProcessInfo> infos;
    ui->tbTable->clear();
    m_ProcessIDs.clear();
    if(!gSystem->GetPartProcess(pP2->PartID,infos))
        return;
    DisplayTable(infos);
}

void vWipPlot::DisplayTable(std::list<ProcessInfo> &infos)
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

void vWipPlot::setupBarChart(int nMax, QVector<double> &v1, QVector<double> &v2, QVector<double> &v3)
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

    if(m_pPart==nullptr)
        strTitle=QString("%1:%2").arg(m_pProduct->ProductID).arg(m_pProduct->CName);
    else
        strTitle=QString("%1:%2(%3)").arg(m_pPart->PartID).arg(m_pPart->CName).arg(m_pPart->Specification);

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

void vWipPlot::RelistParts2ComboBox()
{
    std::map<QString,int>::iterator itPart1;
    std::map<QString,QString>::iterator itMap,itMap2;
    ui->cmdPart->clear();
    m_mapParts.clear();

    // Parts
    for(itMap=m_mapProducts.begin();itMap!=m_mapProducts.end();itMap++)
    {
        Part* pP=nullptr;
        Product* pProduct=nullptr;
        gSystem->GetProductStruct(itMap->first,&pProduct);
        if(pProduct!=nullptr)
        {
            for(itPart1=pProduct->parts.begin();itPart1!=pProduct->parts.end();itPart1++)
            {
                itMap2=m_mapParts.find(itPart1->first);
                if(!(itMap2!=m_mapParts.end()))
                {
                    pP=nullptr;
                    gSystem->GetPartStruct(itPart1->first,&pP);
                    if(pP!=nullptr)
                    {
                        m_mapParts.insert(std::make_pair(pP->PartID,pP->CName));

                    }
                }
            }
        }
    }

    for(itMap=m_mapParts.begin();itMap!=m_mapParts.end();itMap++)
    {
        ui->cmdPart->addItem(QString("%1(%2)").arg(itMap->first).arg(itMap->second));
    }
}


void vWipPlot::on_btnSelect_clicked()
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
        ui->lblType->setText(strSelect);
        RelistProducts();
    }
    delete pNew;
}


void vWipPlot::on_cmdPart_activated(int index)
{
    std::map<QString,QString>::iterator itMap=m_mapParts.begin();
    for(int i=0;i<index;i++)
        itMap++;
    if(itMap!=m_mapParts.end())
    {
        m_pPart=nullptr;
        gSystem->GetPartStruct(itMap->first,&m_pPart);
        if(m_pPart!=nullptr)
            DisplayTable(m_pProduct,m_pPart);
    }
}

void vWipPlot::on_cmdProducts_activated(int index)
{
    std::map<QString,QString>::iterator itMap=m_mapProducts.begin();
    for(int i=0;i<index;i++)
        itMap++;
    if(itMap!=m_mapProducts.end())
    {
        m_pProduct=nullptr;
        m_pPart=nullptr;
        gSystem->GetProductStruct(itMap->first,&m_pProduct);
        if(m_pProduct!=nullptr)
            DisplayTable(m_pProduct);
    }
}

void vWipPlot::on_btnSave_clicked()
{
    QString strProduct="";
    QTableWidgetItem* pItem;
    int nCol=ui->tbTable->columnCount()-1;
    if(m_pProduct!=nullptr)
    {
        if(m_pPart!=nullptr)
            strProduct=m_pPart->PartID;
        else
            strProduct=m_pProduct->ProductID;
    }
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
    m_timer.start(100);
}

void vWipPlot::OnReflash()
{
    m_timer.stop();
    if(m_pProduct!=nullptr)
    {
        if(m_pPart!=nullptr)
            DisplayTable(m_pProduct,m_pPart);
        else
            DisplayTable(m_pProduct);
    }
}
