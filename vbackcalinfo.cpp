#include "vbackcalinfo.h"
#include "ui_vbackcalinfo.h"
#include "dlgtypeselect.h"
#include "helcsignage.h"

extern HElcSignage* gSystem;

vBackCalInfo::vBackCalInfo(QWidget *parent) :
    HTabBase(parent),
    ui(new Ui::vBackCalInfo)
{
    ui->setupUi(this);


    m_colorWip=QColor(153,217,234,70);
    m_colorTarget=QColor(255,128,192,70);


    ui->cmdPart->setInsertPolicy(QComboBox::NoInsert);

}

vBackCalInfo::~vBackCalInfo()
{
    delete ui;
}

void vBackCalInfo::OnInit()
{
    if(gSystem->m_TypeSelect.size()>0)
        ui->lblType->setText(gSystem->m_TypeSelect);
}

void vBackCalInfo::OnShowTab(bool show)
{
    if(!show)
        return;

    ui->lblType->setText(gSystem->m_TypeSelect);

    if(gSystem->m_TypeSelect.size()>0)
        RelistProducts();

}


void vBackCalInfo::RelistProducts()
{
    m_mapProducts.clear();
    m_mapParts.clear();
    m_mapProcess.clear();

    ui->cmdPart->clear();

    gSystem->CopyProducts(gSystem->m_TypeSelect,m_mapProducts);
    if(m_mapProducts.size()<=0)
        return;

    int nRow,nCol=9999;
    std::map<QString,QString>::iterator itMap;
    for(itMap=m_mapProducts.begin();itMap!=m_mapProducts.end();itMap++)
    {
        std::list<ProcessInfo> lInfos;
        gSystem->GetProductProcess(itMap->first,lInfos);
        if(lInfos.size()<nCol) nCol=static_cast<int>(lInfos.size());
        m_mapProcess.insert(std::make_pair(itMap->first,lInfos));
    }
    nRow=static_cast<int>(m_mapProducts.size());

    QTableWidgetItem* pItem;
    int index=0;

    nCol=nCol*2+2;
    nRow=nRow+3;

    ui->tbInfo1->setColumnCount(nCol);
    ui->tbInfo1->setRowCount(nRow);
    ui->tbInfo1->horizontalHeader()->setVisible(false);
    ui->tbInfo1->verticalHeader()->setVisible(false);
    ui->tbInfo1->setEditTriggers(QAbstractItemView::NoEditTriggers);

    index=3;
    for(itMap=m_mapProducts.begin();itMap!=m_mapProducts.end();itMap++)
    {
        pItem=new QTableWidgetItem(itMap->first);
        ui->tbInfo1->setItem(index,0,pItem);

        pItem=new QTableWidgetItem(itMap->second);
        ui->tbInfo1->setItem(index,1,pItem);

        index++;
    }

    std::map<QString,std::list<ProcessInfo>>::iterator itProcess;
    std::list<ProcessInfo>::iterator itV;
    std::list<ProcessInfo>* pInfos;
    ProcessInfo* pInfo;
    for(int i=0;i<nRow;i++)
    {
        if(i==0)
        {
            index=1;
            for(int j=2;j<nCol;j=j+2)
            {
                pItem=new QTableWidgetItem(QString("%1").arg(index++,2,10,QChar('0')));
                ui->tbInfo1->setItem(i,j,pItem);
                pItem->setTextAlignment(Qt::AlignCenter);
            }
        }
        else if(i>2)
        {
            pItem=ui->tbInfo1->item(i,0);
            if(pItem!=nullptr)
            {
                itProcess=m_mapProcess.find(pItem->text());
                if(itProcess!=m_mapProcess.end())
                {
                    pInfos=&itProcess->second;

                    index=pInfos->size()*2+1;
                    for(itV=pInfos->begin();itV!=pInfos->end();itV++)
                    {
                        pInfo=&(*itV);
                        if(i==3)
                        {
                           ui->tbInfo1->setSpan(0,index-1,1,2);
                           ui->tbInfo1->setSpan(1,index-1,1,2);
                           ui->tbInfo1->setSpan(2,index-1,1,2);

                           pItem=new QTableWidgetItem(pInfo->ProcessID);
                           ui->tbInfo1->setItem(1,index-1,pItem);
                           pItem->setTextAlignment(Qt::AlignCenter);

                           pItem=new QTableWidgetItem(pInfo->ProcessName);
                           ui->tbInfo1->setItem(2,index-1,pItem);
                           pItem->setTextAlignment(Qt::AlignCenter);


                        }

                        pItem=new QTableWidgetItem(QString("%1").arg(pInfo->Target));
                        ui->tbInfo1->setItem(i,index-1,pItem);
                        pItem->setTextAlignment(Qt::AlignCenter);
                        if(pInfo->Target<0)
                        {
                            QBrush brush(Qt::red);
                            pItem->setForeground(brush);
                        }
                        else
                        {
                            QBrush brush(Qt::black);
                            pItem->setForeground(brush);
                        }
                        pItem->setBackground(m_colorWip);

                        pItem=new QTableWidgetItem(QString("%1").arg(pInfo->Source));
                        ui->tbInfo1->setItem(i,index,pItem);
                        pItem->setTextAlignment(Qt::AlignCenter);
                        if(pInfo->Source<0)
                        {
                            QBrush brush(Qt::red);
                            pItem->setForeground(brush);
                        }
                        else
                        {
                            QBrush brush(Qt::black);
                            pItem->setForeground(brush);
                        }
                        pItem->setBackground(m_colorTarget);


                        ui->tbInfo1->resizeColumnToContents(index);
                        index=index-2;
                    }
                }
            }
        }
    }

    ui->tbInfo1->resizeColumnsToContents();
    ui->tbInfo1->resizeRowsToContents();

    ReCheckPoints();
}


void vBackCalInfo::RelistPartInfos2(QString strP)
{
    /*
    std::map<QString,std::vector<QString>>::iterator itPart=m_mapParts.find(strP);
    if(!(itPart!=m_mapParts.end()))
    */
        return;

    QString strPart,strName;
    std::list<ProcessInfo>::iterator itInfo;
    std::list<ProcessInfo> infos;
    ProcessInfo* pInfo;
    std::vector<QString>* pvParts;//=&itPart->second;

    //strPart=(*itPart).second[0];
    gSystem->GetPartProcess(strPart,infos);


    int nRow=static_cast<int>(pvParts->size());
    int nCount=static_cast<int>(infos.size());
    QTableWidgetItem* pItem;
    int nCol=nCount*2+2;
    nRow=nRow+3;

    ui->tbInfo2->setColumnCount(nCol);
    ui->tbInfo2->setRowCount(nRow);
    ui->tbInfo2->horizontalHeader()->setVisible(false);
    ui->tbInfo2->verticalHeader()->setVisible(false);
    ui->tbInfo2->setEditTriggers(QAbstractItemView::NoEditTriggers);

    Part* pPart=nullptr;
    int nValue,index=0;
    for(size_t i=0;i<pvParts->size();i++)
    {
        strPart=(*pvParts)[i];
        infos.clear();
        gSystem->GetPartProcess(strPart,infos);
        if(i==0)
        {
            index=0;
            for(itInfo=infos.begin();itInfo!=infos.end();itInfo++)
            {
                nValue=2+2*index;
                pItem=new QTableWidgetItem(QString("%1").arg(index+1,2,10,QChar('0')));
                ui->tbInfo2->setItem(0,nValue,pItem);
                pItem->setTextAlignment(Qt::AlignCenter);

                pInfo=&(*itInfo);
                nValue=nCol-nValue;
                pItem=new QTableWidgetItem(pInfo->ProcessID);
                ui->tbInfo2->setItem(1,nValue,pItem);
                pItem->setTextAlignment(Qt::AlignCenter);

                pItem=new QTableWidgetItem(pInfo->ProcessName);
                ui->tbInfo2->setItem(2,nValue,pItem);
                pItem->setTextAlignment(Qt::AlignCenter);

                ui->tbInfo2->setSpan(0,nValue,1,2);
                ui->tbInfo2->setSpan(1,nValue,1,2);
                ui->tbInfo2->setSpan(2,nValue,1,2);

                index++;
            }


        }

        pItem=new QTableWidgetItem(strPart);
        ui->tbInfo2->setItem(3+i,0,pItem);
        pItem->setTextAlignment(Qt::AlignCenter);

        pPart=nullptr;
        gSystem->GetPartStruct(strPart,&pPart);
        pItem=new QTableWidgetItem(pPart->CName);
        ui->tbInfo2->setItem(3+i,1,pItem);
        pItem->setTextAlignment(Qt::AlignCenter);


        index=0;
        for(itInfo=infos.begin();itInfo!=infos.end();itInfo++)
        {
            pInfo=&(*itInfo);
            nValue=2+2*index;

            pItem=new QTableWidgetItem(QString("%1").arg(pInfo->Target));
            ui->tbInfo2->setItem(3+i,nValue,pItem);
            pItem->setTextAlignment(Qt::AlignCenter);
            if(pInfo->Target<0)
            {
                QBrush brush(Qt::red);
                pItem->setForeground(brush);
            }

            pItem=new QTableWidgetItem(QString("%1").arg(pInfo->Source));
            ui->tbInfo2->setItem(3+i,nValue+1,pItem);
            pItem->setTextAlignment(Qt::AlignCenter);
            if(pInfo->Source<0)
            {
                QBrush brush(Qt::red);
                pItem->setForeground(brush);
            }

            index++;
        }





    }

  /*
    std::map<QString,std::list<ProcessInfo>>::iterator itProcess;
    std::list<ProcessInfo>::iterator itV;
    std::list<ProcessInfo>* pInfos;
    ProcessInfo* pInfo;
    for(int i=0;i<nRow;i++)
    {
        if(i==0)
        {
            index=1;
            for(int j=2;j<nCol;j=j+2)
            {
                pItem=new QTableWidgetItem(QString("%1").arg(index++,2,10,QChar('0')));
                ui->tbInfo1->setItem(i,j,pItem);
                pItem->setTextAlignment(Qt::AlignCenter);
            }
        }
        else if(i>2)
        {
            pItem=ui->tbInfo1->item(i,0);
            if(pItem!=nullptr)
            {
                itProcess=m_mapProcesses.find(pItem->text());
                if(itProcess!=m_mapProcesses.end())
                {
                    pInfos=&itProcess->second;

                    index=pInfos->size()*2+1;
                    for(itV=pInfos->begin();itV!=pInfos->end();itV++)
                    {
                        pInfo=&(*itV);
                        if(i==3)
                        {
                           ui->tbInfo1->setSpan(0,index-1,1,2);
                           ui->tbInfo1->setSpan(1,index-1,1,2);
                           ui->tbInfo1->setSpan(2,index-1,1,2);

                           pItem=new QTableWidgetItem(pInfo->ProcessID);
                           ui->tbInfo1->setItem(1,index-1,pItem);
                           pItem->setTextAlignment(Qt::AlignCenter);

                           pItem=new QTableWidgetItem(pInfo->ProcessName);
                           ui->tbInfo1->setItem(2,index-1,pItem);
                           pItem->setTextAlignment(Qt::AlignCenter);


                        }

                        pItem=new QTableWidgetItem(QString("%1").arg(pInfo->Target));
                        ui->tbInfo1->setItem(i,index-1,pItem);
                        pItem->setTextAlignment(Qt::AlignCenter);
                        pItem=new QTableWidgetItem(QString("%1").arg(pInfo->Source));
                        ui->tbInfo1->setItem(i,index,pItem);
                        pItem->setTextAlignment(Qt::AlignCenter);

                        ui->tbInfo1->resizeColumnToContents(index);
                        index=index-2;
                    }
                }
            }
        }
    }
*/

    ui->tbInfo2->resizeColumnsToContents();
    ui->tbInfo2->resizeRowsToContents();
}

void vBackCalInfo::RelistPartInfos(QString strP)
{

    std::map<QString,std::vector<QString>>::iterator itPart=m_mapParts.find(strP);
    if(!(itPart!=m_mapParts.end()))
        return;

    QString strPart,strName;
    std::list<ProcessInfo>::iterator itInfo;
    std::list<ProcessInfo> infos;
    ProcessInfo* pInfo;
    std::vector<QString>* pvParts=&itPart->second;

    strPart=(*itPart).second[0];
    gSystem->GetPartProcess(strPart,infos);


    int nRow=static_cast<int>(pvParts->size());
    int nCount=static_cast<int>(infos.size());
    QTableWidgetItem* pItem;
    int nCol=nCount*2+2;
    nRow=nRow+3;

    ui->tbInfo2->setColumnCount(nCol);
    ui->tbInfo2->setRowCount(nRow);
    ui->tbInfo2->horizontalHeader()->setVisible(false);
    ui->tbInfo2->verticalHeader()->setVisible(false);
    ui->tbInfo2->setEditTriggers(QAbstractItemView::NoEditTriggers);

    Part* pPart=nullptr;
    int nValue,index=0;
    size_t DataCount=0;
    for(size_t i=0;i<pvParts->size();i++)
    {
        strPart=(*pvParts)[i];
        infos.clear();
        gSystem->GetPartProcess(strPart,infos);
        if(i==0)
        {
            index=0;
            for(itInfo=infos.begin();itInfo!=infos.end();itInfo++)
            {
                nValue=2+2*index;
                pItem=new QTableWidgetItem(QString("%1").arg(index+1,2,10,QChar('0')));
                ui->tbInfo2->setItem(0,nValue,pItem);
                pItem->setTextAlignment(Qt::AlignCenter);

                pInfo=&(*itInfo);
                nValue=nCol-nValue;
                pItem=new QTableWidgetItem(pInfo->ProcessID);
                ui->tbInfo2->setItem(1,nValue,pItem);
                pItem->setTextAlignment(Qt::AlignCenter);

                pItem=new QTableWidgetItem(pInfo->ProcessName);
                ui->tbInfo2->setItem(2,nValue,pItem);
                pItem->setTextAlignment(Qt::AlignCenter);

                ui->tbInfo2->setSpan(0,nValue,1,2);
                ui->tbInfo2->setSpan(1,nValue,1,2);
                ui->tbInfo2->setSpan(2,nValue,1,2);

                index++;
            }


        }

        pItem=new QTableWidgetItem(strPart);
        ui->tbInfo2->setItem(3+i,0,pItem);
        pItem->setTextAlignment(Qt::AlignCenter);

        pPart=nullptr;
        gSystem->GetPartStruct(strPart,&pPart);
        pItem=new QTableWidgetItem(QString("%1(%2)").arg(pPart->CName).arg(pPart->Specification));
        ui->tbInfo2->setItem(3+i,1,pItem);
        pItem->setTextAlignment(Qt::AlignCenter);


        index=0;
        DataCount=infos.size();
        for(itInfo=infos.begin();itInfo!=infos.end();itInfo++)
        {
            pInfo=&(*itInfo);
            nValue=(DataCount-index)*2;

            pItem=new QTableWidgetItem(QString("%1").arg(pInfo->Target));
            ui->tbInfo2->setItem(3+i,nValue,pItem);
            pItem->setTextAlignment(Qt::AlignCenter);
            if(pInfo->Target<0)
            {
                QBrush brush(Qt::red);
                pItem->setForeground(brush);
            }
            else
            {
                QBrush brush(Qt::black);
                pItem->setForeground(brush);
            }
            pItem->setBackground(m_colorWip);

            pItem=new QTableWidgetItem(QString("%1").arg(pInfo->Source));
            ui->tbInfo2->setItem(3+i,nValue+1,pItem);
            pItem->setTextAlignment(Qt::AlignCenter);
            if(pInfo->Source<0)
            {
                QBrush brush(Qt::red);
                pItem->setForeground(brush);
            }
            else
            {
                QBrush brush(Qt::black);
                pItem->setForeground(brush);
            }
            pItem->setBackground(m_colorTarget);

            index++;
        }





    }

  /*
    std::map<QString,std::list<ProcessInfo>>::iterator itProcess;
    std::list<ProcessInfo>::iterator itV;
    std::list<ProcessInfo>* pInfos;
    ProcessInfo* pInfo;
    for(int i=0;i<nRow;i++)
    {
        if(i==0)
        {
            index=1;
            for(int j=2;j<nCol;j=j+2)
            {
                pItem=new QTableWidgetItem(QString("%1").arg(index++,2,10,QChar('0')));
                ui->tbInfo1->setItem(i,j,pItem);
                pItem->setTextAlignment(Qt::AlignCenter);
            }
        }
        else if(i>2)
        {
            pItem=ui->tbInfo1->item(i,0);
            if(pItem!=nullptr)
            {
                itProcess=m_mapProcesses.find(pItem->text());
                if(itProcess!=m_mapProcesses.end())
                {
                    pInfos=&itProcess->second;

                    index=pInfos->size()*2+1;
                    for(itV=pInfos->begin();itV!=pInfos->end();itV++)
                    {
                        pInfo=&(*itV);
                        if(i==3)
                        {
                           ui->tbInfo1->setSpan(0,index-1,1,2);
                           ui->tbInfo1->setSpan(1,index-1,1,2);
                           ui->tbInfo1->setSpan(2,index-1,1,2);

                           pItem=new QTableWidgetItem(pInfo->ProcessID);
                           ui->tbInfo1->setItem(1,index-1,pItem);
                           pItem->setTextAlignment(Qt::AlignCenter);

                           pItem=new QTableWidgetItem(pInfo->ProcessName);
                           ui->tbInfo1->setItem(2,index-1,pItem);
                           pItem->setTextAlignment(Qt::AlignCenter);


                        }

                        pItem=new QTableWidgetItem(QString("%1").arg(pInfo->Target));
                        ui->tbInfo1->setItem(i,index-1,pItem);
                        pItem->setTextAlignment(Qt::AlignCenter);
                        pItem=new QTableWidgetItem(QString("%1").arg(pInfo->Source));
                        ui->tbInfo1->setItem(i,index,pItem);
                        pItem->setTextAlignment(Qt::AlignCenter);

                        ui->tbInfo1->resizeColumnToContents(index);
                        index=index-2;
                    }
                }
            }
        }
    }
*/

    ui->tbInfo2->resizeColumnsToContents();
    ui->tbInfo2->resizeRowsToContents();

}

void vBackCalInfo::ReCheckPoints()
{
    std::map<QString,int>::iterator itPart1;
    std::map<QString,QString>::iterator itMap,itMap2;
    std::map<QString,std::vector<QString>>::iterator itPart2,itPart3;
    std::map<QString,QString> mapParts;
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
                itMap2=mapParts.find(itPart1->first);
                if(!(itMap2!=mapParts.end()))
                {
                    pP=nullptr;
                    gSystem->GetPartStruct(itPart1->first,&pP);
                    if(pP!=nullptr)
                    {
                        mapParts.insert(std::make_pair(pP->PartID,pP->CName));
                        //ui->cmdPart->addItem(QString("%1(%2)").arg(pP->PartID).arg(pP->CName));
                    }
                }
            }
        }
    }
    if(ui->cmdPart->count()>0)
    {
        ui->cmdPart->setCurrentIndex(0);
        itMap2=mapParts.begin();
        if(itMap2!=mapParts.end())
            RelistPartInfos(itMap2->first);
    }


    QString strNew,strOld;
    for(itMap=mapParts.begin();itMap!=mapParts.end();itMap++)
    {
        strNew=itMap->first;

        if(itMap==mapParts.begin())
        {
            std::vector<QString> vParts;
            vParts.push_back(strNew);
            m_mapParts.insert(std::make_pair(strNew,vParts));
            ui->cmdPart->addItem(QString("%1(%2)").arg(strNew).arg(itMap->second));
        }
        else
        {
            itPart3=m_mapParts.end();
            for(itPart2=m_mapParts.begin();itPart2!=m_mapParts.end();itPart2++)
            {
                strOld=itPart2->first;
                if(gSystem->CheckPartProcess(strNew,strOld))
                {
                    itPart3=itPart2;
                    break;
                }
            }
            if(itPart3!=m_mapParts.end())
            {
                itPart3->second.push_back(strNew);
            }
            else
            {
                std::vector<QString> vParts;
                vParts.push_back(strNew);
                m_mapParts.insert(std::make_pair(strNew,vParts));
                ui->cmdPart->addItem(QString("%1(%2)").arg(strNew).arg(itMap->second));
            }
        }
    }

    if(ui->cmdPart->count()>0)
    {
        ui->cmdPart->setCurrentIndex(0);
        itPart3=m_mapParts.begin();
        if(itPart3!=m_mapParts.end())
            RelistPartInfos(itPart3->first);
    }

}

void vBackCalInfo::on_btnSelect_clicked()
{
    QString strName,strSelect;
    dlgTypeSelect* pNew=new dlgTypeSelect(&strSelect,this);
    pNew->setModal(true);
    pNew->setWindowTitle(tr("Type Select"));

    gSystem->m_TypeSelect="";
    ui->lblType->setText(gSystem->m_TypeSelect);

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
    delete pNew;
}



void vBackCalInfo::on_cmdPart_activated(int index)
{
    std::map<QString,std::vector<QString>>::iterator itMap=m_mapParts.begin();
    for(int i=0;i<index;i++)
        itMap++;
    if(itMap!=m_mapParts.end())
        RelistPartInfos(itMap->first);
}

void vBackCalInfo::on_btnLogin_clicked()
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
