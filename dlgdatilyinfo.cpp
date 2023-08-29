#include "dlgdatilyinfo.h"
#include "ui_dlgdatilyinfo.h"
#include "helcsignage.h"

extern HElcSignage* gSystem;

dlgDatilyInfo::dlgDatilyInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgDatilyInfo)
{
    ui->setupUi(this);

    m_Color[0]=QColor(0, 128, 255,70);
    m_Color[1]=QColor(255, 128, 0,70);
    m_Color[2]=QColor(0, 192, 0,70);

    m_pProduct=nullptr;
    m_pPart=nullptr;
    m_pElecTitle=nullptr;
    m_pBarWip = m_pBarTarget = m_pBarSum = nullptr;
    connect(&m_timer,&QTimer::timeout,this,&dlgDatilyInfo::OnInit);
    m_timer.start(100);
}

dlgDatilyInfo::~dlgDatilyInfo()
{
    delete ui;
}


void dlgDatilyInfo::OnInit()
{
    if(m_pProduct!=nullptr)
    {
        m_timer.stop();
        if(m_pPart!=nullptr)
            DisplayTable(m_pProduct,m_pPart);
        else
            DisplayTable(m_pProduct);
    }
}

void dlgDatilyInfo::DisplayTable(Product *pP)
{
    std::list<ProcessInfo> infos;
    ui->tbTable->clear();
    m_ProcessIDs.clear();
    if(!gSystem->GetProductProcess(pP->ProductID,infos))
        return;
    DisplayTable(infos);
}

void dlgDatilyInfo::DisplayTable(Product* pP1,Part *pP2)
{
    std::list<ProcessInfo> infos;
    ui->tbTable->clear();
    m_ProcessIDs.clear();
    if(!gSystem->GetPartProcess(pP2->PartID,infos))
        return;
    DisplayTable(infos);
}

void dlgDatilyInfo::DisplayTable(std::list<ProcessInfo>& infos)
{
    QVector<double> vPlot[3];
    ui->tbTable->clear();
    std::list<ProcessInfo>::iterator itV;
    ProcessInfo* pInfo;
    m_ProcessIDs.clear();


    QStringList strTitles;
    int nMax=0,nValue,index=0;
    int nCol=static_cast<int>(infos.size())+1;
    ui->tbTable->setRowCount(3);
    ui->tbTable->setColumnCount(nCol);
    ui->tbTable->verticalHeader()->setVisible(false);
    strTitles.push_back(tr("process"));
    for(itV=infos.begin();itV!=infos.end();itV++)
    {
        pInfo=&(*itV);
        strTitles.push_back(pInfo->ProcessName);
        ui->tbTable->setColumnWidth(index,100);
        ui->tbTable->setSelectionMode(QAbstractItemView::SingleSelection);
        index++;
    }
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

        for(itV=infos.begin();itV!=infos.end();itV++)
        {
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
        }
    }


    setupBarChart(nMax,vPlot[0],vPlot[1],vPlot[2]);
}

void dlgDatilyInfo::on_btnSave_clicked()
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

/*
void dlgDatilyInfo::setupBarChartDemo(int nMax,QVector<double>& v1,QVector<double>& v2,QVector<double>& v3)
{
    if(v1.size()!=v2.size() || v1.size()!=v3.size() || v2.size()!=v3.size())
        return;
   int nCount=v1.size();
   std::vector<QString> vDatas;
   QTableWidgetItem* pItem;

   vDatas.push_back("");
   for(int i=0;i<nCount;i++)
   {
       pItem=ui->tbTable->horizontalHeaderItem(i+1);
       vDatas.push_back("");
       vDatas.push_back(pItem->text());
       vDatas.push_back("");
       vDatas.push_back("");
   }




  QString demoName = "Bar Chart Demo";
  // set dark background gradient:
  QLinearGradient gradient(0, 0, 0, 400);
  gradient.setColorAt(0,    QColor(90, 90, 90));
  gradient.setColorAt(0.38, QColor(105, 105, 105));
  gradient.setColorAt(1,    QColor(70, 70, 70));
  ui->widget->setBackground(QBrush(gradient));


  // create empty bar chart objects:
  QCPBars *regen = new QCPBars(ui->widget->xAxis, ui->widget->yAxis);
  QCPBars *nuclear = new QCPBars(ui->widget->xAxis, ui->widget->yAxis);
  QCPBars *fossil = new QCPBars(ui->widget->xAxis, ui->widget->yAxis);

  regen->setAntialiased(false); // gives more crisp, pixel aligned bar borders
  nuclear->setAntialiased(false);
  fossil->setAntialiased(false);

  regen->setStackingGap(1);
  nuclear->setStackingGap(1);
  fossil->setStackingGap(1);

  // set names and colors:
  fossil->setName("wip");
  fossil->setPen(QPen(QColor(111, 9, 176).lighter(170)));
  fossil->setBrush(QColor(111, 9, 176));

  nuclear->setName("target");
  nuclear->setPen(QPen(QColor(250, 170, 20).lighter(150)));
  nuclear->setBrush(QColor(250, 170, 20));

  regen->setName("sum");
  regen->setPen(QPen(QColor(0, 168, 140).lighter(130)));
  regen->setBrush(QColor(0, 168, 140));

  // stack bars on top of each other:
  //nuclear->moveAbove(fossil);
  //regen->moveAbove(nuclear);

  // prepare x axis with country labels:
  QVector<double> ticks1,ticks2,ticks3,ticks;
  QVector<QString> labels;
  for(int i=0;i<vDatas.size();i++)
  {
      labels.push_back(vDatas[i]);
      int k=(i+1)%4;
      if(k==2)
          ticks1.push_back(i);
      else if(k==3)
          ticks2.push_back(i);
      else if(k==0)
          ticks3.push_back(i);
      ticks.push_back(i);
  }


  QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
  textTicker->addTicks(ticks, labels);
  ui->widget->xAxis->setTicker(textTicker);
  //ui->widget->xAxis->setTickLabelRotation(90);
  ui->widget->xAxis->setSubTicks(true);
  ui->widget->xAxis->setTickLength(0, 0);

  ui->widget->xAxis->setRange(0, nCount+1);
  ui->widget->xAxis->setBasePen(QPen(Qt::white));
  ui->widget->xAxis->setTickPen(QPen(Qt::white));
  ui->widget->xAxis->grid()->setVisible(true);
  ui->widget->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
  ui->widget->xAxis->setTickLabelColor(Qt::white);
  ui->widget->xAxis->setLabelColor(Qt::white);

  // prepare y axis:
  ui->widget->yAxis->setRange(0, nMax*1.1);
  ui->widget->yAxis->setPadding(0); // a bit more space to the left border
  //ui->widget->yAxis->setLabel("Power Consumption in\nKilowatts per Capita (2007)");
  ui->widget->yAxis->setBasePen(QPen(Qt::white));
  ui->widget->yAxis->setTickPen(QPen(Qt::white));
  ui->widget->yAxis->setSubTickPen(QPen(Qt::white));
  ui->widget->yAxis->grid()->setSubGridVisible(true);
  ui->widget->yAxis->setTickLabelColor(Qt::white);
  ui->widget->yAxis->setLabelColor(Qt::white);
  ui->widget->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
  ui->widget->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));

  // Add data:
  fossil->setData(ticks1, v1);
  nuclear->setData(ticks2, v2);
  regen->setData(ticks3, v3);

  // setup legend:

  ui->widget->legend->setVisible(true);
  ui->widget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignHCenter);
  ui->widget->legend->setBrush(QColor(255, 255, 255, 100));
  ui->widget->legend->setBorderPen(Qt::NoPen);
  QFont legendFont = font();
  legendFont.setPointSize(10);
  ui->widget->legend->setFont(legendFont);
  ui->widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);


  ui->widget->replot();
}
*/

void dlgDatilyInfo::setupBarChart(int nMax, QVector<double> &v1, QVector<double> &v2, QVector<double> &v3)
{
    bool bFirst=(m_pElecTitle==nullptr);
    if(v1.size()!=v2.size() || v1.size()!=v3.size() || v2.size()!=v3.size())
        return;

    // 創建 QCustomPlot 對象
    QCustomPlot *customPlot = ui->widget;
    int nCount=v1.size();

    // 設置標題
    QString strTitle;
    if(m_pElecTitle==nullptr)
    {
        customPlot->plotLayout()->insertRow(0);
        if(m_pPart==nullptr)
            strTitle=QString("%1:%2").arg(m_pProduct->ProductID).arg(m_pProduct->CName);
        else
            strTitle=QString("%1:%2(%3)").arg(m_pPart->PartID).arg(m_pPart->CName).arg(m_pPart->Specification);
        m_pElecTitle=new QCPTextElement(customPlot, strTitle, QFont("sans", 12, QFont::Bold));
        customPlot->plotLayout()->addElement(0, 0,m_pElecTitle);
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

    if(bFirst)
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
        customPlot->xAxis->setTickLabelRotation(90);
    }



    customPlot->replot();
}



