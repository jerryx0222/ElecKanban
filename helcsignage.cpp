#include "helcsignage.h"
#include "xlsx/xlsxdocument.h"

#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

int     gStep=0;
bool    gLogin=false;
uint    gUpdateCount=0;

HElcSignage::HElcSignage(QObject *parent)
{
#ifdef USE_MYSQL
    m_pMySQLDB=new HDataBaseMySQL();
#else
    m_pMySQLDB=nullptr;
#endif
    m_nRunUpdateIndex=0;

    gStep=0;
    CreateDB();

    AddNewSystemData("password",        "1234");
    AddNewSystemData("UpdateDateTime",  "2023/01/01 00:00:00");
    AddNewSystemData("NextUdDateTime",  "2999/01/01 00:00:00");
    AddNewSystemData("AutoUpdate",      1.0);

    AddNewSystemData("BearerToken",     "69cfa069bba26b8bcfd0913501b172f8");
    AddNewSystemData("SFCStatus_Link",  "https://win2016-testvm.intai-corp.com:5004/api/me/getsfcwip?site=1002");
    AddNewSystemData("SFCHistory_Link", "https://win2016-testvm.intai-corp.com:5004/api/me/getsfchis?site=1002");
    AddNewSystemData("ERPWIP_Link",     "http://erpprdap01.intai-corp.com:8000/sap/bc/srt/rfc/sap/zmm036f1/888/zmm036f1/zmm036f1");
    AddNewSystemData("ERPWIP_Login",    "A110110");
    AddNewSystemData("ERPWIP_Pwd",      "sap1234");

    AddNewSystemData("UpdateFilePath",  "/home/intai/ElecKanban_linuxR/Update/");


    LoadDB();


    QObject::connect(&m_tmCheckShipDataFile,&QTimer::timeout,this,&HElcSignage::OnCheckShipDataFile);
    m_tmCheckShipDataFile.start(3000);
}

HElcSignage::~HElcSignage()
{
    if(m_pMySQLDB!=nullptr)
        delete m_pMySQLDB;
    ClearProducts();
}

bool HElcSignage::ResetUpdateTime()
{
    QDateTime dtNow,dtStart,dtEnd;
    QString strValue;
    double dblHour;

    int nS;
    if(LoadSystemData("UpdateDateTime",strValue))
    {
        if(LoadSystemData("AutoUpdate",dblHour))
        {
            if(dblHour<=0)
            {
                // 不進行更新
                return SaveSystemData("NextUdDateTime","2999/01/01 00:00:00");
            }
            nS=static_cast<int>(dblHour*3600); // sec
            dtStart=QDateTime::fromString(strValue,"yyyy/MM/dd hh:mm:ss");
            if(dtStart.isValid())
            {
                dtEnd=dtStart.addSecs(nS);
                dtNow=QDateTime::currentDateTime();
                if(dtNow>dtEnd)
                    dtEnd=dtNow.addSecs(60);     // 60秒後更新
                strValue=dtEnd.toString("yyyy/MM/dd hh:mm:ss");
                return SaveSystemData("NextUdDateTime",strValue);
            }
        }

    }
    return false;
}

bool HElcSignage::IsReady2Update()
{
    QString strDate;
    if(!LoadSystemData("NextUdDateTime",strDate))
        return false;
    QDateTime dtTarget=QDateTime::fromString(strDate,"yyyy/MM/dd hh:mm:ss");
    QDateTime dtNow=QDateTime::currentDateTime();
    QString strNow=dtNow.toString("yyyy/MM/dd hh:mm:ss");
    return (dtNow>dtTarget);
}

// 從檔案建立製程/產品型號的檔案
bool HElcSignage::LoadFile2CreateProductEx(QString strFile,QString strPath)
{
    if(gStep!=0 || m_WebService.IsRunning())
        return false;
    if(strFile.size()<=0 || !QFile::exists(strFile))
        return false;

    QXlsx::Document xlsx(strFile);
    QStringList lstSheets=xlsx.sheetNames();
    if(lstSheets.size()<4)
        return false;

    std::map<QString,ProductCreate>::iterator itType;
    std::map<QString,ProductCreate>   mapTypeNames;
    int nStRow,nStCol,stRow,stCol,edRow,edCol;
    QString strValue,strSrc,strSecond;

     //15大產品產品總表
    xlsx.selectSheet(lstSheets[2]);
    stRow=xlsx.dimension().firstRow();
    stCol=xlsx.dimension().firstColumn();
    edRow=stRow+xlsx.dimension().rowCount();
    edCol=stCol+xlsx.dimension().columnCount();
    nStRow=nStCol=-1;
    strSrc="類別";
    for(int i=stRow;i<edRow;i++)
    {
        for(int j=stCol;j<edCol;j++)
        {
            strValue=xlsx.read(i,j).toString();
            if(strValue.indexOf(strSrc)>=0)
            {
                nStRow=i+1;
                nStCol=j;
                break;
            }
        }
        if(nStRow>=0 && nStCol>=0)
            break;
    }
    for(int i=nStRow;i<edRow;i++)
    {
        strValue=xlsx.read(i,nStCol).toString();
        if(strValue.size()<=0)
            continue;
        itType=mapTypeNames.find(strValue);
        if(itType!=mapTypeNames.end())
            continue;
        ProductCreate pData;
        pData.typeID=strValue;
        pData.typeName=xlsx.read(i,nStCol+1).toString();
        mapTypeNames.insert(std::make_pair(strValue,pData));
    }
    if(mapTypeNames.size()<=0)
        return false;

    //料號對應
    std::map<QString,QString>::iterator itPro;
    xlsx.selectSheet(lstSheets[3]);
    stRow=xlsx.dimension().firstRow();
    stCol=xlsx.dimension().firstColumn();
    edRow=stRow+xlsx.dimension().rowCount();
    edCol=stCol+xlsx.dimension().columnCount();
    nStRow=nStCol=-1;
    strSrc="類別";
    for(int i=stRow;i<edRow;i++)
    {
        for(int j=stCol;j<edCol;j++)
        {
            strValue=xlsx.read(i,j).toString();
            if(strValue.indexOf(strSrc)>=0)
            {
                nStRow=i+1;
                nStCol=j-2;
                break;
            }
        }
        if(nStRow>=0 && nStCol>=0)
            break;
    }
    for(int i=nStRow;i<edRow;i++)
    {
        strValue=xlsx.read(i,nStCol+2).toString();
        if(strValue.size()<=0)
            continue;
        itType=mapTypeNames.find(strValue);
        if(itType!=mapTypeNames.end())
        {
            strValue=xlsx.read(i,nStCol).toString();
            if(strValue.size()>0)
            {
                itPro=itType->second.mapDatas.find(strValue);
                if(itPro!=itType->second.mapDatas.end())
                    continue;
                itType->second.mapDatas.insert(std::make_pair(strValue,xlsx.read(i,nStCol+1).toString()));
            }
        }
    }


    //BOM (15大)
    std::multimap<QString,QString> mapAllDatas;     // 物料/元件對應
    std::map<QString,QString>   mapPartNames;       // 物料說明

    std::map<QString,Product>::iterator itParant;
    std::map<QString,Part>::iterator itChild;
    std::map<QString,QString>::iterator itP1,itP2;
    std::map<QString,QString>   mapParts;
    std::map<QString,QString>   mapProducts;

    xlsx.selectSheet(lstSheets[1]);
    stRow=xlsx.dimension().firstRow();
    stCol=xlsx.dimension().firstColumn();
    edRow=stRow+xlsx.dimension().rowCount();
    edCol=stCol+xlsx.dimension().columnCount();
    nStRow=nStCol=-1;
    for(int i=stRow;i<edRow;i++)
    {
        for(int j=stCol;j<edCol;j++)
        {
            strValue=xlsx.read(i,j).toString();
            if(strValue.size()>0)
            {
                nStRow=i+1;
                nStCol=j;
                break;
            }
        }
        if(nStRow>=0 && nStCol>=0)
            break;
    }

    // 產生 mapAllDatas + mapPartNames
    for(int i=nStRow;i<edRow;i++)
    {
        strSecond=xlsx.read(i,nStCol+8).toString();
        if(strSecond.size()<=0)
            continue;
        itPro=mapParts.find(strSecond);
        if(!(itPro!=mapParts.end()))
            mapParts.insert(std::make_pair(strSecond,""));

        strValue=xlsx.read(i,nStCol+1).toString();
        if(strValue.size()<=0)
            continue;
        itPro=mapProducts.find(strValue);
        if(!(itPro!=mapProducts.end()))
            mapProducts.insert(std::make_pair(strValue,""));

        mapAllDatas.insert(std::make_pair(strValue,strSecond));

        strSecond=xlsx.read(i,nStCol+2).toString();
        if(strSecond.size()<=0)
            continue;
        itPro=mapPartNames.find(strValue);
        if(!(itPro!=mapPartNames.end()))
            mapPartNames.insert(std::make_pair(strValue,strSecond));
    }

    // 出貨料號及元件料號
    for(itType=mapTypeNames.begin();itType!=mapTypeNames.end();itType++)
    {
         ProductCreate *pMyProduct=&itType->second;
        for(itPro=pMyProduct->mapDatas.begin();itPro!=pMyProduct->mapDatas.end();itPro++)
        {
            itP1=mapParts.find(itPro->first);
            itP2=mapProducts.find(itPro->first);
            if(!(itP1!=mapParts.end() ) && itP2!=mapProducts.end())
            {
                Product product;
                product.TypeID=pMyProduct->typeID;
                product.ProductID=itPro->first;
                product.CName=itPro->second;
               pMyProduct->mapProduct.insert(std::make_pair(itPro->first,product));
            }
            else
            {
                Part part;
                part.TypeID=pMyProduct->typeID;
                part.PartID=itPro->first;
                part.CName=itPro->second;
                pMyProduct->mapParts.insert(std::make_pair(itPro->first,part));
            }
        }
    }

    // 元件料號補足"料號對應"Sheet缺的料號
    std::vector<Part> vParts;
    std::map<QString,int>::iterator itPart;
    for(itType=mapTypeNames.begin();itType!=mapTypeNames.end();itType++)
    {
        ProductCreate *pMyProduct=&itType->second;
        for(itParant=pMyProduct->mapProduct.begin();itParant!=pMyProduct->mapProduct.end();itParant++)
        {
            Product *pProduct=&itParant->second;
            auto range = mapAllDatas.find(pProduct->ProductID);
            if (range != mapAllDatas.end())
            {
                for (auto it = range; it != mapAllDatas.end() && it->first == itParant->first; ++it)
                    pProduct->parts.insert(std::make_pair(it->second,1));
            }
        }


        for(itChild=pMyProduct->mapParts.begin();itChild!=pMyProduct->mapParts.end();itChild++)
        {
            Part* pPart=&itChild->second;
            auto range = mapAllDatas.find(pPart->PartID);
            if (range != mapAllDatas.end())
            {
                for (auto it = range; it != mapAllDatas.end() && it->first == itChild->first; ++it)
                {
                    pPart->parts.insert(std::make_pair(it->second,1));

                     // 元件料號補足 不在物料而在元件的
                    std::map<QString,Part>::iterator itP1=pMyProduct->mapParts.find(it->second);
                    std::map<QString,QString>::iterator itP2=mapProducts.find(it->second);
                    std::map<QString,QString>::iterator itP3=mapParts.find(it->second);
                    if(!(itP1!=pMyProduct->mapParts.end()) && itP2!=mapProducts.end())
                    {
                        Part part;
                        part.TypeID=pMyProduct->typeID;
                        part.PartID=it->second;
                        std::map<QString,QString>::iterator itName=mapPartNames.find(part.PartID);
                        if(itName!=mapPartNames.end())
                            part.CName=itName->second;
                        else
                            part.CName=it->second;
                        vParts.push_back(part);
                    }
                    else if(itP3!=mapParts.end() && (!(itP2!=mapProducts.end())))
                    {
                        Part part;
                        part.TypeID=pMyProduct->typeID;
                        part.PartID=it->second;
                        std::map<QString,QString>::iterator itName=mapPartNames.find(part.PartID);
                        if(itName!=mapPartNames.end())
                            part.CName=itName->second;
                        else
                            part.CName=it->second;
                        vParts.push_back(part);
                    }
                }
            }
        }
    }
     std::vector<Part>::iterator itVPart;
     for(itVPart=vParts.begin();itVPart!=vParts.end();itVPart++)
     {
         itType=mapTypeNames.find((*itVPart).TypeID);
         if(itType!=mapTypeNames.end())
         {
              std::map<QString,Part>::iterator itPt=itType->second.mapParts.find((*itVPart).PartID);
             if(!(itPt!=itType->second.mapParts.end()))
             {
                 itType->second.mapParts.insert(std::make_pair((*itVPart).PartID,(*itVPart)));
             }
         }
     }


      //BOP (15大)
     std::map<QString,std::map<int,Process>>  mapProcesses;
     std::map<QString,std::map<int,Process>>::iterator itProcess;
     xlsx.selectSheet(lstSheets[0]);
     stRow=xlsx.dimension().firstRow();
     stCol=xlsx.dimension().firstColumn();
     edRow=stRow+xlsx.dimension().rowCount();
     edCol=stCol+xlsx.dimension().columnCount();
     nStRow=nStCol=-1;
     for(int i=stRow;i<edRow;i++)
     {
         for(int j=stCol;j<edCol;j++)
         {
             strValue=xlsx.read(i,j).toString();
             if(strValue.size()>0)
             {
                 nStRow=i+1;
                 nStCol=j;
                 break;
             }
         }
         if(nStRow>=0 && nStCol>=0)
             break;
     }
     for(int i=nStRow;i<edRow;i++)
     {
         strValue=xlsx.read(i,nStCol+1).toString();
         if(strValue.size()<=0)
             continue;

         itProcess=mapProcesses.find(strValue);
         if(!(itProcess!=mapProcesses.end()))
        {
            std::map<int,Process> mapProcess;
            mapProcesses.insert(std::make_pair(strValue,mapProcess));
         }
     }

     for(int i=nStRow;i<edRow;i++)
     {
         strValue=xlsx.read(i,nStCol+1).toString();
         if(strValue.size()<=0)
             continue;

         itProcess=mapProcesses.find(strValue);
         std::map<int,Process>* pMapProcess;
         if(itProcess!=mapProcesses.end())
             pMapProcess=&itProcess->second;
         else
             continue;
         Process sProcess;
         sProcess.ProcessID=xlsx.read(i,nStCol+12).toString();
         sProcess.CName=xlsx.read(i,nStCol+13).toString();
         sProcess.order=xlsx.read(i,nStCol+10).toInt();
         pMapProcess->insert(std::make_pair(sProcess.order,sProcess));
     }


    // output 2 files
     for(itType=mapTypeNames.begin();itType!=mapTypeNames.end();itType++)
     {
            ProductCreate *pMyProduct=&itType->second;
            QString strFileSave=QString("%1/%2_%3.xlsx").arg(strPath).arg(pMyProduct->typeName).arg(pMyProduct->typeID);
            QXlsx::Document xlsx(strFileSave);
            QStringList lstSheets=xlsx.sheetNames();
            foreach(QString sName,lstSheets)
            xlsx.deleteSheet(sName);
            std::string strName="MES No.";
            xlsx.addSheet(QString::fromStdString(strName));
            strName="Parts";
            xlsx.addSheet(QString::fromStdString(strName));

            // MES
            lstSheets=xlsx.sheetNames();
            xlsx.selectSheet(lstSheets[0]);

            xlsx.write(1,1,QString("MES No."));
            xlsx.write(1,2,QString("MES Customer"));
            xlsx.write(1,3,QString("ThroughRate"));
            xlsx.write(1,4,QString("Components"));

            std::map<QString,Product>::iterator itP1=pMyProduct->mapProduct.begin();
            if(!(itP1!=pMyProduct->mapProduct.end()))
                continue;
            int nPartCount=itP1->second.parts.size();
            xlsx.write(1,4 + nPartCount,QString("Process"));

            int nRow=2;
            for(itP1=pMyProduct->mapProduct.begin();itP1!=pMyProduct->mapProduct.end();itP1++)
            {
                xlsx.write(nRow,1,itP1->first);
                xlsx.write(nRow,2,itP1->second.CName);
                xlsx.write(nRow,3,"90%");
                std::map<QString,int>::iterator itPart;
                int nCol=4;
                for(itPart=itP1->second.parts.begin();itPart!=itP1->second.parts.end();itPart++)
                {
                    xlsx.write(nRow,nCol,itPart->first);
                    nCol++;
                }
                 std::map<QString,std::map<int,Process>>::iterator itProcess=mapProcesses.find(itP1->first);
                if(itProcess!=mapProcesses.end())
                {
                    std::map<int,Process>::iterator itPP;
                    std::map<int,Process>* pProcess=&itProcess->second;
                    for(itPP=pProcess->begin();itPP!=pProcess->end();itPP++)
                    {
                        xlsx.write(nRow,nCol,itPP->second.ProcessID);
                        nCol++;
                    }
                }
                xlsx.write(nRow,nCol,"3001");
                nRow++;
            }


            // Parts
            xlsx.selectSheet(lstSheets[1]);

            xlsx.write(1,1,QString("Components"));
            xlsx.write(1,2,QString("PartName"));
            xlsx.write(1,3,QString("Specifications"));
            xlsx.write(1,4,QString("SecondaryPart"));

            nPartCount=0;
             std::map<QString,Part>::iterator itP2;
             for(itP2=pMyProduct->mapParts.begin();itP2!=pMyProduct->mapParts.end();itP2++)
             {
                 if(itP2->second.parts.size()>nPartCount)
                     nPartCount=itP2->second.parts.size();
             }
            xlsx.write(1,4+nPartCount,QString("Process"));

            nRow=2;
            for(itP2=pMyProduct->mapParts.begin();itP2!=pMyProduct->mapParts.end();itP2++)
            {
                xlsx.write(nRow,1,itP2->first);
                xlsx.write(nRow,2,itP2->second.CName);
                xlsx.write(nRow,3,"");
                std::map<QString,int>::iterator itPart;
                int nCol=4;

                std::map<QString,std::map<int,Process>>::iterator itProcess;
                for(itPart=itP2->second.parts.begin();itPart!=itP2->second.parts.end();itPart++)
                {
                    //itProcess=mapProcesses.find(itPart->first);
                    //if(itProcess!=mapProcesses.end())
                    {
                        xlsx.write(nRow,nCol,itPart->first);
                        nCol++;
                    }
                }
                if(nCol==4)
                    nCol++;

                 itProcess=mapProcesses.find(itP2->first);
                if(itProcess!=mapProcesses.end())
                {
                    std::map<int,Process>::iterator itPP;
                    std::map<int,Process>* pProcess=&itProcess->second;
                    for(itPP=pProcess->begin();itPP!=pProcess->end();itPP++)
                    {
                        xlsx.write(nRow,nCol,itPP->second.ProcessID);
                        nCol++;
                    }
                }
                if(nCol==5)
                    xlsx.write(nRow,nCol,"2002");
                else
                    xlsx.write(nRow,nCol,"2001");
                nRow++;
            }

            xlsx.save();
        }

    // output 2 types
    QString strFileSave=QString("%1/Types.xlsx").arg(strPath);
    QXlsx::Document xlsx2(strFileSave);
    QStringList lstSheets2=xlsx.sheetNames();
    if(lstSheets2.size()<=0)
    {
        xlsx2.addSheet("types");
        xlsx2.selectSheet("types");
    }
    else
        xlsx2.selectSheet(lstSheets[0]);

    xlsx2.write(1,1,QString("Type"));
    xlsx2.write(1,2,QString("Name"));
    int nRow=2;

    std::map<int,Process>::iterator itPP;
    std::map<QString,QString> mapPName;
    for(itProcess=mapProcesses.begin();itProcess!=mapProcesses.end();itProcess++)
    {
        std::map<int,Process>* pMap=&itProcess->second;
        for(itPP=pMap->begin();itPP!=pMap->end();itPP++)
        {
            itPro=mapPName.find(itPP->second.ProcessID);
            if(!(itPro!=mapPName.end()))
                mapPName.insert(std::make_pair(itPP->second.ProcessID,itPP->second.CName));
        }
    }
    for(itPro=mapPName.begin();itPro!=mapPName.end();itPro++)
    {
        xlsx2.write(nRow,1,itPro->first);
        xlsx2.write(nRow,2,itPro->second);
        nRow++;
    }
    xlsx2.save();

    return false;
}


// 從檔案建立製程/產品型號
bool HElcSignage::LoadFile2CreateProduct(QString strFile)
{
    if(gStep!=0 || m_WebService.IsRunning())
        return false;

    int nValue;
    std::vector<QStringList>::iterator itD;
    std::vector<QStringList> vDatas;
    if(strFile.size()<=0)
        return false;

    if(!QFile::exists(strFile))
        return false;

    QFileInfo fileInfo(strFile);
    QString fileName = fileInfo.baseName();
    nValue=fileName.indexOf("_");
    if(nValue<=0)
        return false;

    // 型號名
    QString strTypeName=fileName.left(nValue);
    // 型號
    QString strType=fileName.right(fileName.size()-nValue-1);

    QXlsx::Document xlsx(strFile);
    QStringList lstSheets=xlsx.sheetNames();
    if(lstSheets.size()<2)
        return false;

    xlsx.selectSheet(lstSheets[0]);     // 第一頁:MES客戶
    QString strTemp;
    int nProcessStart=-1;
    int nRow=xlsx.dimension().rowCount();
    int nCol=xlsx.dimension().columnCount();
    for(int i=5;i<nCol;i++)
    {
        strTemp = xlsx.read(1, i).toString();    // '製程'
        if(strTemp.size()>0)
        {
            nProcessStart=i;    // 製程開始位置
            break;
        }
    }
    if(nProcessStart<0 || nProcessStart>nCol)
        return false;


    // 產品製程
    std::map<QString,QStringList>::iterator itP,itP2;
    std::map<QString,Part>::iterator itPart,itPart2,itPart3;
    std::vector<Product>   vProducts;
    std::map<QString,Part> mapMainParts,mapChildParts;
    QStringList lstPartProcess;
    std::map<QString,QStringList>   mapParductProcess;
    for(int i=2;i<=nRow;i++)
    {
        // MES客戶
        lstPartProcess.clear();
        Product newProduct;
        newProduct.ProductID = xlsx.read(i, 1).toString();    //MES客戶編號
        newProduct.TypeID=strType;
        newProduct.Stock=0;
        if(newProduct.ProductID.size()>0)
        {
           newProduct.CName = xlsx.read(i, 2).toString();       //MES客戶中文
           QString strPass=xlsx.read(i, 3).toString();
           if(strPass.indexOf("%")>0)
           {
               strPass.remove("%");
               newProduct.FirstPass = strPass.toDouble()/100.0f;    //直通率
           }
           else
                newProduct.FirstPass = strPass.toDouble();    //直通率
           if(newProduct.FirstPass<=0 || newProduct.FirstPass>1.0f)
               newProduct.FirstPass=0.9;
           for(int j=4;j<nProcessStart;j++)
           {
               if(j>=nCol)
                   break;
               strTemp = xlsx.read(i, j).toString();       //組成零件
               if(strTemp.size()>0)
               {
                    newProduct.parts.insert(std::make_pair(strTemp,1));
                    itPart=mapMainParts.find(strTemp);
                    if(!(itPart!=mapMainParts.end()))
                    {
                        Part part;
                        part.PartID=strTemp;
                        part.Stock=0;
                        part.TypeID=strType;
                        mapMainParts.insert(std::make_pair(strTemp,part));
                    }
               }
               else
                   break;
           }
           vProducts.push_back(newProduct);
        }
        else
            break;

        lstPartProcess.clear();
        for(int j=nProcessStart;j<=nCol;j++)
        {
            strTemp = xlsx.read(i, j).toString();
            if(strTemp.size()>0)
                lstPartProcess.push_back(strTemp);
            else
                break;
        }
        if(lstPartProcess.size()>0)
        {
            itP=mapParductProcess.find(newProduct.ProductID);
            if(itP!=mapParductProcess.end())
                itP->second=lstPartProcess;
            else
                mapParductProcess.insert(std::make_pair(newProduct.ProductID,lstPartProcess));
        }
    }

    // 零件製程
    std::map<QString,int>::iterator itPartChild;
    std::map<QString,QStringList>   mapPartProcess;
    xlsx.selectSheet(lstSheets[1]);         // 第二頁:零件
    nRow=xlsx.dimension().rowCount();
    nCol=xlsx.dimension().columnCount();
    nProcessStart=-1;
    for(int i=4;i<=nCol;i++)
    {
        strTemp = xlsx.read(1, i).toString();    // 製程開始位置
        if(strTemp.size()>0 && (strTemp=="Process"))
        {
            nProcessStart=i;
            break;
        }
    }
    /*
    if(nProcessStart<0 || nProcessStart>nCol)
        return false;
        */

    for(int index=2;index<=nRow;index++)
    {
        Part part;
        part.PartID = xlsx.read(index, 1).toString();
        part.CName = xlsx.read(index, 2).toString();
        part.Specification = xlsx.read(index, 3).toString();
        part.TypeID=strType;
        for(int i=4;i<=nProcessStart;i++)
        {
            strTemp = xlsx.read(index, i).toString();
            if(strTemp.size()>0)
                part.parts.insert(std::make_pair(strTemp,1));
            else
                break;
        }
        lstPartProcess.clear();
        for(int i=nProcessStart;i<=nCol;i++)
        {
            strTemp = xlsx.read(index, i).toString();
            if(strTemp.size()>0)
                lstPartProcess.push_back(strTemp);
            else
                break;
        }
        itP=mapPartProcess.find(part.PartID);
        if(itP!=mapPartProcess.end())
            itP->second=lstPartProcess;
        else
            mapPartProcess.insert(std::make_pair(part.PartID,lstPartProcess));

        itPart=mapMainParts.find(part.PartID);
        if(itPart!=mapMainParts.end())
            itPart->second=part;
        else
        {
            itPart=mapChildParts.find(part.PartID);
            if(!(itPart!=mapChildParts.end()))
                mapChildParts.insert(std::make_pair(part.PartID,part));
        }
    }

    // 將零件製程統一在產品零件中
    std::map<QString,std::map<int,QString>> mapPartInPart;
    QStringList *plstPartProcess;
    for(itPart=mapMainParts.begin();itPart!=mapMainParts.end();itPart++)
    {
        plstPartProcess=nullptr;
        Part* pPartMain=&itPart->second;
        itP=mapPartProcess.find(pPartMain->PartID);
        if(itP!=mapPartProcess.end())
            plstPartProcess=&itP->second;
        else
            continue;
        for(itPartChild=pPartMain->parts.begin();itPartChild!=pPartMain->parts.end();itPartChild++)
        {
            itPart2=mapChildParts.find(itPartChild->first);
            itP=mapPartProcess.find(itPartChild->first);
            if(plstPartProcess!=nullptr && itPart2!=mapChildParts.end() && itP!=mapPartProcess.end())
            {
                (*plstPartProcess)=itP->second + (*plstPartProcess);
            }
        }
    }


    // save 2 db
    HRecordset* pRS;
    HDataBase* pDB;
    QString strSQL;
    if(m_pMySQLDB!=nullptr)
    {
        pDB=m_pMySQLDB;
        if(!m_pMySQLDB->Open())
            return false;
        pRS=new HRecordsetMySQL();
    }
    else
    {
        pDB=&m_LiteDB;
        if(!pDB->Open())
            return false;
        pRS=new HRecordsetSQLite();
    }

    // Type save
    strSQL=QString("Select count(*) from IntaiWeb_types Where TypeID='%1'").arg(strType);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        nValue=0;
        if(!pRS->GetValue(L"count(*)",nValue) || nValue<=0)
            strSQL=QString("insert into IntaiWeb_types(TypeID,TypeName) Values('%1','%2')").arg(strType).arg(strTypeName);
        else
            strSQL=QString("update IntaiWeb_types Set TypeName = '%2' Where TypeID='%1'").arg(strType).arg(strTypeName);
        pDB->ExecuteSQL(strSQL);
    }

    // Product/Part Save
    std::vector<Product>::iterator itVP1;
    Product* pProduct;
    QString strFields,strValues;
    for(itVP1=vProducts.begin();itVP1!=vProducts.end();itVP1++)
    {
        pProduct=&(*itVP1);
        strSQL=QString("Select count(*) from IntaiWeb_product Where ProductID='%1'").arg(pProduct->ProductID);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            nValue=0;
            if(!pRS->GetValue(L"count(*)",nValue) || nValue<=0)
                strSQL=QString("insert into IntaiWeb_product(ProductID,CName,FirstPass,TypeID,Stock) Values('%1','%2',%3,'%4',%5)").arg(
                            pProduct->ProductID).arg(
                            pProduct->CName).arg(
                            pProduct->FirstPass).arg(
                            pProduct->TypeID).arg(
                            pProduct->Stock);
            else
                strSQL=QString("update IntaiWeb_product Set CName='%2',FirstPass=%3,TypeID='%4',Stock=%5 where ProductID='%1'").arg(
                            pProduct->ProductID).arg(
                            pProduct->CName).arg(
                            pProduct->FirstPass).arg(
                            pProduct->TypeID).arg(
                            pProduct->Stock);
            pDB->ExecuteSQL(strSQL);
        }

        for(itPartChild=pProduct->parts.begin();itPartChild!=pProduct->parts.end();itPartChild++)
        {
            strSQL=QString("Select count(*) from IntaiWeb_productlink Where ProductID='%1' and PartID='%2'").arg(pProduct->ProductID).arg(itPartChild->first);
            if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
            {
                nValue=0;
                if(!pRS->GetValue(L"count(*)",nValue) || nValue<=0)
                {
                    strSQL=QString("insert into IntaiWeb_productlink(ProductID,PartID) Values('%1','%2')").arg(
                                pProduct->ProductID).arg(
                                itPartChild->first);
                    pDB->ExecuteSQL(strSQL);
                }
            }
        }
    }

    for(itPart=mapChildParts.begin();itPart!=mapChildParts.end();itPart++)
    {
        Part* pPart=&itPart->second;
        strSQL=QString("Select count(*) from IntaiWeb_part Where PartID='%1'").arg(pPart->PartID);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            nValue=0;
            if(!pRS->GetValue(L"count(*)",nValue) || nValue<=0)
                strSQL=QString("insert into IntaiWeb_part(PartID,CName,Specification,TypeID) Values('%1','%2','%3','%4')").arg(
                            pPart->PartID).arg(
                            pPart->CName).arg(
                            pPart->Specification).arg(
                            pPart->TypeID);
            else
                strSQL=QString("update IntaiWeb_part Set CName='%2',Specification='%3',TypeID='%4' where PartID='%1'").arg(
                            pPart->PartID).arg(
                            pPart->CName).arg(
                            pPart->Specification).arg(
                            pPart->TypeID);
            pDB->ExecuteSQL(strSQL);
        }
    }

    for(itPart=mapMainParts.begin();itPart!=mapMainParts.end();itPart++)
    {
        Part* pPart=&itPart->second;
        strSQL=QString("Select count(*) from IntaiWeb_part Where PartID='%1'").arg(pPart->PartID);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            nValue=0;
            if(!pRS->GetValue(L"count(*)",nValue) || nValue<=0)
                strSQL=QString("insert into IntaiWeb_part(PartID,CName,Specification,TypeID) Values('%1','%2','%3','%4')").arg(
                            pPart->PartID).arg(
                            pPart->CName).arg(
                            pPart->Specification).arg(
                            pPart->TypeID);
            else
                strSQL=QString("update IntaiWeb_part Set CName='%2',Specification='%3',TypeID='%4' where PartID='%1'").arg(
                            pPart->PartID).arg(
                            pPart->CName).arg(
                            pPart->Specification).arg(
                            pPart->TypeID);
            pDB->ExecuteSQL(strSQL);
        }

        for(itPartChild=pPart->parts.begin();itPartChild!=pPart->parts.end();itPartChild++)
        {
            strSQL=QString("Select count(*) from IntaiWeb_productlink Where ProductID='%1' and PartID='%2'").arg(pPart->PartID).arg(itPartChild->first);
            if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
            {
                nValue=0;
                if(!pRS->GetValue(L"count(*)",nValue) || nValue<=0)
                {
                    strSQL=QString("insert into IntaiWeb_productlink(ProductID,PartID) Values('%1','%2')").arg(
                                pPart->PartID).arg(
                                itPartChild->first);
                    pDB->ExecuteSQL(strSQL);
                }
            }
        }
    }


    // process save:產品
    int index=0;
    int nMax,nOrder=0;
    for(itP=mapParductProcess.begin();itP!=mapParductProcess.end();itP++)
    {
        index=0;
        nMax=itP->second.size();
        foreach(const QString &info,itP->second)
        {
            strSQL=QString("Select count(*) from IntaiWeb_process Where ProcessID='%1'").arg(info);
            if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
            {
                nValue=0;
                if(!pRS->GetValue(L"count(*)",nValue) || nValue<=0)
                {
                    strSQL=QString("insert into IntaiWeb_process(ProcessID,CName) Values('%1','%1')").arg(info);
                    pDB->ExecuteSQL(strSQL);
                }
            }

            strSQL=QString("Select count(*) from IntaiWeb_processlink Where ProductID='%1' and theOrder=%2").arg(
                        itP->first).arg(nMax-index-1);
            if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
            {
                nValue=0;
                if(!pRS->GetValue(L"count(*)",nValue) || nValue<=0)
                {
                    strSQL=QString("insert into IntaiWeb_processlink(ProductID,PartID,ProcessID,theOrder,DayTarget,Stock,Schedule) Values('%1','%1','%2',%3,%4,%5,%6)").arg(
                                itP->first).arg(
                                info).arg(
                                nMax-index-1).arg(
                                0).arg(
                                0).arg(
                                0);
                    pDB->ExecuteSQL(strSQL);
                    index++;
                }
            }


        }
        index=itP->second.size();
        strSQL=QString("delete from IntaiWeb_processlink Where ProductID='%1' and theOrder>=%2").arg(itP->first).arg(index);
        pDB->ExecuteSQL(strSQL);
    }

    // process save:零件
    for(itPart=mapMainParts.begin();itPart!=mapMainParts.end();itPart++)
    {
        index=0;
        Part* pPart=&itPart->second;
        itP=mapPartProcess.find(pPart->PartID);
        if(itP!=mapPartProcess.end())
        {
            plstPartProcess=&itP->second;
            nMax=plstPartProcess->size();
            foreach(const QString &info,*plstPartProcess)
            {
                strSQL=QString("Select count(*) from IntaiWeb_process Where ProcessID='%1'").arg(info);
                if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
                {
                    nValue=0;
                    if(!pRS->GetValue(L"count(*)",nValue) || nValue<=0)
                    {
                        strSQL=QString("insert into IntaiWeb_process(ProcessID,CName) Values('%1','%1')").arg(info);
                        pDB->ExecuteSQL(strSQL);
                    }
                }

                strSQL=QString("Select count(*) from IntaiWeb_processlink Where ProductID='%1' and theOrder=%2").arg(
                            pPart->PartID).arg(nMax-index-1);
                if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
                {
                    nValue=0;
                    if(!pRS->GetValue(L"count(*)",nValue) || nValue<=0)
                    {
                        strSQL=QString("insert into IntaiWeb_processlink(ProductID,PartID,ProcessID,theOrder,DayTarget,Stock,Schedule) Values('%1','%2','%3',%4,%5,%6,%7)").arg(
                                    pPart->PartID).arg(
                                    pPart->PartID).arg(
                                    info).arg(
                                    nMax-index-1).arg(
                                    0).arg(
                                    0).arg(
                                    0);
                        pDB->ExecuteSQL(strSQL);
                        index++;
                    }
                }  
            }
            index=itP->second.size();
            strSQL=QString("delete from IntaiWeb_processlink Where ProductID='%1' and theOrder>=%2").arg(itP->first).arg(index);
            pDB->ExecuteSQL(strSQL);
        }

        index=0;
        itP=mapPartProcess.find(pPart->PartID);
        if(pPart->parts.size()>0 && itP!=mapPartProcess.end())
        {
            plstPartProcess=&itP->second;
            nMax=plstPartProcess->size();
            int nPIndex=0;
            itPartChild=pPart->parts.end();
            itPartChild--;
            while(itPartChild!=pPart->parts.end())
            {
                itP=mapPartProcess.find(itPartChild->first);
                if(itP!=mapPartProcess.end())
                {
                    index+=itP->second.size();
                    nOrder=plstPartProcess->size()-index;
                    strSQL=QString("update IntaiWeb_processlink Set PartID='%1' Where ProductID='%2' and theOrder=%3").arg(
                                itP->first).arg(
                                pPart->PartID).arg(
                                nOrder);
                    pDB->ExecuteSQL(strSQL);
                }
                itPartChild--;
                nPIndex++;
                if(nPIndex>=pPart->parts.size())
                    break;
            }
        }
    }



    // 出貨資訊
    QDate myDate=QDate::currentDate();
    for(itVP1=vProducts.begin();itVP1!=vProducts.end();itVP1++)
    {
        pProduct=&(*itVP1);
        strSQL=QString("Select count(*) from IntaiWeb_Shipment Where ProductID='%1'").arg(pProduct->ProductID);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            nValue=0;
            if(!pRS->GetValue(L"count(*)",nValue) || nValue<=0)
            {
                strSQL=QString("insert into IntaiWeb_Shipment(ProductID,OutDate,OutCount) Values('%1','%2',0)").arg(
                            pProduct->ProductID).arg(
                            myDate.toString("yyyy/MM/dd"));
                pDB->ExecuteSQL(strSQL);
            }
        }

    }




    delete pRS;
    pDB->Close();


    LoadDB();

    CreateTitles(strType);
    return true;
}



bool HElcSignage::LoadProcessIDFromFile(QString strFile)
{
    if(gStep!=0 || m_WebService.IsRunning())
        return false;

    int nValue;
    std::vector<QStringList>::iterator itD;
    std::vector<QStringList> vDatas;
    if(strFile.size()<=0)
        return false;

    if(!QFile::exists(strFile))
        return false;

    QXlsx::Document xlsx(strFile);
    QStringList lstSheets=xlsx.sheetNames();
    if(lstSheets.size()<1)
        return false;

    xlsx.selectSheet(lstSheets[0]);     // 第一頁
    QString strTemp;
    int nRow=xlsx.dimension().rowCount();
    int nCol=xlsx.dimension().columnCount();
    if(nCol<2 || nRow<2)
        return false;

    std::map<QString,QString>::iterator itMap;
    std::map<QString,QString> mapProcess;
    for(int i=2;i<=nRow;i++)
    {
        QString vID = xlsx.read(i, 1).toString();
        QString vName = xlsx.read(i, 2).toString();
        if(vID.size()>0 && vName.size()>0)
        {
            itMap=mapProcess.find(vID);
            if(!(itMap!=mapProcess.end()))
                mapProcess.insert(std::make_pair(vID,vName));
        }
    }

    HRecordset* pRS;
    HDataBase* pDB;
    QString strSQL;
    if(m_pMySQLDB!=nullptr)
    {
        pDB=m_pMySQLDB;
        if(!m_pMySQLDB->Open())
            return false;
        pRS=new HRecordsetMySQL();
    }
    else
    {
        pDB=&m_LiteDB;
        if(!pDB->Open())
            return false;
        pRS=new HRecordsetSQLite();
    }

    for(itMap=mapProcess.begin();itMap!=mapProcess.end();itMap++)
    {
        strSQL=QString("Select count(*) from IntaiWeb_process Where ProcessID='%1'").arg(itMap->first);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            nValue=0;
            if(pRS->GetValue(L"count(*)",nValue))
            {
                if(nValue<=0)
                {
                    strSQL=QString("insert into IntaiWeb_process(ProcessID,CName) Values('%1','%2')").arg(
                                itMap->first).arg(
                                itMap->second);
                    pDB->ExecuteSQL(strSQL);
                }
                else
                {
                    strSQL=QString("update IntaiWeb_process Set CName='%2' Where ProcessID='%1'").arg(
                                itMap->first).arg(
                                itMap->second);
                    pDB->ExecuteSQL(strSQL);
                }
            }
            else
            {
                nValue=-1;
            }
        }
    }





    if(pRS!=nullptr)
        delete pRS;
    pDB->Close();
    return true;
}


void HElcSignage::run()
{
    QString strValue;
    QDateTime dt;
    int ercode;

    while(true)
    {
        switch(gStep)
        {
        case stepIdle:
            //emit OnUpdateError(m_nRunUpdateIndex);
            return;
        case stepLoadHistory:
            if(LoadHistorys())
                gStep=stepLoadStatus;
            break;
        case stepLoadStatus:
            if(LoadStatus())
                gStep=stepLoadERP;
            break;
        case stepLoadERP:
            if(LoadWips(ercode))
            {
                emit OnError("");
                gStep=stepExportShip;
            }
            else if(ercode<0)
            {
                emit OnError("Net Failed!");
                gStep=stepIdle;
            }
            break;

        case stepExportShip:
            if(ExportShip())
                gStep=stepExportBack;
            break;

        case stepExportBack:
            if(ExportBack())
                gStep=stepSaveDateTime;
            break;

        case stepSaveDateTime:
            dt=QDateTime::currentDateTime();
            if(SaveSystemData("UpdateDateTime",dt.toString("yyyy/MM/dd hh:mm:ss")))
                gStep=stepReStartUpdateTimer;
            break;

        case stepReStartUpdateTimer:
            if(ResetUpdateTime())
            {
                gStep=stepIdle;
                emit OnUpdateError(m_nRunUpdateIndex);
            }
            break;



        case stepCheckShipFile:
            if(ImportShipmentDateFile())
            {
                emit OnUpdateError(m_nRunUpdateIndex);
                gStep=stepIdle;
                RunUpdateDataBase();
            }
            else
            {
                emit OnUpdateError(m_nRunUpdateIndex);
                gStep=stepIdle;
            }
            break;

        case stepCheckExportFile:

            break;
        }
    }
}

bool HElcSignage::CreateDB()
{
    //抓取程式所在的目錄
    m_strAppPath = QDir::currentPath();

    //建立所需的新目錄
    QDir dir;
    QString strTemp,strSQL;
    strTemp=m_strAppPath + "/DataBase";
    dir.mkdir(strTemp);

    strTemp=m_strAppPath + "/Update";
    dir.mkdir(strTemp);

    strTemp=m_strAppPath + "/Export";
    dir.mkdir(strTemp);

    QString strDBFile;
    strDBFile = m_strAppPath + "/DataBase/Data.db";
    HDataBase* pDB;

    m_LiteDB.Open(strDBFile);
    m_LiteDB.Close();
    if(m_pMySQLDB!=nullptr)
    {
        pDB=m_pMySQLDB;
        if(!m_pMySQLDB->Open("mydata"))
            return false;
    }
    else
    {
        pDB=&m_LiteDB;
        if(!pDB->Open())
            return false;
    }

    //建立系統資料表
    if(!pDB->CheckTableExist("IntaiWeb_systemset"))
    {
        strSQL = "CREATE TABLE IntaiWeb_systemset (";
        strSQL += "DataName VARCHAR(255),";
        strSQL += "IntData integer,";
        strSQL += "DblData double,";
        strSQL += "StrData TEXT,";

        strSQL += "PRIMARY KEY(DataName));";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }

    //建立Product(產品)資料表
    if(!pDB->CheckTableExist("IntaiWeb_product"))
    {
        strSQL = "CREATE TABLE IntaiWeb_product (";
        strSQL += "ProductID VARCHAR(32),";           // 料號
        strSQL += "CName TEXT,";                        // 中文名稱
        strSQL += "FirstPass double default 0.9,";      // 直通率
        strSQL += "Stock integer default 0,";           // 成品倉(3001) from ERP
        strSQL += "TypeID TEXT,";                       //

        strSQL += "PRIMARY KEY(ProductID));";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }

    //建立Shipment(出貨)資料表
    if(!pDB->CheckTableExist("IntaiWeb_Shipment"))
    {
        strSQL = "CREATE TABLE IntaiWeb_Shipment (";
        strSQL += "id INT AUTO_INCREMENT PRIMARY KEY,";
        strSQL += "ProductID VARCHAR(32),";           // 料號
        strSQL += "OutDate VARCHAR(16),";                      // 出貨日期
        strSQL += "OutCount integer default 0)";        // 出貨數量

        //strSQL += "PRIMARY KEY(ProductID,OutDate));";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }

    //建立Part(部件)資料表
    if(!pDB->CheckTableExist("IntaiWeb_part"))
    {
        strSQL = "CREATE TABLE IntaiWeb_part (";
        strSQL += "PartID VARCHAR(32),";           // 部件料號
        strSQL += "CName TEXT,";                     // 中文名稱
        strSQL += "Specification TEXT,";             // 規格
        strSQL += "Stock integer default 0,";        // 原料倉+零件倉(100x + 200x) from ERP
        strSQL += "TypeID TEXT,";                       //

        strSQL += "PRIMARY KEY(PartID));";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }

    //建立ProductLink(產品+部件)資料表
    if(!pDB->CheckTableExist("IntaiWeb_productlink"))
    {
        strSQL = "CREATE TABLE IntaiWeb_productlink (";
        strSQL += "id INT AUTO_INCREMENT PRIMARY KEY,";
        strSQL += "ProductID VARCHAR(32),";           // 產品料號
        strSQL += "PartID VARCHAR(32),";              // 部件料號
        strSQL += "PartCount integer default 1)";       // 組成部件個數

        //strSQL += "PRIMARY KEY(ProductID,PartID));";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }

    //建立Process(製程)資料表
    if(!pDB->CheckTableExist("IntaiWeb_process"))
    {
        strSQL = "CREATE TABLE IntaiWeb_process (";
        strSQL += "ProcessID VARCHAR(32),";           // 製程編號(工序編號)
        strSQL += "CName TEXT,";                        // 中文名稱(工序名稱)

        strSQL += "PRIMARY KEY(ProcessID));";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }

    //建立ProcessLink(製程)資料表
    if(!pDB->CheckTableExist("IntaiWeb_processlink"))
    {
        strSQL = "CREATE TABLE IntaiWeb_processlink (";
        strSQL += "id INT AUTO_INCREMENT PRIMARY KEY,";
        strSQL += "ProcessID VARCHAR(32),";           // 製程編號
        strSQL += "ProductID VARCHAR(32),";           // 產品料號
        strSQL += "PartID VARCHAR(32),";              // 部件料號

        strSQL += "theOrder integer default 0,";        // 工序
        strSQL += "DayTarget integer default 0,";       // 日目標      // 手動
        strSQL += "Stock integer default 0,";           // 庫存       // SFC 現況表
        strSQL += "Schedule integer default 0)";        // 累計進度數  // SFC 歷程表


        //strSQL += "PRIMARY KEY(ProcessID,ProductID,PartID));";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }



    //建立ERPStock(庫存)資料表
    if(!pDB->CheckTableExist("IntaiWeb_ERPStock"))
    {
        strSQL = "CREATE TABLE IntaiWeb_ERPStock (";
        strSQL += "id INT AUTO_INCREMENT PRIMARY KEY,";

        strSQL += "ProductID VARCHAR(32),";           // 料號
        strSQL += "StockID VARCHAR(16),";             // 倉位
        strSQL += "Type VARCHAR(32),";                // 種類

        strSQL += "StockCount double default 0,";       // 存量
        strSQL += "MakeDate TEXT NOT NULL)";            // 製造日期

        //strSQL += "PRIMARY KEY(ProductID,StockID,Type));";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }

    //建立SFCStatus(現況)資料表
    if(!pDB->CheckTableExist("IntaiWeb_SFCStatus"))
    {
        strSQL = "CREATE TABLE IntaiWeb_SFCStatus (";
        strSQL += "id INT AUTO_INCREMENT PRIMARY KEY,";

        strSQL += "ProductID VARCHAR(32),";           // 料號
        strSQL += "ProcessID VARCHAR(32),";           // 工序編號
        strSQL += "Count double default 0,";            // 目前數量
        strSQL += "Type VARCHAR(32))";                // 種類

        //strSQL += "PRIMARY KEY(ProductID,ProcessID,Type));";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }

    //建立SFCHistory(歷程)資料表
    if(!pDB->CheckTableExist("IntaiWeb_SFCHistory"))
    {
        strSQL = "CREATE TABLE IntaiWeb_SFCHistory (";
        strSQL += "id INT AUTO_INCREMENT PRIMARY KEY,";

        strSQL += "ProductID VARCHAR(32),";           // 料號
        strSQL += "ProcessID VARCHAR(32),";             // 工序編號
        strSQL += "Count double default 0,";            // 目前數量
        strSQL += "Type VARCHAR(32))";             // 種類

        //strSQL += "PRIMARY KEY(ProductID,ProcessID,Type));";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }

    //建立Types(歷程)資料表
    if(!pDB->CheckTableExist("IntaiWeb_types"))
    {
        strSQL="CREATE TABLE IntaiWeb_types (";

        strSQL+="TypeID VARCHAR(32),";
        strSQL+="TypeName TEXT NOT NULL,";

        strSQL += "PRIMARY KEY(TypeID));";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }

    //建立出貨資料表
    if(!pDB->CheckTableExist("IntaiWeb_shiptable"))
    {
        strSQL="CREATE TABLE IntaiWeb_shiptable (";
        strSQL+="id INT AUTO_INCREMENT PRIMARY KEY,";

        strSQL+="TypeID VARCHAR(32),";
        strSQL+="ProductID VARCHAR(32),";
        strSQL+="ShipDate VARCHAR(16),";

        strSQL+="TargetCount double,";
        strSQL+="WipCount double)";

        //strSQL += "PRIMARY KEY(TypeID,ProductID,ShipDate));";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }



    //建立進度資料表
    if(!pDB->CheckTableExist("IntaiWeb_runningtables"))
    {
        strSQL="CREATE TABLE IntaiWeb_runningtables (";

        strSQL+="id INT AUTO_INCREMENT PRIMARY KEY,";
        strSQL+="TypeID VARCHAR(32),";
        strSQL+="ProductID VARCHAR(32),";
        strSQL+="ProcessIndex integer,";

        strSQL+="ProcessID VARCHAR(32),";
        strSQL+="WipCount double,";
        strSQL+="TargetCount double,";
        strSQL+="SumCount double";

        strSQL += ");";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }


    //建立列表資料表
    if(!pDB->CheckTableExist("IntaiWeb_titles"))
    {
        strSQL="CREATE TABLE IntaiWeb_titles (";

        strSQL+="id INT AUTO_INCREMENT PRIMARY KEY,";
        strSQL+="TypeID VARCHAR(32),";
        strSQL+="ProductID VARCHAR(32),";
        strSQL+="Members VARCHAR(32)";

        strSQL += ");";
        if (!pDB->ExecuteSQL(strSQL))
        {
            pDB->Close();
            return false;
        }
    }

    pDB->Close();

    CopyDBs();
    return true;

}

bool HElcSignage::CopyDBs()
{
    if(m_pMySQLDB==nullptr)
        return false;

    QString strValue="ProductID,StockID,Type,MakeDate,StockCount";
    CopyDBs("IntaiWeb_ERPStock",strValue.split(","));

    strValue="PartID,CName,Specification,Stock,TypeID";
    CopyDBs("IntaiWeb_part",strValue.split(","));

    strValue="ProcessID,CName";
    CopyDBs("IntaiWeb_process",strValue.split(","));

    strValue="ProcessID,ProductID,PartID,theOrder,DayTarget,Stock,Schedule";
    CopyDBs("IntaiWeb_processlink",strValue.split(","));

    strValue="ProductID,CName,FirstPass,Stock,TypeID";
    CopyDBs("IntaiWeb_product",strValue.split(","));

    strValue="ProductID,PartID,PartCount";
    CopyDBs("IntaiWeb_productlink",strValue.split(","));

    strValue="TypeID,ProductID,ProcessIndex,ProcessID,WipCount,TargetCount,SumCount";
    CopyDBs("IntaiWeb_runningtables",strValue.split(","));

    strValue="ProductID,ProcessID,Type,Count";
    CopyDBs("IntaiWeb_SFCHistory",strValue.split(","));

    strValue="ProductID,ProcessID,Type,Count";
    CopyDBs("IntaiWeb_SFCStatus",strValue.split(","));

    strValue="TypeID,ProductID,ShipDate,TargetCount,WipCount";
    CopyDBs("IntaiWeb_shiptable",strValue.split(","));

    strValue="ProductID,OutDate,OutCount";
    CopyDBs("IntaiWeb_Shipment",strValue.split(","));

    strValue="DataName,IntData,DblData,StrData";
    CopyDBs("IntaiWeb_systemset",strValue.split(","));

    strValue="TypeID,TypeName";
    CopyDBs("IntaiWeb_types",strValue.split(","));


    strValue="TypeID,ProductID,Members";
    CopyDBs("IntaiWeb_titles",strValue.split(","));


    return false;
}

bool HElcSignage::CopyDBs(QString strTable, QStringList Fields)
{
    QString strSQL,strTemp,strFiles,strValues;
    HRecordsetSQLite rsLite;
    HRecordsetMySQL rsMySql;
    int count=0;
    if(!m_pMySQLDB->Open())
        return false;

    strSQL=QString("Select count(*) from %1").arg(strTable);
    if(rsMySql.ExcelSQL(strSQL.toStdWString(),m_pMySQLDB))
    {
        count=0;
        if(rsMySql.GetValue(L"count(*)",count) && count>0)
        {
            m_pMySQLDB->Close();
            return false;
        }
    }
    m_pMySQLDB->Close();

    QVariant qValue;
    std::vector<std::vector<QVariant>> vValues;
    std::vector<QString>    vFields;
    strSQL=QString("Select * from %1").arg(strTable);
    if(!m_LiteDB.Open())
        return false;
    bool bStart=true;
    if(rsLite.ExcelSQL(strSQL.toStdWString(),&m_LiteDB))
    {
        while(!rsLite.isEOF())
        {
            std::vector<QVariant> vTemp;
            foreach (const QString &field, Fields)
            {
                if(rsLite.GetValue(field,qValue))
                {
                    vTemp.push_back(qValue);
                    if(bStart)
                    {
                        if(strFiles.size()<=0)
                            strFiles=field;
                        else
                        {
                            strFiles+=",";
                            strFiles+=field;
                        }
                        vFields.push_back(field);
                    }
                }
            }
            vValues.push_back(vTemp);
            bStart=false;
            rsLite.MoveNext();
        }
    }
    m_LiteDB.Close();

    if(!m_pMySQLDB->Open())
        return false;
    std::vector<std::vector<QVariant>>::iterator itV;
    std::vector<QVariant>::iterator itTemp;
    std::vector<QVariant>* pMapValue;
    for(itV=vValues.begin();itV!=vValues.end();itV++)
    {
        pMapValue=&(*itV);
        strValues.clear();
        for(itTemp=pMapValue->begin();itTemp!=pMapValue->end();itTemp++)
        {
            if((*itTemp).type()==QVariant::Type::String)
            {
                QString strTest=(*itTemp).toString();
                if(strTest.size()<=0)
                    strTemp="''";
                else
                    strTemp=QString("'%1'").arg(strTest);
            }
            else
                strTemp=QString("%1").arg((*itTemp).toString());

            if(strValues.size()<=0)
                strValues=strTemp;
            else
            {
                strValues+=",";
                strValues+=strTemp;
            }
        }
        strSQL=QString("Insert into %1(%2) Values(%3)").arg(strTable).arg(strFiles).arg(strValues);
        m_pMySQLDB->ExecuteSQL(strSQL);
    }


    m_pMySQLDB->Close();

    return true;
}

void HElcSignage::OnCheckShipDataFile()
{
    int nTemp;
    if(gStep!=0)
    {
        nTemp=gStep;
        return;
    }
    if(m_WebService.IsRunning())
        return;

    gStep=stepCheckShipFile;
    start();
}


bool HElcSignage::LoadSysatem()
{

    QString strSQL,strValue;
    int nCount=0;
    std::map<QString, SystemData*>::iterator itMap;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    for(itMap=m_mapSystemDatas.begin();itMap!=m_mapSystemDatas.end();itMap++)
    {
        strSQL=QString("Select Count(*) from IntaiWeb_systemset Where DataName='%1'").arg(itMap->first);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            nCount=0;
            pRS->GetValue(L"Count(*)",nCount);
            if(nCount<=0)
            {
                strSQL=QString("Insert Into IntaiWeb_systemset(DataName,IntData,DblData,StrData) Values('%1',%2,%3,'%4')").arg(
                            itMap->first).arg(
                            itMap->second->nValue).arg(
                            itMap->second->dblValue).arg(
                            itMap->second->strValue);
                pDB->ExecuteSQL(strSQL);
            }
        }
    }

    SystemData* pNewData;
    strSQL="Select * from IntaiWeb_systemset Order by DataName";
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strValue="";
            if(pRS->GetValue(L"DataName",strValue) && strValue.size()>0)
            {
                itMap=m_mapSystemDatas.find(strValue);
                if(itMap!=m_mapSystemDatas.end())
                    pNewData=itMap->second;
                else
                {
                    pNewData=new SystemData();
                    pNewData->name=strValue;
                    m_mapSystemDatas.insert(std::make_pair(pNewData->name,pNewData));
                }
                pRS->GetValue(L"IntData",pNewData->nValue);
                pRS->GetValue(L"DblData",pNewData->dblValue);
                pRS->GetValue(L"StrData",pNewData->strValue);
            }
            pRS->MoveNext();
        }
    }

    delete pRS;
    pDB->Close();


    SystemData *pSData=GetSystemData("BearerToken");
    if(pSData!=nullptr)
        m_WebService.m_strBearerToken=pSData->strValue;

    pSData=GetSystemData("SFCStatus_Link");
    if(pSData!=nullptr)
        m_WebService.m_strStatusLink=pSData->strValue;

    pSData=GetSystemData("SFCHistory_Link");
    if(pSData!=nullptr)
        m_WebService.m_strHistoryLink=pSData->strValue;

    pSData=GetSystemData("ERPWIP_Link");
    if(pSData!=nullptr)
        m_WebService.m_strWipLink=pSData->strValue;

    pSData=GetSystemData("ERPWIP_Login");
    if(pSData!=nullptr)
        m_WebService.m_strERPName=pSData->strValue;

    pSData=GetSystemData("ERPWIP_Pwd");
    if(pSData!=nullptr)
        m_WebService.m_strERPPwd=pSData->strValue;




    return true;
}


bool HElcSignage::CopyProducts(QString type,std::map<QString,QString>& datas)
{
     std::map<QString,Product*>::iterator itMap;
     for(itMap=m_mapProducts.begin();itMap!=m_mapProducts.end();itMap++)
     {
         if(itMap->second->TypeID==type)
            datas.insert(std::make_pair(itMap->second->ProductID,itMap->second->CName));
     }
     return datas.size()>0;

}

bool HElcSignage::NewProducts(std::map<QString, QString> &)
{
    return true;
}


bool HElcSignage::DeleteProduct(QString id)
{
    HDataBase* pDB;
    (m_pMySQLDB==nullptr)?(pDB=&m_LiteDB):(pDB=m_pMySQLDB);
    if(!pDB->Open())
        return false;
    QString strSQL=QString("delete from IntaiWeb_product Where ProductID='%1'").arg(id);
    bool ret=pDB->ExecuteSQL(strSQL);
    pDB->Close();
    if(ret)
    {
        std::map<QString,Product*>::iterator itMap=m_mapProducts.find(id);
        if(itMap!=m_mapProducts.end())
        {
            delete itMap->second;
            m_mapProducts.erase(itMap);
        }
    }
    return ret;

}

bool HElcSignage::SaveProcessDayCount(QString strPID, std::map<QString, int> &datas)
{
    std::map<QString, int>::iterator itMap;
    std::map<QString,Product*>::iterator itP1=m_mapProducts.find(strPID);
    std::map<QString,Part*>::iterator itP2=m_mapParts.find(strPID);
    Product* pProduct=nullptr;
    Part* pPart=nullptr;
    QString strSQL;

    if(itP1!=m_mapProducts.end())
        pProduct=itP1->second;
    if(itP2!=m_mapParts.end())
        pPart=itP2->second;
    if(pPart!=nullptr)
    {
        for(itP1=m_mapProducts.begin();itP1!=m_mapProducts.end();itP1++)
        {
            itMap=itP1->second->parts.find(strPID);
            if(itMap!=itP1->second->parts.end())
            {
                pProduct=itP1->second;
                break;
            }
        }
    }

    if(pProduct==nullptr)
        return false;
    HDataBase* pDB;
    (m_pMySQLDB==nullptr)?(pDB=&m_LiteDB):(pDB=m_pMySQLDB);
    if(!pDB->Open())
        return false;


    for(itMap=datas.begin();itMap!=datas.end();itMap++)
    {
        if(pPart==nullptr)
        {
            strSQL=QString("Update IntaiWeb_processlink Set DayTarget=%1 Where ProcessID='%2' and ProductID='%3'").arg(
                        itMap->second).arg(
                        itMap->first).arg(
                        strPID);
        }
        else
        {
            strSQL=QString("Update IntaiWeb_processlink Set DayTarget=%1 Where ProcessID='%2' and ProductID='%3'").arg(
                        itMap->second).arg(
                        itMap->first).arg(
                        strPID);
        }
        pDB->ExecuteSQL(strSQL);
    }





    pDB->Close();
    return true;
}

bool HElcSignage::GetProductsFromType(QString strType, std::vector<QString> &datas)
{
    std::map<QString,Product*>::iterator itP;
    for(itP=m_mapProducts.begin();itP!=m_mapProducts.end();itP++)
    {
        if(itP->second->TypeID==strType)
            datas.push_back(itP->second->ProductID);
    }
    return datas.size()>0;
}

bool HElcSignage::GetProductStruct(QString pID,Product** pP)
{
    std::map<QString,Product*>::iterator itP=m_mapProducts.find(pID);
    if(!(itP!=m_mapProducts.end()))
        return false;
    (*pP)=itP->second;
    return true;
}

bool HElcSignage::GetProductStruct(QString pID, std::vector<Product *> &datas)
{
    std::map<QString,Product*>::iterator itP;
    Product* pPSource=nullptr;
    bool ret=GetProductStruct(pID,&pPSource);
    if(!ret)
        return false;
    std::list<ProcessInfo>::iterator itS,itT;
    std::list<ProcessInfo> InfoSrc;
    if(!GetProductProcess(pID,InfoSrc))
        return false;

    for(itP=m_mapProducts.begin();itP!=m_mapProducts.end();itP++)
    {
        if(itP->second==pPSource)
        {
            datas.push_back(itP->second);
            continue;
        }
        else
        {
            std::list<ProcessInfo> info;

            if(GetProductProcess(itP->first,info) && info.size()==InfoSrc.size())
            {
                bool bFind=false;
                itT=info.begin();
                for(itS=InfoSrc.begin();itS!=InfoSrc.end();itS++)
                {
                    if(itT->ProcessID!=itS->ProcessID)
                    {
                        bFind=true;
                        break;
                    }
                    itT++;
                }
                if(!bFind)
                    datas.push_back(itP->second);
            }
        }
    }
    return datas.size()>0;
}

bool HElcSignage::GetPartStruct(QString pID, Part **pP)
{
    std::map<QString,Part*>::iterator itP=m_mapParts.find(pID);
    if(!(itP!=m_mapParts.end()))
        return false;
    (*pP)=itP->second;
    return true;
}

bool HElcSignage::GetPartStruct(QString pID, std::vector<Part *> &datas)
{
    std::list<ProcessInfo> InfoSrc;
    Part* pPartSrc=nullptr;
    if(!GetPartStruct(pID,&pPartSrc))
        return false;
    if(!GetPartProcess(pID,InfoSrc))
        return false;

    std::list<ProcessInfo>::iterator itS,itT;
    std::map<QString,Part*>::iterator itP;
    for(itP=m_mapParts.begin();itP!=m_mapParts.end();itP++)
    {
        std::list<ProcessInfo> InfoTar;
        if(itP->first==pID)
        {
            datas.push_back(itP->second);
            continue;
        }
        else if(GetPartProcess(itP->first,InfoTar) && InfoTar.size()==InfoSrc.size())
        {
            bool bFind=false;
            itS=InfoSrc.begin();
            for(itT=InfoTar.begin();itT!=InfoTar.end();itT++)
            {
                if(itT->ProcessID!=itS->ProcessID)
                {
                   bFind=true;
                   break;
                }
                itS++;
            }
            if(!bFind)
                datas.push_back(itP->second);
        }
    }

    return true;
}

bool HElcSignage::GetProductProcess(QString pID, std::list<ProcessInfo> &datas)
{
    std::map<QString,Process*>::iterator itPro;
    std::list<ProcessInfo>::iterator itV;
    std::map<QString,Product*>::iterator itP=m_mapProducts.find(pID);
    Product* pProduct;
    datas.clear();
    if(!(itP!=m_mapProducts.end()))
        return false;
    pProduct=itP->second;


    QString strID,strProcess,strSQL;
    double dblValue,dbl3001=0;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }
    //strSQL=QString("select StockCount from IntaiWeb_ERPStock Where ProductID='%1' and StockID='3001' and Type='%2'").arg(pID).arg(m_TypeSelect);
    strSQL=QString("select StockCount from IntaiWeb_ERPStock Where ProductID='%1' and StockID='3001'").arg(pID);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        pRS->GetValue(L"StockCount",dbl3001);


    strSQL=QString("select * from IntaiWeb_processlink Where ProductID='%1' and PartID='%1' order by theOrder").arg(pID);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strID.clear();
            if(pRS->GetValue(L"ProcessID",strID) && strID.size()>0)
            {
                ProcessInfo info;
                info.ProductID=pID;
                info.ProcessID=strID;
                if(pRS->GetValue(L"theOrder",info.order))
                {
                   itPro=m_mapProcesses.find(strID);
                   if(itPro!=m_mapProcesses.end())
                   {
                        info.DayTarget=0;
                       pRS->GetValue(L"DayTarget",info.DayTarget);
                        info.ProcessName=itPro->second->CName;

                        if(info.ProcessID=="3001")
                            info.Target=static_cast<int>(dbl3001);
                        else
                            info.Target=0;
                        info.Source=0;
                        info.sum=0;
                        datas.push_back(info);
                   }
                }
            }
            pRS->MoveNext();
        }
    }

    ProcessInfo* pInfo;
    //strSQL=QString("select * from SFCStatus Where ProductID='%1' and Type='%2'").arg(pID).arg(m_TypeSelect);
    strSQL=QString("select * from IntaiWeb_SFCStatus Where ProductID='%1'").arg(pID);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strID.clear();
            if(pRS->GetValue(L"ProcessID",strID) && strID.size()>0)
            {
                strProcess.clear();
                if(pRS->GetValue(L"ProcessID",strProcess) && strProcess.size()>0)
                {
                    if(pRS->GetValue(L"Count",dblValue))
                    {
                        itV=datas.begin();
                        for(size_t i=0;i<datas.size();i++)
                        {
                            pInfo=&(*itV);
                            if(pInfo->ProcessID==strProcess)
                            {
                                pInfo->Target=static_cast<int>(dblValue);
                                break;
                            }
                            itV++;
                        }
                    }
                }
            }
            pRS->MoveNext();
        }
    }

    int sum=0;
    strSQL=QString("select sum(OutCount) from IntaiWeb_Shipment Where ProductID='%1'").arg(pID);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        sum=0;
        pRS->GetValue(L"sum(OutCount)",sum);
        sum=sum*(2-pProduct->FirstPass);
    }

    ProcessInfo* pOld=nullptr;
    for(itV=datas.begin();itV!=datas.end();itV++)
    {
        pInfo=&(*itV);
        if(itV==datas.begin())
        {
            if(pInfo->ProcessID!="3001")
                break;
            pInfo->Source=pInfo->Target-sum;
        }
        else if(pOld!=nullptr)
        {
            pInfo->Source=pOld->Source+pInfo->Target;
        }
        pOld=pInfo;
    }


    // from sfc歷程表
    double dblCount;
    for(itV=datas.begin();itV!=datas.end();itV++)
    {
        pInfo=&(*itV);
        pInfo->sum=0;
        strSQL=QString("Select * from IntaiWeb_SFCHistory Where ProductID='%1' and ProcessID='%2' and Type='%3'").arg(pID).arg(pInfo->ProcessID).arg(m_TypeSelect);
        if(pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) && !pRS->isEOF())
        {
            dblCount=0;
            if(pRS->GetValue(L"Count",dblCount))
                pInfo->sum=static_cast<int>(dblCount);
        }

    }



    delete pRS;
    pDB->Close();
    return datas.size()>0;
}

bool HElcSignage::SavetProductProcess(QString pID, std::list<ProcessInfo> &datas)
{

    QString strID,strProcess,strSQL;
    int nCount=0;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    strSQL=QString("select Count(*) From IntaiWeb_processlink Where ProductID='%1'").arg(pID);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        if(pRS->GetValue(L"Count(*)",nCount))
        {
            if(nCount > static_cast<int>(datas.size()))
            {
                strSQL=QString("delete IntaiWeb_processlink Where ProductID='%1' and theOrder>=%2").arg(pID).arg(datas.size());
                pDB->ExecuteSQL(strSQL);
            }
        }
    }

    ProcessInfo* pInfo;
    int index=0;
    std::list<ProcessInfo>::iterator itL;
    for(itL=datas.begin();itL!=datas.end();itL++)
    {
        pInfo=&(*itL);
        nCount=-1;
        strSQL=QString("select Count(*) From IntaiWeb_processlink Where ProcessID='%1' and ProductID='%2' and PartID='%3'").arg(
                    pInfo->ProcessID).arg(
                    pID).arg(
                    pInfo->ProductID);
        if(pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) && pRS->GetValue(L"Count(*)",nCount) && nCount>0)
        {
            strSQL=QString("update IntaiWeb_processlink set theOrder=%1 Where ProcessID='%2' and ProductID='%3' and PartID='%4'").arg(
                        index).arg(
                        pInfo->ProcessID).arg(
                        pID).arg(
                        pInfo->ProductID);
        }
        else
        {
            strSQL=QString("insert into IntaiWeb_processlink(ProcessID,ProductID,PartID,theOrder,DayTarget,Stock,Schedule) Values('%1','%2','%3',%4,%5,%6,%7)").arg(
                        pInfo->ProcessID).arg(
                        pID).arg(
                        pInfo->ProductID).arg(
                        index).arg(
                        0).arg(
                        0).arg(
                        0);
        }
        pDB->ExecuteSQL(strSQL);
        index++;
    }

    delete pRS;
    pDB->Close();
    return true;
}

bool HElcSignage::RemoveProductProcess(QString pID, QString Product, QString Part)
{
    QString strSQL;
    HDataBase* pDB;
    (m_pMySQLDB==nullptr)?(pDB=&m_LiteDB):(pDB=m_pMySQLDB);
    if(!pDB->Open())
        return false;
    strSQL=QString("delete from IntaiWeb_processlink Where ProcessID='%1' and ProductID='%2' and PartID='%3'").arg(
                pID).arg(
                Product).arg(
                Part);

    bool ret=pDB->ExecuteSQL(strSQL);
    pDB->Close();
    return ret;
}

bool HElcSignage::GetPartProcess(QString part, std::list<ProcessInfo> &datas)
{
    int needed=0;
    if(!GetPartCountSum(part,needed))
    {

    }

    std::map<QString,int>::iterator itC;
    std::map<QString,double>::iterator itC2;
    std::map<QString,Process*>::iterator itPro;
    std::list<ProcessInfo>::iterator itV;
    std::map<QString,Part*>::iterator itP=m_mapParts.find(part);
    Part* pPart;
    datas.clear();
    if(!(itP!=m_mapParts.end()))
        return false;
    pPart=itP->second;

    QString strID,strProcess,strSQL;
    double dblValue;
    std::map<QString,double> map2001,map2002;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    dblValue=0;
    //strSQL=QString("select StockCount from ERPStock Where ProductID='%1' and StockID='2001' and Type='%2'").arg(part).arg(m_TypeSelect);
    strSQL=QString("select StockCount from IntaiWeb_ERPStock Where ProductID='%1' and StockID='2001'").arg(part);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        if(pRS->GetValue(L"StockCount",dblValue))
            map2001.insert(std::make_pair(part,dblValue));
    }
    if(abs(dblValue)<=0.00001  && pPart->parts.size()>0)
    {
        for(itC=pPart->parts.begin();itC!=pPart->parts.end();itC++)
        {
            strSQL=QString("select StockCount from IntaiWeb_ERPStock Where ProductID='%1' and StockID='2001' and Type='%2'").arg(itC->first).arg(m_TypeSelect);
            if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
            {
                if(pRS->GetValue(L"StockCount",dblValue))
                    map2001.insert(std::make_pair(itC->first,dblValue));
            }
        }
    }

    dblValue=0;
    //strSQL=QString("select StockCount from ERPStock Where ProductID='%1' and StockID='2002' and Type='%2'").arg(part).arg(m_TypeSelect);
    strSQL=QString("select StockCount from IntaiWeb_ERPStock Where ProductID='%1' and StockID='2002'").arg(part);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        if(pRS->GetValue(L"StockCount",dblValue))
            map2002.insert(std::make_pair(part,dblValue));
    }
    if(abs(dblValue)<=0.00001 && pPart->parts.size()>0)
    {
        for(itC=pPart->parts.begin();itC!=pPart->parts.end();itC++)
        {
            strSQL=QString("select StockCount from IntaiWeb_ERPStock Where ProductID='%1' and StockID='2002' and Type='%2'").arg(itC->first).arg(m_TypeSelect);
            if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
            {
                if(pRS->GetValue(L"StockCount",dblValue))
                    map2002.insert(std::make_pair(itC->first,dblValue));
            }
        }
    }
    pDB->Close();

    GetPartPariantID(part,part,"");

    pDB->Open();
    strSQL=QString("select * from IntaiWeb_processlink Where ProductID='%1' order by theOrder").arg(part);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strID.clear();
            if(pRS->GetValue(L"ProcessID",strID) && strID.size()>0)
            {
                ProcessInfo info;
                //info.ProductID=part;
                info.ProcessID=strID;
                if(pRS->GetValue(L"theOrder",info.order))
                {
                   pRS->GetValue(L"PartID",info.ProductID);
                   itPro=m_mapProcesses.find(strID);
                   if(itPro!=m_mapProcesses.end())
                   {
                        info.ProcessName=itPro->second->CName;
                        info.Source=0;
                        if(info.ProcessID=="2001")
                        {
                            itC2=map2001.find(info.ProductID);
                            if(itC2!=map2001.end())
                                info.Target=static_cast<int>(itC2->second);
                            info.Source=info.Target+needed;
                        }
                        else if(info.ProcessID=="2002")
                        {
                            itC2=map2002.find(info.ProductID);
                            if(itC2!=map2002.end())
                                info.Target=static_cast<int>(itC2->second);
                        }
                        else
                            info.Target=0;
                        info.DayTarget=0;
                        pRS->GetValue(L"DayTarget",info.DayTarget);

                        datas.push_back(info);
                   }
                }
            }
            pRS->MoveNext();
        }
    }




    ProcessInfo* pInfo;
    //strSQL=QString("select * from SFCStatus Where ProductID='%1' and Type='%2'").arg(part).arg(m_TypeSelect);
    strSQL=QString("select * from IntaiWeb_SFCStatus Where ProductID='%1'").arg(part);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strID.clear();
            if(pRS->GetValue(L"ProcessID",strID) && strID.size()>0)
            {
                strProcess.clear();
                if(pRS->GetValue(L"ProcessID",strProcess) && strProcess.size()>0)
                {
                    if(pRS->GetValue(L"Count",dblValue))
                    {
                        itV=datas.begin();
                        for(size_t i=0;i<datas.size();i++)
                        {
                            pInfo=&(*itV);
                            if(pInfo->ProcessID==strProcess)
                            {
                                pInfo->Target=static_cast<int>(dblValue);
                                break;
                            }
                            itV++;
                        }
                    }
                }
            }
            pRS->MoveNext();
        }
    }


    ProcessInfo* pOld=nullptr;
    for(itV=datas.begin();itV!=datas.end();itV++)
    {
        pInfo=&(*itV);
        if(itV==datas.begin())
        {
            if(pInfo->ProcessID!="2001" && pInfo->ProcessID!="2002")
                break;
        }
        else if(pOld!=nullptr)
        {
            pInfo->Source=pOld->Source+pInfo->Target;
        }
        pOld=pInfo;
    }


    // from sfc歷程表
    double dblCount;
    for(itV=datas.begin();itV!=datas.end();itV++)
    {
        pInfo=&(*itV);
        pInfo->sum=0;
        strSQL=QString("Select * from IntaiWeb_SFCHistory Where ProductID='%1' and ProcessID='%2' and Type='%3'").arg(pInfo->ProductID).arg(pInfo->ProcessID).arg(m_TypeSelect);
        if(pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) && !pRS->isEOF())
        {
            dblCount=0;
            if(pRS->GetValue(L"Count",dblCount))
                pInfo->sum=static_cast<int>(dblCount);
        }

    }

    delete pRS;
    pDB->Close();
    return datas.size()>0;
}

/*
bool HElcSignage::GetPartProcess1(QString productID,QString part, std::list<ProcessInfo> &datas)
{
    int needed=0;
    if(!GetPartCountSum(part,needed))
    {

    }


    std::map<QString,Process*>::iterator itPro;
    std::list<ProcessInfo>::iterator itV;
    std::map<QString,Part*>::iterator itP=m_mapParts.find(part);
    Part* pPart;
    datas.clear();
    if(!(itP!=m_mapParts.end()))
        return false;
    pPart=itP->second;

    QString strID,strProcess,strSQL;
    double dblValue,dbl2001=0,dbl2002=0;
    if(!pDB->Open())
        return false;
    //strSQL=QString("select StockCount from ERPStock Where ProductID='%1' and StockID='2001' and Type='%2'").arg(productID).arg(m_TypeSelect);
    strSQL=QString("select StockCount from ERPStock Where ProductID='%1' and StockID='2001'").arg(productID);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
        rs.GetValue(L"StockCount",dbl2001);
    //strSQL=QString("select StockCount from ERPStock Where ProductID='%1' and StockID='2002' and Type='%2'").arg(productID).arg(m_TypeSelect);
    strSQL=QString("select StockCount from ERPStock Where ProductID='%1' and StockID='2002'").arg(productID);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
        rs.GetValue(L"StockCount",dbl2002);

    pDB->Close();
    pDB->Open();
    //strSQL=QString("select * from ProcessLink Where ProductID='%1' and Type='%2' order by theOrder").arg(productID).arg(m_TypeSelect);
    strSQL=QString("select * from ProcessLink Where ProductID='%1' order by theOrder").arg(productID);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!rs.isEOF())
        {
            strID.clear();
            if(rs.GetValue(L"ProcessID",strID) && strID.size()>0)
            {
                ProcessInfo info;
                //info.ProductID=part;
                info.ProcessID=strID;
                if(rs.GetValue(L"theOrder",info.order))
                {
                   rs.GetValue(L"PartID",info.ProductID);
                   itPro=m_mapProcesses.find(strID);
                   if(itPro!=m_mapProcesses.end())
                   {
                        info.ProcessName=itPro->second->CName;
                        info.Source=0;
                        if(info.ProcessID=="2001")
                        {
                            info.Target=static_cast<int>(dbl2001);
                            info.Source=info.Target+needed;
                        }
                        else if(info.ProcessID=="2002")
                            info.Target=static_cast<int>(dbl2002);
                        else
                            info.Target=0;
                        info.DayTarget=0;
                        rs.GetValue(L"DayTarget",info.DayTarget);

                        datas.push_back(info);
                   }
                }
            }
            rs.MoveNext();
        }
    }




    ProcessInfo* pInfo;
    //strSQL=QString("select * from SFCStatus Where ProductID='%1' and Type='%2'").arg(part).arg(m_TypeSelect);
    strSQL=QString("select * from SFCStatus Where ProductID='%1'").arg(part);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!rs.isEOF())
        {
            strID.clear();
            if(rs.GetValue(L"ProcessID",strID) && strID.size()>0)
            {
                strProcess.clear();
                if(rs.GetValue(L"ProcessID",strProcess) && strProcess.size()>0)
                {
                    if(rs.GetValue(L"Count",dblValue))
                    {
                        itV=datas.begin();
                        for(size_t i=0;i<datas.size();i++)
                        {
                            pInfo=&(*itV);
                            if(pInfo->ProcessID==strProcess)
                            {
                                pInfo->Target=static_cast<int>(dblValue);
                                break;
                            }
                            itV++;
                        }
                    }
                }
            }
            rs.MoveNext();
        }
    }


    ProcessInfo* pOld=nullptr;
    for(itV=datas.begin();itV!=datas.end();itV++)
    {
        pInfo=&(*itV);
        if(itV==datas.begin())
        {
            if(pInfo->ProcessID!="2001" && pInfo->ProcessID!="2002")
                break;
        }
        else if(pOld!=nullptr)
        {
            pInfo->Source=pOld->Source+pInfo->Target;
        }
        pOld=pInfo;
    }


    // from sfc歷程表
    double dblCount;
    for(itV=datas.begin();itV!=datas.end();itV++)
    {
        pInfo=&(*itV);
        pInfo->sum=0;
        strSQL=QString("Select * from SFCHistory Where ProductID='%1' and ProcessID='%2' and Type='%3'").arg(pInfo->ProductID).arg(pInfo->ProcessID).arg(m_TypeSelect);
        if(rs.ExcelSQL(strSQL.toStdWString(),&m_LiteDB) && !rs.isEOF())
        {
            dblCount=0;
            if(rs.GetValue(L"Count",dblCount))
                pInfo->sum=static_cast<int>(dblCount);
        }

    }

    pDB->Close();
    return datas.size()>0;
}

*/

bool HElcSignage::GetPartProcess(QString part, std::list<QString> &datas)
{
    std::map<QString,Process*>::iterator itPro;
    std::list<ProcessInfo>::iterator itV;
    std::map<QString,Part*>::iterator itP=m_mapParts.find(part);
    Part* pPart;
    datas.clear();
    if(!(itP!=m_mapParts.end()))
        return false;
    pPart=itP->second;

    QString strID,strProcess,strSQL;

    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    strSQL=QString("select * from IntaiWeb_processlink Where ProductID='%1' order by theOrder").arg(part);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strID.clear();
            if(pRS->GetValue(L"ProcessID",strID) && strID.size()>0)
            {
               datas.push_back(strID);
            }
            pRS->MoveNext();
        }
    }


    delete pRS;
    pDB->Close();
    return datas.size()>0;
}

bool HElcSignage::GetPartPariantID(QString partID, QString &PariantID,QString pariDefault)
{
    QString strTemp="";
    std::map<QString,int>::iterator itChild;
    std::vector<QString> vTemps;
    std::map<QString,Part*>::iterator itPart=m_mapParts.find(partID);
    if(itPart!=m_mapParts.end())// && itPart->second->parts.size()>0)  
    {
        for(itPart=m_mapParts.begin();itPart!=m_mapParts.end();itPart++)
        {
            itChild=itPart->second->parts.find(partID);
            if(itChild!=itPart->second->parts.end())
            {
                /*
                strTemp=itPart->first;
                break;
                */
                vTemps.push_back(itPart->first);
            }
        }
    }
    else
    {
        //strTemp=partID;
        PariantID=partID;
        return true;
     }

    if(vTemps.size()<=0)
        return false;
    for(int i=0;i<vTemps.size();i++)
    {
        if(vTemps[i]==pariDefault)
        {
            PariantID=pariDefault;
            return true;
        }
    }
    PariantID=vTemps[0];
    return true;
}

bool HElcSignage::GetPartCountSum(QString strPart, int &needed)
{
    std::list<ProcessInfo>::iterator itInfo;
    std::list<ProcessInfo> lstProductInfos;

     needed=0;
     HRecordset* pRS;
     HDataBase* pDB;
     if(m_pMySQLDB==nullptr)
     {
         pDB=&m_LiteDB;
         pRS=new HRecordsetSQLite();
     }
     else
     {
         pDB=m_pMySQLDB;
         pRS=new HRecordsetMySQL();
     }
     if(!pDB->Open())
     {
         delete pRS;
         return false;
     }

    std::map<QString,double>::iterator itMap;
    std::map<QString,double> mapParducts;
    int nCount;
    //QString strID,strSQL=QString("select * from IntaiWeb_productlink Where ProductID='%1' and PartID='%2'").arg(product).arg(strPart);
    QString strID,strSQL=QString("select * from IntaiWeb_productlink Where  PartID='%1'").arg(strPart);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            nCount=0;
            strID.clear();
            pRS->GetValue(L"ProductID",strID);
            if(pRS->GetValue(L"PartCount",nCount) && nCount>0 && strID.size()>0)
            {
                itMap=mapParducts.find(strID);
                if(itMap!=mapParducts.end())
                    itMap->second+=nCount;
                else
                    mapParducts.insert(std::make_pair(strID,nCount));
            }
            pRS->MoveNext();
        }
    }

    int n3001=0;
    for(itMap=mapParducts.begin();itMap!=mapParducts.end();itMap++)
    {
        lstProductInfos.clear();
        if(!GetProductProcess(itMap->first,lstProductInfos))
            continue;
        itInfo=lstProductInfos.end();
        itInfo--;
        if(itInfo!=lstProductInfos.end())
            n3001=static_cast<int>((*itInfo).Source);
        needed+=(n3001*itMap->second);
    }


    delete pRS;
    pDB->Close();
    return false;
}

bool HElcSignage::LoadDB()
{
    std::map<QString,Product*>::iterator itP;// m_mapProducts;
    std::map<QString,Part*>::iterator itPart;//    m_mapParts;
    std::map<QString,Process*>::iterator itPro;// m_mapProcesses;

    QString strValue,strSQL;

    ClearProducts();
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    strSQL="Select * from IntaiWeb_process Order by ProcessID";
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strValue="";
            if(pRS->GetValue(L"ProcessID",strValue) && strValue.size()>0)
            {
                Process* pProcess=new Process();
                pProcess->ProcessID=strValue;
                pRS->GetValue(L"CName",pProcess->CName);

                m_mapProcesses.insert(std::make_pair(pProcess->ProcessID,pProcess));
            }
            pRS->MoveNext();
        }
    }

    delete pRS;
    pDB->Close();
    LoadSysatem();
    LoadProducts();
    LoadParts();
    LoadProductLink();
    return true;
}

bool HElcSignage::LoadProducts()
{

    QString strSQL,strValue;
    std::map<QString, Product *>::iterator itMap;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    for(itMap=m_mapProducts.begin();itMap!=m_mapProducts.end();itMap++)
        delete itMap->second;
    m_mapProducts.clear();

    strSQL="Select * from IntaiWeb_product Order by ProductID";
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strValue="";
            if(pRS->GetValue(L"ProductID",strValue) && strValue.size()>0)
            {
                Product* pProduct=new Product();
                pProduct->ProductID=strValue;
                pRS->GetValue(L"CName",pProduct->CName);
               pRS->GetValue(L"FirstPass",pProduct->FirstPass);
                pRS->GetValue(L"Stock",pProduct->Stock);
                pRS->GetValue(L"TypeID",pProduct->TypeID);
                m_mapProducts.insert(std::make_pair(pProduct->ProductID,pProduct));
            }
            pRS->MoveNext();
        }
    }

    delete pRS;
    pDB->Close();

    return true;
}

bool HElcSignage::LoadParts()
{

    QString strSQL,strValue;
    std::map<QString, Part *>::iterator itMap;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    for(itMap=m_mapParts.begin();itMap!=m_mapParts.end();itMap++)
        delete itMap->second;
    m_mapParts.clear();

    strSQL="Select * from IntaiWeb_part Order by PartID";
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strValue="";
            if(pRS->GetValue(L"PartID",strValue) && strValue.size()>0)
            {
                Part* pPart=new Part();
                pPart->PartID=strValue;
                pRS->GetValue(L"CName",pPart->CName);
                pRS->GetValue(L"Specification",pPart->Specification);
                pRS->GetValue(L"Stock",pPart->Stock);
                pRS->GetValue(L"TypeID",pPart->TypeID);
                if(pPart->PartID.size()>0)
                    m_mapParts.insert(std::make_pair(pPart->PartID,pPart));
                else
                    delete pPart;
            }
            pRS->MoveNext();
        }
    }

    delete pRS;
    pDB->Close();
    return false;
}



bool HElcSignage::SaveDB()
{

    return true;
}


bool HElcSignage::LoadProductLink()
{

    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }
    int nValue;
    QString strID,strSQL;

    std::map<QString, Product *>::iterator itMap;
    for(itMap=m_mapProducts.begin();itMap!=m_mapProducts.end();itMap++)
    {
        itMap->second->parts.clear();
        strSQL=QString("select * from IntaiWeb_productlink Where ProductID='%1' and PartCount > 0").arg(itMap->second->ProductID);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            while(!pRS->isEOF())
            {
                strID="";
                nValue=0;
                if(pRS->GetValue(L"PartID",strID) && strID.size()>0)
                {
                    if(pRS->GetValue(L"PartCount",nValue) && nValue>0>0)
                        itMap->second->parts.insert(std::make_pair(strID,nValue));
                }
                pRS->MoveNext();
            }
        }
    }

    std::map<QString, Part*>::iterator itPart;
    for(itPart=m_mapParts.begin();itPart!=m_mapParts.end();itPart++)
    {
        itPart->second->parts.clear();
        strSQL=QString("select * from IntaiWeb_productlink Where ProductID='%1' and PartCount > 0").arg(itPart->second->PartID);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            while(!pRS->isEOF())
            {
                strID="";
                nValue=0;
                if(pRS->GetValue(L"PartID",strID) && strID.size()>0)
                {
                    if(pRS->GetValue(L"PartCount",nValue) && nValue>0>0)
                        itPart->second->parts.insert(std::make_pair(strID,nValue));
                }
                pRS->MoveNext();
            }
        }
    }
    delete pRS;
    pDB->Close();
    return true;
}

bool HElcSignage::SaveProducts(QString type,std::map<QString, Product *> &datas)
{

    std::map<QString, Product *>::iterator itMap;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    Product* pProduct;
    QString strSQL,strID;
    int nCount=0;
    std::map<QString,QString>::iterator itDel;
    std::map<QString,QString> mapDeletes;

    strSQL=QString("select * from IntaiWeb_product Where TypeID='%1'").arg(type);
    if(!pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    while(!pRS->isEOF())
    {
        if(pRS->GetValue(L"ProductID",strID) && strID.size()>0)
            mapDeletes.insert(std::make_pair(strID,strID));
        pRS->MoveNext();
    }


    for(itMap=datas.begin();itMap!=datas.end();itMap++)
    {
        pProduct=itMap->second;
        itDel=mapDeletes.find(pProduct->ProductID);
        if(itDel!=mapDeletes.end())
            mapDeletes.erase(itDel);
        if(pProduct->ProductID.size()<=0)
            continue;
        //m_mapProducts.insert(std::make_pair(itMap->first,pProduct));
        strSQL=QString("select Count(*) from IntaiWeb_product Where ProductID='%1'").arg(pProduct->ProductID);
        if(!pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) || !pRS->GetValue(L"Count(*)",nCount))
            continue;
        if(nCount<=0)
            strSQL=QString("insert into IntaiWeb_product(ProductID,CName,FirstPass,Stock,TypeID) Values('%1','%2',%3,%4,'%5')").arg(
                        pProduct->ProductID).arg(
                        pProduct->CName).arg(
                        pProduct->FirstPass).arg(
                        pProduct->Stock).arg(
                        pProduct->TypeID);
        else
            strSQL=QString("update IntaiWeb_product set CName='%2',FirstPass=%3,Stock=%4,TypeID='%5' Where ProductID='%1'").arg(
                        pProduct->ProductID).arg(
                        pProduct->CName).arg(
                        pProduct->FirstPass).arg(
                        pProduct->Stock).arg(
                        pProduct->TypeID);
        pDB->ExecuteSQL(strSQL);

    }
    datas.clear();

    for(itDel=mapDeletes.begin();itDel!=mapDeletes.end();itDel++)
    {
        strSQL=QString("delete from IntaiWeb_product Where ProductID='%1'").arg(itDel->first);
        pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB);
    }


    delete pRS;
    pDB->Close();
    LoadProducts();
    LoadProductLink();
    return true;
}

bool HElcSignage::SaveParts(QString pariant,std::map<QString, Part *> &datas)
{
    std::map<QString, Part *>::iterator itSource;
    std::map<QString, Part *>::iterator itMap;
    std::map<QString,int>::iterator itDel;
    HDataBase* pDB;
    (m_pMySQLDB==nullptr)?(pDB=&m_LiteDB):(pDB=m_pMySQLDB);
    if(!pDB->Open())
        return false;


    Part* pPart;
    Part* pDelete;
    QString strSQL;

    for(itMap=datas.begin();itMap!=datas.end();itMap++)
    {
        pPart=itMap->second;
        if(pPart->parts.size()<=0)
            continue;
        itSource=m_mapParts.find(itMap->first);
        if(itSource!=m_mapParts.end())
        {
            pDelete=itSource->second;
            strSQL=QString("delete from IntaiWeb_PorductLink Where ProductID='%1'").arg(pDelete->PartID);
            pDB->ExecuteSQL(strSQL);
            delete pDelete;
            m_mapParts.erase(itSource);
        }
        if(pPart->PartID.size()>0)
            m_mapParts.insert(std::make_pair(pPart->PartID,pPart));
        else
            delete pPart;

        strSQL=QString("insert into IntaiWeb_product(PartID,CName,Specification,Stock,TypeID) Values('%1','%2','%3',%4,'%5')").arg(
                    pPart->PartID).arg(
                    pPart->CName).arg(
                    pPart->Specification).arg(
                    pPart->Stock).arg(
                    pPart->TypeID);
        pDB->ExecuteSQL(strSQL);


    }
    datas.clear();


    pDB->Close();
    LoadParts();
    LoadProductLink();
    return true;
}

bool HElcSignage::GetTypeLists(std::map<QString, QString> &datas)
{
    if(datas.size()>0)
        return false;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    QString strID,strValue,strSQL="select * from IntaiWeb_types";
    if(!pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    while(!pRS->isEOF())
    {
        if(pRS->GetValue(L"TypeID",strID) && strID.size()>0)
        {
            pRS->GetValue(L"TypeName",strValue);
            datas.insert(std::make_pair(strID,strValue));
        }
        pRS->MoveNext();
    }
    delete pRS;
    pDB->Close();
    return datas.size()>0;
}

bool HElcSignage::SaveTypeLists(std::map<QString, QString> &datas)
{

    int nCount=0;
    if(datas.size()<=0)
        return false;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    QString strID,strValue,strSQL;
    std::map<QString,QString>::iterator itMap;
    for(itMap=datas.begin();itMap!=datas.end();itMap++)
    {
        strSQL=QString("select count(*) from IntaiWeb_types Where TypeID='%1'").arg(itMap->first);
        if(!pRS->ExcelSQL(strSQL.toStdWString(),pDB))
            continue;
        if(pRS->GetValue(L"count(*)",nCount))
        {
            if(nCount<=0)
            {
                strSQL=QString("insert into IntaiWeb_types(TypeID,TypeName) Values('%1','%2')").arg(
                            itMap->first).arg(itMap->second);
            }
            else
            {
                strSQL=QString("update IntaiWeb_types set TypeName='%2' Where TypeID='%1'").arg(
                            itMap->first).arg(itMap->second);
            }
            pDB->ExecuteSQL(strSQL);
        }
    }
    delete pRS;
    pDB->Close();
    return true;
}

bool HElcSignage::DeleteType(QString type)
{
    HDataBase* pDB;
    (m_pMySQLDB==nullptr)?(pDB=&m_LiteDB):(pDB=m_pMySQLDB);
    if(!pDB->Open())
        return false;
    QString strSQL=QString("delete from IntaiWeb_types Where TypeID='%1'").arg(type);
    bool ret=pDB->ExecuteSQL(strSQL);
    pDB->Close();
    return ret;
}

QString HElcSignage::GetTypeFullName(QString type)
{

    QString strReturn;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return strReturn;
    }

    QString strSQL=QString("select * from IntaiWeb_types Where TypeID='%1'").arg(type);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        pRS->GetValue(L"TypeName",strReturn);
    }

    delete pRS;
    pDB->Close();
    return strReturn;
}

bool HElcSignage::GetSFC()
{
    QString strLink="https://win2016-testvm.intai-corp.com:5004/api/me";
    //strLink+="/getsfchis?site=1002&order=1000023241"; // SFC歷程
    strLink+= "/getsfcwip?site=1002&order=1000023241";  // SFC 現況

    QUrl url(strLink);
    QNetworkRequest request(url);

    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone); // Disable SSL
    request.setSslConfiguration(sslConfig);

    QString bearerToken = "69cfa069bba26b8bcfd0913501b172f8";
    QByteArray authHeader = "Bearer " + bearerToken.toUtf8();
    request.setRawHeader("Authorization", authHeader);


    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString jsonString;
    if (reply->error() == QNetworkReply::NoError)
    {
        jsonString = reply->readAll();
        qDebug() << "Response:" << jsonString;
    } else
    {
        qDebug() << "Request error:" << reply->errorString();
        reply->deleteLater();
        return false;
    }



    // 解析JSON字符串
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (!jsonDoc.isNull())
    {
        if (jsonDoc.isArray())
        {
            QJsonArray jsonArray = jsonDoc.array();
            foreach (const QJsonValue& value , jsonArray)
            {
                if (value.isObject())
                {
                    QJsonObject jsonObj = value.toObject();
                    QString item =  jsonObj["item"].toString();
                    QString order=  jsonObj["shop_Order"].toString();
                    QString sfc=  jsonObj["sfc"].toString();
                    //int sequence = jsonObj["steP_SEQUENCE"].toInt();
                    QString operation = jsonObj["operation"].toString();
                    QString oP_DESC = jsonObj["oP_DESC"].toString();
                    double iN_WORK = jsonObj["iN_WORK"].toDouble();
                    QString statuS_DESCRIPTION = jsonObj["statuS_DESCRIPTION"].toString();

                    qDebug() << "item:" << item;
                    qDebug() << "operation:" << operation;
                    qDebug() << "iN_WORK:" << iN_WORK;
                }
            }
        }
        else
        {
            qDebug() << "jsonDoc is not Array.";
            reply->deleteLater();
            return false;
        }
    }
    else
    {
        qDebug() << "Failed to parse JSON.";
        reply->deleteLater();
        return false;
    }



    reply->deleteLater();
    return true;
}

bool HElcSignage::RunUpdateDataBase()
{
    if(gStep!=0 || m_WebService.IsRunning())
        return false;

    gUpdateCount=4*(m_mapProducts.size()+m_mapParts.size());
    m_nRunUpdateIndex=0;


    std::map<QString,Product*>::iterator itProduct;
    std::map<QString,QString> ProductInfos;
    for(itProduct=m_mapProducts.begin();itProduct!=m_mapProducts.end();itProduct++)
    {
        QString strType=itProduct->second->TypeID;
        if(strType.size()>6)
            ProductInfos.insert(std::make_pair(itProduct->first,strType.left(6)));
        else
            ProductInfos.insert(std::make_pair(itProduct->first,strType));
    }
    GetShipMents(ProductInfos,m_shipments);
    gUpdateCount+=(2*m_shipments.size());

    gUpdateCount+=m_mapProcess.size();

    gStep=1;
    start();
    emit OnAutoUpdate(true);
    return true;
}

bool HElcSignage::IsRunning()
{
    return gStep!=0;
}




bool HElcSignage::CopyProcess(std::map<QString, QString> &datas)
{
    std::map<QString,Process*>::iterator itMap;
    for(itMap=m_mapProcesses.begin();itMap!=m_mapProcesses.end();itMap++)
    {
        datas.insert(std::make_pair(itMap->first,itMap->second->CName));
    }
    return datas.size()>0;
}

bool HElcSignage::SaveProcess(QString id, QString name)
{
    int nCount=0;
    bool ret=false;
    std::map<QString,Process*>::iterator itMap=m_mapProcesses.find(id);
    Process* pNew=nullptr;
    QString strSQL;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }
    strSQL=QString("Select Count(*) from IntaiWeb_process Where ProcessID='%1'").arg(id);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        if(!pRS->GetValue(L"Count(*)",nCount) || nCount<=0)
            strSQL=QString("insert into IntaiWeb_process(ProcessID,CName) Values('%1','%2')").arg(id).arg(name);
        else
            strSQL=QString("update IntaiWeb_process set CName='%2' Where ProcessID='%1'").arg(id).arg(name);
        if(!pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            delete pRS;
            pDB->Close();
            return false;
        }

        if(itMap!=m_mapProcesses.end())
        {
            pNew=itMap->second;
            pNew->CName=name;
        }
        else
        {
            pNew=new Process();
            pNew->ProcessID=id;
            pNew->CName=name;
            m_mapProcesses.insert(std::make_pair(id,pNew));
        }
        ret=true;
    }

    delete pRS;
    pDB->Close();
    return ret;
}

bool HElcSignage::CheckProcess(std::list<ProcessInfo> &info1, std::list<ProcessInfo> &info2)
{
    if(info1.size()!=info2.size())
        return false;
    std::list<ProcessInfo>::iterator itV1;
    std::list<ProcessInfo>::iterator itV2=info2.begin();
    for(itV1=info1.begin();itV1!=info1.end();itV1++)
    {
        if((*itV1).ProcessID!=(*itV2).ProcessID)
            return false;
        itV2++;
    }
    return true;
}

bool HElcSignage::CheckProcess(std::list<QString> &info1, std::list<QString> &info2)
{
    if(info1.size()!=info2.size())
        return false;
    std::list<QString>::iterator itV1;
    std::list<QString>::iterator itV2=info2.begin();
    for(itV1=info1.begin();itV1!=info1.end();itV1++)
    {
        if((*itV1)!=(*itV2))
            return false;
        itV2++;
    }
    return true;
}

bool HElcSignage::CheckPartProcess(QString p1, QString p2)
{
    std::list<QString> info1,info2;
    std::map<QString,Part*>::iterator itP1=m_mapParts.find(p1);
    std::map<QString,Part*>::iterator itP2=m_mapParts.find(p2);
    if(itP1!=m_mapParts.end() && itP2!=m_mapParts.end())
    {
        GetPartProcess(p1,info1);
        GetPartProcess(p2,info2);
        return CheckProcess(info1,info2);
    }
    return false;
}

bool HElcSignage::CheckProductProcess(QString p1, QString p2)
{
    std::list<ProcessInfo> info1,info2;
    std::map<QString,Product*>::iterator itP1=m_mapProducts.find(p1);
    std::map<QString,Product*>::iterator itP2=m_mapProducts.find(p2);
    if(itP1!=m_mapProducts.end() && itP2!=m_mapProducts.end())
    {
        GetProductProcess(p1,info1);
        GetProductProcess(p2,info2);
        return CheckProcess(info1,info2);
    }
    return false;
}

QString HElcSignage::GetProcessName(QString id)
{
    std::map<QString,Process*>::iterator itMap=m_mapProcesses.find(id);
    if(itMap!=m_mapProcesses.end())
        return itMap->second->CName;
    return "";
}

bool HElcSignage::CopyProductProcess(QString target, QString source)
{
    std::map<QString,Product*>::iterator itTar=m_mapProducts.find(target);
    std::map<QString,Product*>::iterator itSrc=m_mapProducts.find(source);
    if(!(itTar!=m_mapProducts.end()))
        return false;
    if(!(itSrc!=m_mapProducts.end()))
        return false;

    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    QString strProcess,strSQL;
    int nOrder,nTarget=0,nSource=0,nCount=0;

    strSQL=QString("Select Count(*) from IntaiWeb_processlink Where ProductID='%1' and PartID='%1'").arg(target);
    if(!pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) || !pRS->GetValue(L"Count(*)",nTarget))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    strSQL=QString("Select Count(*) from IntaiWeb_processlink Where ProductID='%1' and PartID='%1'").arg(source);
    if(!pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) || !pRS->GetValue(L"Count(*)",nSource))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    if(nSource>nTarget)
    {
        strSQL=QString("Delete from IntaiWeb_processlink Where ProductID='%1' and PartID='%1' and theOrder>=%2").arg(source).arg(nTarget);
        pDB->ExecuteSQL(strSQL);
    }


    strSQL=QString("Select * from IntaiWeb_processlink Where ProductID='%1' and PartID='%1' order by theOrder").arg(target);
    if(!pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    nOrder=0;
    std::vector<QString> vProcesses;
    while(!pRS->isEOF())
    {
        strProcess="";
        pRS->GetValue(L"ProcessID",strProcess);
        if(strProcess.size()>0)
            vProcesses.push_back(strProcess);
        pRS->MoveNext();
    }

    int index=0;
    for(size_t i=0;i<vProcesses.size();i++)
    {
        strSQL=QString("Select Count(*) from IntaiWeb_processlink Where ProductID='%1' and PartID='%1' and theOrder=%2").arg(source).arg(i);
        if(pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) && pRS->GetValue(L"Count(*)",nCount))
        {
            if(nCount<=0)
                strSQL=QString("insert into IntaiWeb_processlink(ProcessID,ProductID,PartID,theOrder) Values('%1','%2','%2',%3)").arg(
                            vProcesses[i]).arg(
                            source).arg(
                            index);
            else
                strSQL=QString("update IntaiWeb_processlink set ProcessID='%1',theOrder=%3 Where ProductID='%2' and PartID='%2'").arg(
                            vProcesses[i]).arg(
                            source).arg(
                            index);
            if(pDB->ExecuteSQL(strSQL))
                index++;
        }
    }

    delete pRS;
    pDB->Close();
    return true;
}

bool HElcSignage::CopyPartProcess(QString target, QString source)
{
    std::map<QString,Part*>::iterator itTar=m_mapParts.find(target);
    std::map<QString,Part*>::iterator itSrc=m_mapParts.find(source);
    if(!(itTar!=m_mapParts.end()))
        return false;
    if(!(itSrc!=m_mapParts.end()))
        return false;

    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    QString strProcess,strPart,strSQL;
    int nOrder,nTarget=0,nSource=0,nCount=0;
    strSQL=QString("Select Count(*) from IntaiWeb_IntaiWeb_processlink Where ProductID='%1'").arg(target);
    if(!pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) || !pRS->GetValue(L"Count(*)",nTarget))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    strSQL=QString("Select Count(*) from IntaiWeb_processlink Where ProductID='%1'").arg(source);
    if(!pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) || !pRS->GetValue(L"Count(*)",nSource))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    if(nSource>nTarget)
    {
        strSQL=QString("Delete from IntaiWeb_processlink Where ProductID='%1' and theOrder>=%2").arg(source).arg(nTarget);
        pDB->ExecuteSQL(strSQL);
    }


    strSQL=QString("Select * from IntaiWeb_processlink Where IntaiWeb_productID='%1' order by theOrder").arg(target);
    if(!pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
         delete pRS;
        pDB->Close();
        return false;
    }
    nOrder=0;
    std::vector<QString> vProcesses;
    std::vector<QString> vParts;
    while(!pRS->isEOF())
    {
        strProcess="";
        pRS->GetValue(L"ProcessID",strProcess);
        strPart="";
        pRS->GetValue(L"PartID",strPart);
        if(strProcess.size()>0)
        {
            vProcesses.push_back(strProcess);
            //if(strPart.size()>0)
            //    vParts.push_back(strPart);
            //else
                vParts.push_back(source);
        }
        pRS->MoveNext();
    }
    if(vProcesses.size()!=vParts.size())
    {
        delete pRS;
        pDB->Close();
        return false;

    }
    int index=0;
    for(size_t i=0;i<vProcesses.size();i++)
    {
        strSQL=QString("Select Count(*) from IntaiWeb_processlink Where ProductID='%1' and PartID='%2' and theOrder=%3").arg(
                    source).arg(
                    vParts[i]).arg(
                    i);
        if(pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) && pRS->GetValue(L"Count(*)",nCount))
        {
            if(nCount<=0)
                strSQL=QString("insert into IntaiWeb_processlink(ProcessID,ProductID,PartID,theOrder) Values('%1','%2','%3',%4)").arg(
                            vProcesses[i]).arg(
                            source).arg(
                            vParts[i]).arg(
                            index);
            else
                strSQL=QString("update IntaiWeb_processlink set ProcessID='%1',theOrder=%4 Where ProductID='%2' and PartID='%3'").arg(
                            vProcesses[i]).arg(
                            source).arg(
                            vParts[i]).arg(
                            index);
            if(pDB->ExecuteSQL(strSQL))
                index++;
        }
    }

    delete pRS;
    pDB->Close();
    return true;
}

/*
bool HElcSignage::NewProcess(std::map<QString, QString> &datas)
{
    std::map<QString, QString>::iterator itMap,itMap2;
    for(itMap=datas.begin();itMap!=datas.end();itMap++)
    {
        itMap2=m_Processes.find(itMap->first);
        if(itMap2!=m_Processes.end())
            itMap2->second=itMap->second;
        else
            m_Processes.insert(std::make_pair(itMap->first,itMap->second));
    }
    return true;
}


bool HElcSignage::DeleteProcess(QString id)
{
    if(!pDB->Open())
        return false;

    QString strSQL,strID,strName;
    bool ret=false;

    strSQL=QString("delete from MyProcess Where ID=PartNo and ID='%1'").arg(id);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
        ret=pDB->ExecuteSQL(strSQL);

    pDB->Close();

    if(ret)
    {
         std::map<QString, QString>::iterator itMap=m_Processes.find(id);
         if(itMap!=m_Processes.end())
             m_Processes.erase(itMap);
    }
    return ret;
}

*/
bool HElcSignage::CopyParts(QString type,std::map<QString, QString> &datas)
{
    std::map<QString, Part*>::iterator itMap;
    datas.clear();
    for(itMap=m_mapParts.begin();itMap!=m_mapParts.end();itMap++)
    {
        if(type.size()<=0 || itMap->second->TypeID==type)
            datas.insert(std::make_pair(itMap->first,itMap->second->CName));
    }
   return true;
}

// 取得出貨日期
void HElcSignage::GetDatesFromShipments(QString strType,QStringList &list)
{
    QString strDateNow,strValue,strSQL;
    if(strType.size()<=0)
        return;

    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return;
    }

    std::map<QString,QString>::iterator itMap;
    std::map<QString,QString> datas;    // productID,productName
    if(!CopyProducts(strType,datas))
    {
        pDB->Close();
        return;
    }

    // 出貨日期
    strDateNow=QDate::currentDate().toString("yyyy/MM/dd");
    strSQL=QString("Delete FROM IntaiWeb_Shipment Where OutDate < '%1'").arg(strDateNow);
    pDB->ExecuteSQL(strSQL);

    std::map<QString,QString>::iterator itDate;
    std::map<QString,QString> mapDates;
    for(itMap=datas.begin();itMap!=datas.end();itMap++)
    {
        strSQL=QString("SELECT DISTINCT OutDate FROM IntaiWeb_Shipment Where ProductID='%1' order by OutDate ").arg(itMap->first);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            while(!pRS->isEOF())
            {
                pRS->GetValue(L"OutDate",strValue);
                if(strValue.size()>0)
                {
                    itDate=mapDates.find(strValue);
                    if(!(itDate!=mapDates.end()))
                        mapDates.insert(std::make_pair(strValue,""));   // date+""
                }
                pRS->MoveNext();
            }
        }
    }


    for(itDate=mapDates.begin();itDate!=mapDates.end();itDate++)
    {
        list.push_back(itDate->first);
    }

    delete pRS;
    pDB->Close();
}

bool HElcSignage::AddNewShipment(QString strType,QDate &newDate)
{
    QDate nowDate=QDate::currentDate();
    if(newDate.isNull() || newDate < nowDate)
        return false;

    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    int nCount;
    QString strSQL,strAddDate=newDate.toString("yyyy/MM/dd");
    bool ret=true;

    // 刪除以前的資料
    strSQL=QString("delete from IntaiWeb_Shipment Where OutDate<'%1'").arg(nowDate.toString("yyyy/MM/dd"));
    pDB->ExecuteSQL(strSQL);


    std::vector<Product*>   vProducts;
    std::map<QString,Product*>::iterator itMap;
    Product* pProduct;
    for(itMap=m_mapProducts.begin();itMap!=m_mapProducts.end();itMap++)
    {
        pProduct=itMap->second;
        if(pProduct->TypeID==strType)
            vProducts.push_back(pProduct);
    }

    for(size_t i=0;i<vProducts.size();i++)
    {
        // 資料己存在
        pProduct=vProducts[i];
        strSQL=QString("select Count(*) from IntaiWeb_Shipment Where OutDate='%1' and ProductID='%2'").arg(strAddDate).arg(pProduct->ProductID);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            nCount=-1;
            if(pRS->GetValue(L"Count(*)",nCount) && nCount<=0)
            {
                strSQL=QString("insert into IntaiWeb_Shipment(ProductID,OutDate,OutCount) Values('%1','%2',%3)").arg(
                            pProduct->ProductID).arg(strAddDate).arg(0);
                if(!pDB->ExecuteSQL(strSQL))
                    ret=false;
            }
        }
    }

    delete pRS;
    return ret;
}

bool HElcSignage::DelShipment(QDate &delDate)
{
    HDataBase* pDB;
    (m_pMySQLDB==nullptr)?(pDB=&m_LiteDB):(pDB=m_pMySQLDB);
    if(!pDB->Open())
        return false;

    QString strSQL,strDelDate=delDate.toString("yyyy/MM/dd");
    QString strNowDate=QDate::currentDate().toString("yyyy/MM/dd");

    // 刪除以前的資料
    strSQL=QString("delete from IntaiWeb_Shipment Where OutDate<'%1'").arg(strNowDate);
    pDB->ExecuteSQL(strSQL);

    // 刪除資料
    strSQL=QString("delete from IntaiWeb_Shipment Where OutDate='%1'").arg(strDelDate);
    bool ret=pDB->ExecuteSQL(strSQL);


    pDB->Close();
    return ret;
}

void HElcSignage::GetShipMents(QStringList &Products, std::map<QString, Shipment> &outShipments)
{
    QString strStockID="3001";  // 成本倉
    QString strDate,strValue,strProduct,strSQL;

    std::map<QString,std::map<QString,Shipment>>    myDatas;    // ProductID,Date
    std::map<QString,std::map<QString,Shipment>>::iterator itP;
    std::map<QString,Shipment>::iterator itDate;
    std::map<QString,Shipment>* pMapValues=nullptr;

    outShipments.clear();
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return;
    }

    // 刪除以前的資料
    strSQL=QString("delete from IntaiWeb_Shipment Where OutData<'%1'").arg(QDate::currentDate().toString("yyyy/MM/dd"));
    pDB->ExecuteSQL(strSQL);

    for(int j=0;j<Products.size();j++)
    {
        std::map<QString,Shipment> mapData;
        myDatas.insert(std::make_pair(Products[j],mapData));
    }

    for(itP=myDatas.begin();itP!=myDatas.end();itP++)
    {
        pMapValues=&itP->second;
        strProduct=itP->first;
        strSQL=QString("Select * from IntaiWeb_Shipment Where ProductID='%1' order by OutDate").arg(strProduct);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            while(!pRS->isEOF())
            {
                strDate="";
                pRS->GetValue(L"OutDate",strDate);
                Shipment ship;
                pRS->GetValue(L"OutCount",ship.OutCount);

                if(strDate.size()>0)
                {
                    ship.date=QDate::fromString(strDate,"yyyy/MM/dd");
                    ship.StockCount=0;
                    pMapValues->insert(std::make_pair(strDate,ship));
                }

                pRS->MoveNext();
            }
        }

    }



    delete pRS;
    pDB->Close();

    // 載入庫存
    std::map<QString,Shipment>::iterator itP2;
    Shipment* pOldShip;
    double dblValue;
    for(itP=myDatas.begin();itP!=myDatas.end();itP++)
    {
        pOldShip=nullptr;
        pMapValues=&itP->second;
        if(GetStockCount(m_TypeSelect,itP->first,strStockID,dblValue))
        {
            itP2=pMapValues->begin();
            while(itP2!=pMapValues->end())
            {
                if(pOldShip==nullptr)
                    itP2->second.StockCount=dblValue;
                else
                    itP2->second.StockCount=pOldShip->StockCount-pOldShip->OutCount;
                pOldShip=&itP2->second;

                strValue=QString("%1:%2").arg(itP2->first,itP->first);
                outShipments.insert(std::make_pair(strValue,itP2->second));

                itP2++;
            }
        }
    }




}

void HElcSignage::GetShipMents(std::map<QString,QString>& ProductInfos, std::map<QString, Shipment> &outShipments)
{
    QString strStockID="3001";  // 成本倉
    QString strDate,strValue,strProduct,strSQL;

    std::map<QString,QString>::iterator itInfo;
    std::map<QString,std::map<QString,Shipment>>    myDatas;    // ProductID,Date
    std::map<QString,std::map<QString,Shipment>>::iterator itP;
    std::map<QString,Shipment>::iterator itDate;
    std::map<QString,Shipment>* pMapValues=nullptr;

    outShipments.clear();
    if(ProductInfos.size()<=0)
        return;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return;
    }

    // 刪除以前的資料
    strSQL=QString("delete from IntaiWeb_Shipment Where OutData<'%1'").arg(QDate::currentDate().toString("yyyy/MM/dd"));
    pDB->ExecuteSQL(strSQL);

    for(itInfo=ProductInfos.begin();itInfo!=ProductInfos.end();itInfo++)
    {
        std::map<QString,Shipment> mapData;
        myDatas.insert(std::make_pair(itInfo->first,mapData));
    }

    for(itP=myDatas.begin();itP!=myDatas.end();itP++)
    {
        pMapValues=&itP->second;
        strProduct=itP->first;
        strSQL=QString("Select * from IntaiWeb_Shipment Where ProductID='%1' order by OutDate").arg(strProduct);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            while(!pRS->isEOF())
            {
                strDate="";
                pRS->GetValue(L"OutDate",strDate);
                Shipment ship;
                pRS->GetValue(L"OutCount",ship.OutCount);

                if(strDate.size()>0)
                {
                    ship.date=QDate::fromString(strDate,"yyyy/MM/dd");
                    ship.StockCount=0;
                    pMapValues->insert(std::make_pair(strDate,ship));
                }

                pRS->MoveNext();
            }
        }

    }



    delete pRS;
    pDB->Close();

    // 載入庫存
    std::map<QString,Shipment>::iterator itP2;
    Shipment* pOldShip;
    double dblValue;
    for(itP=myDatas.begin();itP!=myDatas.end();itP++)
    {
        pOldShip=nullptr;
        pMapValues=&itP->second;
        itInfo=ProductInfos.find(itP->first);
        if(itInfo!=ProductInfos.end())
        {
            if(!GetStockCount(itInfo->second,itP->first,strStockID,dblValue))
                dblValue=0;
            itP2=pMapValues->begin();
            while(itP2!=pMapValues->end())
            {
                if(pOldShip==nullptr)
                    itP2->second.StockCount=dblValue;
                else
                    itP2->second.StockCount=pOldShip->StockCount-pOldShip->OutCount;
                pOldShip=&itP2->second;

                strValue=QString("%1:%2").arg(itP2->first,itP->first);
                outShipments.insert(std::make_pair(strValue,itP2->second));

                itP2++;
            }
        }
    }




}

void HElcSignage::GetShipMentsFromDB(QString type,QString strDate,std::map<QString,ShipTable>& outputs)
{
    if(type.size()<=0 || strDate.size()<=0 || outputs.size()!=0)
        return;

    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return;
    }

    double dblValue;
    QString strProduct;
    QString strSQL=QString("SELECT * FROM IntaiWeb_shiptable Where TypeID='%1' and ShipDate='%2'").arg(type).arg(strDate);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strProduct.clear();
            pRS->GetValue(L"ProductID",strProduct);
            if(strProduct.size()>0)
            {
                ShipTable st;
                st.ProductID=strProduct;
                st.TypeID=type;
                st.ship.date=QDate::fromString(strDate,"yyyy/MM/dd");
                pRS->GetValue(L"TargetCount",dblValue);
                st.ship.OutCount=static_cast<int>(dblValue);
                pRS->GetValue(L"WipCount",dblValue);
                st.ship.StockCount=static_cast<int>(-1*dblValue);

                if(st.ship.OutCount<=0)
                {
                    st.rate=-999;
                    st.diff=-999;
                }
                else
                {
                    st.rate=static_cast<double>(st.ship.StockCount)/st.ship.OutCount;
                    st.diff=st.ship.OutCount-st.ship.StockCount;
                }

                outputs.insert(std::make_pair(strProduct,st));
            }

           pRS->MoveNext();
        }
    }

    delete pRS;
    pDB->Close();
}

void HElcSignage::GetShipMentsFromDB(QString type,std::vector<ShipTable> &outputs)
{
    if(outputs.size()!=0)
        return;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return;
    }

    int nValue;
    QString strProduct,strDate;
    QString strSQL=QString("SELECT * FROM IntaiWeb_Shipment as S,IntaiWeb_product as P Where S.ProductID=P.ProductID and P.TypeID='%1'").arg(type);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strProduct.clear();
            pRS->GetValue(L"ProductID",strProduct);
            if(strProduct.size()>0)
            {
                ShipTable st;
                st.ProductID=strProduct;
                pRS->GetValue(L"OutDate",strDate);
                st.ship.date=QDate::fromString(strDate,"yyyy/MM/dd");
                pRS->GetValue(L"OutCount",nValue);
                st.ship.OutCount=nValue;
                outputs.push_back(st);
            }

            pRS->MoveNext();
        }
    }

    if(outputs.size()<=0)
    {
        strSQL=QString("SELECT * FROM IntaiWeb_product Where TypeID='%1'").arg(type);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            while(!pRS->isEOF())
            {
                strProduct.clear();
                pRS->GetValue(L"ProductID",strProduct);
                if(strProduct.size()>0)
                {
                    ShipTable st;
                    st.ProductID=strProduct;
                    st.TypeID=type;
                    st.diff=0;
                    st.rate=0;
                    st.ship.StockCount=0;
                    st.ship.date=QDate::currentDate();
                    st.ship.OutCount=0;
                    outputs.push_back(st);
                }

                pRS->MoveNext();
            }
        }
    }

    delete pRS;
    pDB->Close();
}


void HElcSignage::GetShipMentsFromDB(std::vector<ShipTable> &outputs)
{
    if(outputs.size()!=0)
        return;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return;
    }
    int nValue;
    QString strProduct,strDate;
    QString strSQL;
    std::map<QString,std::vector<ShipTable>> mapOKDatas,mapNewDatas;
    std::map<QString,std::vector<ShipTable>>::iterator itPro;


    strSQL=QString("SELECT * FROM IntaiWeb_Shipment as S, IntaiWeb_product as P Where S.ProductID=P.ProductID");
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strProduct.clear();
           pRS->GetValue(L"ProductID",strProduct);
            if(strProduct.size()>0)
            {
                itPro=mapOKDatas.find(strProduct);
                if(itPro!=mapOKDatas.end())
                {
                   std::vector<ShipTable> *pTables=&itPro->second;
                   ShipTable st;
                   pRS->GetValue(L"TypeID",st.TypeID);
                   st.ProductID=strProduct;
                  pRS->GetValue(L"OutDate",strDate);
                   st.ship.date=QDate::fromString(strDate,"yyyy/MM/dd");
                   if(!st.ship.date.isValid())
                       st.ship.date=QDate::currentDate();
                   pRS->GetValue(L"OutCount",nValue);
                   st.ship.OutCount=nValue;
                   pTables->push_back(st);
                }
                else
                {
                     std::vector<ShipTable> tables;
                    ShipTable st;
                   pRS->GetValue(L"TypeID",st.TypeID);
                    st.ProductID=strProduct;
                    pRS->GetValue(L"OutDate",strDate);
                    st.ship.date=QDate::fromString(strDate,"yyyy/MM/dd");
                    if(!st.ship.date.isValid())
                        st.ship.date=QDate::currentDate();
                    pRS->GetValue(L"OutCount",nValue);
                    st.ship.OutCount=nValue;
                    tables.push_back(st);
                    mapOKDatas.insert(std::make_pair(strProduct,tables));
                }
            }

            pRS->MoveNext();
        }
    }


   strSQL=QString("SELECT * FROM IntaiWeb_product order by TypeID");
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            strProduct.clear();
           pRS->GetValue(L"ProductID",strProduct);
            if(strProduct.size()>0)
            {
                itPro=mapOKDatas.find(strProduct);
                if(itPro!=mapOKDatas.end())
                {
                    pRS->MoveNext();
                    continue;
                }
                itPro=mapNewDatas.find(strProduct);
                if(itPro!=mapNewDatas.end())
                {
                   std::vector<ShipTable> *pTables=&itPro->second;
                   ShipTable st;
                   st.ProductID=strProduct;
                   pRS->GetValue(L"TypeID",st.TypeID);
                   pRS->GetValue(L"OutDate",strDate);
                   st.ship.date=QDate::fromString(strDate,"yyyy/MM/dd");
                   if(!st.ship.date.isValid())
                       st.ship.date=QDate::currentDate();
                   st.ship.OutCount=0;
                   pTables->push_back(st);
                }
                else
                {
                     std::vector<ShipTable> tables;
                    ShipTable st;
                    st.ProductID=strProduct;
                   pRS->GetValue(L"TypeID",st.TypeID);
                    pRS->GetValue(L"OutDate",strDate);
                    st.ship.date=QDate::fromString(strDate,"yyyy/MM/dd");
                    if(!st.ship.date.isValid())
                        st.ship.date=QDate::currentDate();
                    st.ship.OutCount=0;
                    tables.push_back(st);
                    mapNewDatas.insert(std::make_pair(strProduct,tables));
                }
            }

            pRS->MoveNext();
        }
    }


    delete pRS;
    pDB->Close();

    for(itPro=mapOKDatas.begin();itPro!=mapOKDatas.end();itPro++)
    {
        std::vector<ShipTable>::iterator itTable;
        for(itTable=itPro->second.begin();itTable!=itPro->second.end();itTable++)
            outputs.push_back(*itTable);
    }
    for(itPro=mapNewDatas.begin();itPro!=mapNewDatas.end();itPro++)
    {
        std::vector<ShipTable>::iterator itTable;
        for(itTable=itPro->second.begin();itTable!=itPro->second.end();itTable++)
            outputs.push_back(*itTable);
    }


}

void HElcSignage::GetBacksFromDB(QString type, std::vector<QString> &Products, std::map<int, ProcessTable> &outputs)
{
    std::map<QString,Process*>::iterator itP1;
    std::map<QString,Product*>::iterator itP2;
    std::map<QString,Part*>::iterator itP3;
    if(type.size()<=0 || Products.size()<=0 || outputs.size()!=0)
        return;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return;
    }

    QString strProduct;
    QString strSQL;
    int nCount=0,nIndex=-1;
    std::vector<QString>::iterator itV;
    for(itV=Products.begin();itV!=Products.end();itV++)
    {
        strSQL=QString("SELECT * FROM IntaiWeb_runningtables Where TypeID='%1' and ProductID='%2' order by ProcessIndex").arg(type).arg(*itV);
        if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
        {
            while(!pRS->isEOF())
            {
                nIndex=-1;
               pRS->GetValue(L"ProcessIndex",nIndex);
                if(nIndex>=0)
                {
                    ProcessTable table;
                    table.type=type;
                    table.info.ProductID=(*itV);
                    table.info.order=nIndex;
                    pRS->GetValue(L"ProcessID",table.info.ProcessID);
                    pRS->GetValue(L"WipCount",table.info.Source);
                    pRS->GetValue(L"TargetCount",table.info.Target);
                    pRS->GetValue(L"SumCount",table.info.sum);
                    itP1=m_mapProcesses.find(table.info.ProcessID);
                    if(itP1!=m_mapProcesses.end())
                        table.info.ProcessID=itP1->second->CName;
                    itP2=m_mapProducts.find(table.info.ProductID);
                    if(itP2!=m_mapProducts.end())
                        table.info.ProductID=itP2->second->CName;
                    else
                    {
                        itP3=m_mapParts.find(table.info.ProductID);
                        if(itP3!=m_mapParts.end())
                            table.info.ProductID=itP3->second->CName;
                    }
                    //table.info.DayTarget
                    outputs.insert(std::make_pair(nCount,table));
                }
                pRS->MoveNext();
            }
        }
        nCount++;
    }
    delete pRS;
    pDB->Close();
}

void HElcSignage::GetRunningsFromDB(QString type, QString strProduct,QString &ProductName,std::map<int, ProcessTable> &outputs)
{
    std::map<QString,Process*>::iterator itP1;
    std::map<QString,Product*>::iterator itP2;
    std::map<QString,Part*>::iterator itP3;
    if(type.size()<=0 || strProduct.size()<=0 || outputs.size()!=0)
        return;

    itP2=m_mapProducts.find(strProduct);
    if(itP2!=m_mapProducts.end())
        ProductName=itP2->second->CName;
    else
    {
        itP3=m_mapParts.find(strProduct);
        if(itP3!=m_mapParts.end())
            ProductName=itP3->second->CName;
    }
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return;
    }

    QString strSQL;
    int nCount=0,nIndex=-1;


    strSQL=QString("SELECT * FROM IntaiWeb_runningtables Where TypeID='%1' and ProductID='%2' order by ProcessIndex").arg(type).arg(strProduct);
    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!pRS->isEOF())
        {
            nIndex=-1;
            pRS->GetValue(L"ProcessIndex",nIndex);
            if(nIndex>=0)
            {
                ProcessTable table;
                table.type=type;
                table.info.ProductID=strProduct;
                table.info.order=nIndex;
                pRS->GetValue(L"ProcessID",table.info.ProcessID);
                pRS->GetValue(L"WipCount",table.info.Source);
                pRS->GetValue(L"TargetCount",table.info.Target);
                pRS->GetValue(L"SumCount",table.info.sum);
                itP1=m_mapProcesses.find(table.info.ProcessID);
                if(itP1!=m_mapProcesses.end())
                    table.info.ProcessName=itP1->second->CName;
                table.ProductName=ProductName;

                //table.info.DayTarget
                outputs.insert(std::make_pair(outputs.size(),table));
            }
            pRS->MoveNext();
        }
    }

    delete pRS;
    pDB->Close();
}

bool HElcSignage::GetStockCount(QString type,QString productId, QString position, double &out)
{
    double dblValue=-1;
    QString strValue,strDate,strProduct;
    /*
    QString strSQL=QString("SELECT stockcount FROM IntaiWeb_erpstock Where ProductID='%1' and StockID='%2' and Type='%3'").arg(
                productId).arg(
                position).arg(
                type);//m_TypeSelect);
    */
    QString strSQL=QString("SELECT StockCount FROM IntaiWeb_ERPStock Where ProductID='%1' and StockID='%2'").arg(
                productId).arg(
                position);//m_TypeSelect);
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))// && !pRS->isEOF())
    {

        if(pRS->GetValue(L"StockCount",dblValue) && dblValue>=0)
        {
            out=dblValue;
            delete pRS;
            pDB->Close();
            return true;
        }

    }
    delete pRS;
    pDB->Close();
    return false;
}

bool HElcSignage::SaveShipMents(std::map<QString, Shipment> &datas)
{
    std::map<QString, Shipment>::iterator itMap;
    Shipment* pShip;
    bool ret=true;
    QString strValue,strPID,strSQL;
    int sel;
    if(datas.size()<=0)
        return false;

    HDataBase* pDB;
    (m_pMySQLDB==nullptr)?(pDB=&m_LiteDB):(pDB=m_pMySQLDB);
    if(!pDB->Open())
        return false;

    std::map<QString,QString> productInfos;
    for(itMap=datas.begin();itMap!=datas.end();itMap++)
    {
        strValue=itMap->first;
        pShip=&itMap->second;
        sel=strValue.indexOf(":");
        if(sel>0)
        {
            strPID=strValue.right(strValue.size()-sel-1);
            strSQL=QString("update IntaiWeb_Shipment Set OutCount=%1 Where ProductID='%2' and OutDate='%3'").arg(
                        pShip->OutCount).arg(
                        strPID).arg(
                        pShip->date.toString("yyyy/MM/dd"));
            if(!pDB->ExecuteSQL(strSQL))
                ret=false;


            productInfos.insert(std::make_pair(strPID,m_TypeSelect));
        }
    }

    pDB->Close();


    if(ret)
    {
        this->ExportShip(productInfos);
    }
    return ret;
}

bool HElcSignage::ImportERPFile(QString type,QString strFile)
{
    int TargetColumn=9;
    std::vector<QString> names;
    std::map<int,QString>::iterator itMap;
    std::vector<QStringList> vDatas;
    if(type.size()<=0)
        return false;

    if(!QFile::exists(strFile))
        return false;

    QXlsx::Document xlsx(strFile);
    int nRow=xlsx.dimension().rowCount();
    int nCol=xlsx.dimension().columnCount();
    if(nCol!=TargetColumn)
        return false;

    QString cellName,cellValue;
    std::vector<QString> vData;
    for(int col=1; col<=nCol; ++col)
    {
        QVariant v = xlsx.read(1, col);
        if(v.isValid())
            names.push_back(v.toString());
    }
    if(names.size()!=TargetColumn)
        return false;
/*
    if(names[0] != L"工廠"
            || names[1] != L"儲存位置的說明"
            || names[2] != L"儲存地點"
            || names[3] != L"物料說明"
            || names[4] != L"物料"
            || names[5] != L"未限制"
            || names[6] != L"值未限制"
            || names[7] != L"庫存移轉中的值"
            || names[8] != L"製造日期"
            )
    {
        return false;
    }
*/

    std::map<QString,ERPStock> mapERPStocks;
    std::map<QString,ERPStock>::iterator itERP;
    QString strKey;
    //std::vector<ERPStock> vERPStocks;
    for(int row=2;row<=nRow;++row)
    {
        QStringList values;
        QVariant v1 = xlsx.read(row, 1);
        QVariant v3 = xlsx.read(row, 3);
        QVariant v4 = xlsx.read(row, 5);
        QVariant v5 = xlsx.read(row, 6);
        QVariant v9 = xlsx.read(row, 9);
        if(v1.isValid())
        {
            if(v1.toString()=="1002")   // 醫療
            {
                ERPStock stock;
                stock.position=v3.toString();
                if(stock.position.size()>0)
                {
                    stock.productId=v4.toString();
                    if( stock.productId.size()>0)
                    {
                        stock.value=v5.toDouble();
                        stock.makeDate=v9.toDate();
                        strKey=QString("%1:%2").arg(stock.productId).arg(stock.position);
                        itERP=mapERPStocks.find(strKey);
                        if(itERP!=mapERPStocks.end())
                            itERP->second.value+=stock.value;
                        else
                            mapERPStocks.insert(std::make_pair(strKey,stock));
                    }
                }
            }
        }
    }

    bool ret=true;
    int nCount=0;
    QString strValue,strSQL,strDate;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    strSQL=QString("Delete from IntaiWeb_ERPStock Where Type='%1'").arg(type);
   pDB->ExecuteSQL(strSQL);


    for(itERP=mapERPStocks.begin();itERP!=mapERPStocks.end();itERP++)
    {
        strSQL=QString("select Count(*) from IntaiWeb_ERPStock Where ProductID='%1' and StockID='%2' and Type='%3'").arg(
                    itERP->second.productId).arg(
                    itERP->second.position).arg(
                    type);
        if(pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) && pRS->GetValue(L"Count(*)",nCount))
        {
            if(nCount<=0)
                strSQL=QString("insert into IntaiWeb_ERPStock(ProductID,StockID,StockCount,MakeDate,Type) Values('%1','%2',%3,'%4','%5')").arg(
                            itERP->second.productId).arg(
                            itERP->second.position).arg(
                            itERP->second.value).arg(
                            "").arg(
                            type);
            else
                strSQL=QString("Update IntaiWeb_ERPStock Set StockCount=StockCount+%3,MakeDate='%4' Where ProductID='%1' and StockID='%2' and Type='%5'").arg(
                            itERP->second.productId).arg(
                            itERP->second.position).arg(
                            itERP->second.value).arg(
                            "").arg(
                            type);

            if(!pDB->ExecuteSQL(strSQL))
                ret=false;
        }
    }

    delete pRS;
    pDB->Close();
    return ret;
}

bool HElcSignage::ImportSFCStatusFile(QString type,QString strFile)
{
    int TargetColumn=13;
    std::vector<QString> names;
    std::map<int,QString>::iterator itMap;
    std::vector<QStringList> vDatas;
    if(type.size()<=0)
        return false;

    if(!QFile::exists(strFile))
        return false;
    QXlsx::Document xlsx(strFile);
    int nRow=xlsx.dimension().rowCount();
    int nCol=xlsx.dimension().columnCount();
    if(nCol!=TargetColumn)
        return false;

    QString cellName;
    QString cellValue;
    std::vector<QString> vData;
    for(int col=1; col<=nCol; ++col)
    {
        QVariant v = xlsx.read(1, col);
        if(v.isValid())
            names.push_back(v.toString());
    }
    if(names.size()!=TargetColumn)
        return false;

    QString strKey;
    std::map<QString,SFCData>::iterator itData;
    std::map<QString,SFCData> mapSFCDatas;
    //std::vector<SFCData> vSFCDatas;
    for(int row=2;row<=nRow;++row)
    {
        QStringList values;
        QVariant v1 = xlsx.read(row, 1);    //產品編號
        QVariant v8 = xlsx.read(row, 8);    //工序編號
        QVariant v11 = xlsx.read(row, 11);  //目前狀態
        QVariant v12 = xlsx.read(row, 12);  //目前數量

        QString strUsed="啟用中\n";
        SFCData data;
        data.productId=v1.toString();
        if(data.productId.size()>0)
        {
            data.ProcessID=v8.toString();
            if(data.ProcessID.size()>0)
            {
                cellValue=v11.toString();
                if(cellValue == "啟用中\n" || cellValue=="佇列中\n" || cellValue=="保留\n")
                {
                    data.count=v12.toDouble();
                    strKey=QString("%1:%2").arg(data.productId).arg(data.ProcessID);
                    itData=mapSFCDatas.find(strKey);
                    if(itData!=mapSFCDatas.end())
                        itData->second.count+=data.count;
                    else
                        mapSFCDatas.insert(std::make_pair(strKey,data));
                }
            }
        }

    }

    bool ret=true;
    QString strValue,strSQL;
    HDataBase* pDB;
    (m_pMySQLDB==nullptr)?(pDB=&m_LiteDB):(pDB=m_pMySQLDB);
    if(!pDB->Open())
        return false;
    strSQL=QString("delete from IntaiWeb_SFCStatus Where Type='%1'").arg(type);
    pDB->ExecuteSQL(strSQL);

    for(itData=mapSFCDatas.begin();itData!=mapSFCDatas.end();itData++)
    {
        strSQL=QString("insert into IntaiWeb_SFCStatus(ProductID,ProcessID,Count,Type) Values('%1','%2',%3,'%4')").arg(
                    itData->second.productId).arg(
                    itData->second.ProcessID).arg(
                    itData->second.count).arg(
                    type);
        if(!pDB->ExecuteSQL(strSQL))
            ret=false;
    }

    pDB->Close();
    return ret;

}

bool HElcSignage::ImportSFCHistoryFile(QString type,QString strFile)
{
    int TargetColumn=19;
    std::vector<QString> names;
    std::map<int,QString>::iterator itMap;
    std::vector<QStringList> vDatas;
    if(type.size()<=0)
        return false;
    if(!QFile::exists(strFile))
        return false;
    QXlsx::Document xlsx(strFile);
    int nRow=xlsx.dimension().rowCount();
    int nCol=xlsx.dimension().columnCount();
    if(nCol!=TargetColumn)
        return false;

    QString cellName;
    std::wstring cellValue;
    std::vector<QString> vData;
    for(int col=1; col<=nCol; ++col)
    {
        QVariant v = xlsx.read(1, col);
        if(v.isValid())
            names.push_back(v.toString());
    }
    if(names.size()!=TargetColumn)
        return false;

    std::vector<SFCData> vSFCDatas;
    for(int row=2;row<=nRow;++row)
    {
        QStringList values;
        QVariant v1 = xlsx.read(row, 1);    //產品編號
        QVariant v7 = xlsx.read(row, 7);    //工序編號
        QVariant v13 = xlsx.read(row, 13);  //目前數量

        SFCData data;
        data.productId=v1.toString();
        if(data.productId.size()>0)
        {
            data.ProcessID=v7.toString();
            if(data.ProcessID.size()>0)
            {
                data.count=v13.toDouble();
                vSFCDatas.push_back(data);

            }
        }

    }

    bool ret=true;
    QString strValue,strSQL;
    int nCount=0;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }
    strSQL=QString("delete from IntaiWeb_SFCHistory Where Type='%1'").arg(type);
   pDB->ExecuteSQL(strSQL);

    for(size_t i=0;i<vSFCDatas.size();i++)
    {
        strSQL=QString("select Count(*) from IntaiWeb_SFCHistory Where ProductID='%1' and ProcessID='%2' and Type='%3'").arg(
                    vSFCDatas[i].productId).arg(
                    vSFCDatas[i].ProcessID).arg(
                    type);
        if(pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) && pRS->GetValue(L"Count(*)",nCount))
        {
            if(nCount<=0)
                strSQL=QString("insert into IntaiWeb_SFCHistory(ProductID,ProcessID,Count,Type) Values('%1','%2',%3,'%4')").arg(
                            vSFCDatas[i].productId).arg(
                            vSFCDatas[i].ProcessID).arg(
                            vSFCDatas[i].count).arg(
                            type);
            else
                strSQL=QString("Update IntaiWeb_SFCHistory Set Count=Count+%3 Where ProductID='%1' and ProcessID='%2' and Type='%4'").arg(
                            vSFCDatas[i].productId).arg(
                            vSFCDatas[i].ProcessID).arg(
                            vSFCDatas[i].count).arg(
                            type);
            if(!pDB->ExecuteSQL(strSQL))
                ret=false;
        }
    }

    delete pRS;
    pDB->Close();
    return ret;
}

bool HElcSignage::ImportTypeFile(QString strFile)
{
    size_t TargetColumn=2;
    std::map<QString,QString> names;
    std::map<QString,QString>::iterator itMap;
    if(!QFile::exists(strFile))
        return false;
    QXlsx::Document xlsx(strFile);
    int nRow=xlsx.dimension().rowCount();
    int nCol=xlsx.dimension().columnCount();
    if(static_cast<size_t>(nCol)<TargetColumn)
        return false;


    std::vector<SFCData> vSFCDatas;
    for(int row=2;row<=nRow;++row)
    {
        QString v1 = xlsx.read(row, 1).toString();    //型錄
        QString v2 = xlsx.read(row, 2).toString();    //產品名稱
        if(v1.size()>0 && v2.size()>0)
            names.insert(std::make_pair(v1,v2));


    }

    bool ret=true;
    QString strValue,strSQL;

    HDataBase* pDB;
    (m_pMySQLDB==nullptr)?(pDB=&m_LiteDB):(pDB=m_pMySQLDB);

    if(!pDB->Open())
        return false;

    for(itMap=names.begin();itMap!=names.end();itMap++)
    {
        strSQL=QString("insert into IntaiWeb_types(TypeID,TypeName) Values('%1','%2')").arg(
                    itMap->first).arg(
                    itMap->second);
        if(!pDB->ExecuteSQL(strSQL))
           ret=false;
    }

    pDB->Close();
    return ret;
}

bool HElcSignage::ImportShipmentDateFile(QString strFile)
{
    int TargetColumn=5;
    std::vector<QStringList>::iterator itD;
    std::vector<QStringList> vDatas;
    //if(m_TypeSelect.size()<=0)
    //    return false;
    if(strFile.size()<=0)
        return false;

    if(!QFile::exists(strFile))
        return false;
    QXlsx::Document xlsx(strFile);
    int nRow=xlsx.dimension().rowCount();
    int nCol=xlsx.dimension().columnCount();
    if(nCol!=TargetColumn)
        return false;

    for(int row=2;row<=(nRow+1);++row)
    {
        QStringList values;
        QDate vDate = xlsx.read(row, 1).toDate();    //出貨日期
        QVariant vType = xlsx.read(row, 2);    // Type
        QVariant vMes = xlsx.read(row, 3);    //MES客戶編號
        QVariant vCount = xlsx.read(row, 4);  //出貨數量
        QVariant vName = xlsx.read(row, 5);  //產品名稱

        QStringList lst;
        lst.push_back(vDate.toString("yyyy/MM/dd"));
        lst.push_back(vType.toString());
        lst.push_back(vMes.toString());
        lst.push_back(vCount.toString());
        lst.push_back(vName.toString());
        vDatas.push_back(lst);
    }

    bool ret=true;
    QString strValue,strSQL;
    HDataBase* pDB;
    (m_pMySQLDB==nullptr)?(pDB=&m_LiteDB):(pDB=m_pMySQLDB);

    if(!pDB->Open())
        return false;
    //strSQL=QString("delete from IntaiWeb_Shipment as S,IntaiWeb_product as P Where S.ProductID=P.ProductID and P.TypeID='%1'").arg(m_TypeSelect);
    //strSQL=QString("delete from IntaiWeb_Shipment Where ProductID In (select ProductID from IntaiWeb_product where TypeID='%1')").arg(m_TypeSelect);
    strSQL=QString("delete from IntaiWeb_Shipment");

    if(!pDB->ExecuteSQL(strSQL))
    {
        pDB->Close();
        return false;
    }

    QStringList* pList;
    for(itD=vDatas.begin();itD!=vDatas.end();itD++)
    {
        pList=&(*itD);
        if(pList->size()>=4)
        {
            if((*pList)[2].size()<=32)
            {
                strSQL=QString("insert into IntaiWeb_Shipment(ProductID,OutDate,OutCount) Values('%1','%2',%3)").arg(
                            (*pList)[2]).arg(
                            (*pList)[0]).arg(
                            (*pList)[3].toInt());
                if(!pDB->ExecuteSQL(strSQL))
                    ret=false;
            }
        }
    }

    pDB->Close();



    // save to export
    //QString strFileName=QString("%1/Export/DataExport.XLSX").arg(m_strAppPath);
    // bool copySuccess = QFile::copy(strFile,strFileName);

    //return copySuccess;

    return true;

}

bool HElcSignage::ImportShipmentDateFile()
{
    QString strFilePath=m_strAppPath + "/Update";
    SystemData *pSData=GetSystemData("UpdateFilePath");
    if(pSData!=nullptr)
        strFilePath=pSData->strValue;

    QDir directory(strFilePath);
    QString strType,strFile;
    int st,end;
    QStringList fileList = directory.entryList(QDir::Files);
    foreach (QString fileName, fileList)
    {
        /*
        st=fileName.indexOf('_');
        end=fileName.indexOf('.',st);
        if(end>st)
        {
            strType=fileName.mid(st+1,end-st-1);
            strFile=directory.path() + "/";
            strFile += fileName;
            if(strType.size()>0)
            {
                m_TypeSelect=strType;
                if(ImportShipmentDateFile(strFile))
                {
                    QFile file(strFile);
                    file.remove();
                    return true;
                }
            }
        }
        */
         st=fileName.indexOf(".XLSX");
         if(st<=0)
              st=fileName.indexOf(".xlsx");
         if(st>0)
         {
             strFile=directory.path() + "/";
             strFile += fileName;
             if(ImportShipmentDateFile(strFile))
             {
                 QFile file(strFile);
                 if(file.exists())
                 {
                     if(file.remove())
                         return true;
                     /*
                     if (file.setPermissions(QFile::ReadOther | QFile::WriteOther))
                         return file.remove();

                    QStringList args;
                    args << "rm" << "-f" << strFile;

                    QProcess process;
                    process.start("sudo", args);
                    process.write("1234\n"); // 替換為您的 sudo 密碼

                    process.start("bash", args);

                    if(process.waitForFinished())
                        return true;
                        */
                 }
             }
         }

    }
    return false;
}

bool HElcSignage::ExportRunnintTable(QString strFile, QString strType, QString strPartID,bool wip)
{
    QFileInfo info(strFile);
    if(info.isFile())
        return false;

    HRecordset* pRS;
    QString strValue,strSQL,strSelect;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }

    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    // 創建一個QXlsx::Document對象
    QXlsx::Document xlsx;

    // 在工作表中寫入數據
    QXlsx::Format format;
    format.setFontColor(Qt::red);

    QStringList myList;

    int nMax=0;
    QString strPID,strPName;
    strSelect="SELECT R.TypeID as 'type',R.ProcessIndex as 'PO',R.ProcessID as 'PID',C.CName as 'PName'"
            "FROM IntaiWeb_runningtables R"
            "JOIN IntaiWeb_product P ON R.ProductID = P.ProductID"
            "JOIN IntaiWeb_process C ON R.ProcessID = C.ProcessID"
            "where type= ";
    strSQL=QString("%1'%2'  GROUP BY PO order by PO Desc").arg(strSelect).arg(strType);
    if(!pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        delete pRS;
        pDB->Close();
        return false;
    }


    int nIndex=0;
    while(!pRS->isEOF())
    {
        pRS->GetValue(L"PID",strPID);
        pRS->GetValue(L"PName",strPName);

        xlsx.write(1,    nIndex+3,  nIndex+1);
        xlsx.write(2,    nIndex+3, strPID);
        xlsx.write(3,    nIndex+3, strPID);
        nIndex++;

        pRS->MoveNext();
    }

    strSelect="SELECT R.TypeID as 'type',R.ProductID AS 'PID', P.CName AS 'PName',R.ProcessIndex as 'index',R.ProcessID as 'PID2',C.CName as 'PName2',R.WipCount as 'count' "
            "FROM IntaiWeb_runningtables R "
            "JOIN IntaiWeb_product P ON R.ProductID = P.ProductID"
            "JOIN IntaiWeb_process C ON R.ProcessID = C.ProcessID"
            "where type=";
    strSQL=QString("%1'%2'").arg(strSelect).arg(strType);

    if(!pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    int nCount,nID=0;
    QString strProduct,strProcess,strProcessName;
    while(!pRS->isEOF())
    {
        pRS->GetValue(L"PID",strProcess);
        pRS->GetValue(L"PName",strPName);
        pRS->GetValue(L"index",nIndex);
        pRS->GetValue(L"PID2",strProcess);
        pRS->GetValue(L"PName2",strProcessName);
        pRS->GetValue(L"count",nCount);

        xlsx.write(1,    nIndex+3,  nIndex+1);
        xlsx.write(2,    nIndex+3, strPID);
        xlsx.write(3,    nIndex+3, strPID);


        pRS->MoveNext();
    }








    // 保存Excel檔案
    //xlsx.write(cell,nValue,format);
     //xlsx.write(cell,pItem->text());
    //xlsx.saveAs(fileName);


    delete pRS;
    pDB->Close();
    return false;
}

bool HElcSignage::SavePart(QString strType,QString strProduct,Part *pPart,int count)
{
    int nCount=0;
    Product* pProduct=nullptr;
    std::map<QString,Part*>::iterator itP2;
    std::map<QString,Product*>::iterator itP=m_mapProducts.find(strProduct);
    if(!(itP!=m_mapProducts.end()))
    {
        itP2=m_mapParts.find(strProduct);
        if(!(itP2!=m_mapParts.end()))
            return false;
        strProduct=itP2->second->PartID;
    }
    else
        pProduct=itP->second;

    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    QString strSQL=QString("select Count(*) from Part Where PartID='%1'").arg(pPart->PartID);
    if(!pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) || !pRS->GetValue(L"Count(*)",nCount))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    if(nCount<=0)
        strSQL=QString("insert into IntaiWeb_part(PartID,CName,Specification,Stock,TypeID) Values('%1','%2','%3',%4,'%5')").arg(
                    pPart->PartID).arg(
                    pPart->CName).arg(
                    pPart->Specification).arg(
                    pPart->Stock).arg(
                    pPart->TypeID);
    else
        strSQL=QString("update IntaiWeb_part set CName='%2',Specification='%3',Stock=%4,TypeID='%5' Where PartID='%1'").arg(
                    pPart->PartID).arg(
                    pPart->CName).arg(
                    pPart->Specification).arg(
                    pPart->Stock).arg(
                    pPart->TypeID);
    if(!pDB->ExecuteSQL(strSQL))
    {
        delete pRS;
        pDB->Close();
        return false;
    }


    strSQL=QString("select Count(*) from IntaiWeb_productlink Where ProductID='%1' and PartID='%2'").arg(strProduct).arg(pPart->PartID);
    if(!pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) || !pRS->GetValue(L"Count(*)",nCount))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    if(nCount<=0)
        strSQL=QString("insert into IntaiWeb_productlink(ProductID,PartID,PartCount) Values('%1','%2',%3)").arg(
                    strProduct).arg(
                    pPart->PartID).arg(
                    count);
    else
        strSQL=QString("update IntaiWeb_productlink set PartCount=%3 Where ProductID='%1' and PartID='%2'").arg(
                    strProduct).arg(
                    pPart->PartID).arg(
                    count);
    if(!pDB->ExecuteSQL(strSQL))
    {
        delete pRS;
        pDB->Close();
        return false;
    }

    delete pRS;
    pDB->Close();



    Part* pNew;
    std::map<QString,Part*>::iterator itMap=m_mapParts.find(pPart->PartID);
    if(itMap!=m_mapParts.end())
        pNew=itMap->second;
    else
    {
        pNew=new Part();
        m_mapParts.insert(std::make_pair(pPart->PartID,pNew));

    }
    pNew->CName=pPart->CName;
    pNew->Stock=pPart->Stock;
    //pNew->parts=pPart->parts;
    pNew->PartID=pPart->PartID;
    pNew->TypeID=pPart->TypeID;
    pNew->Specification=pPart->Specification;


    if(pProduct!=nullptr)
    {
        std::map<QString,int>::iterator itC=pProduct->parts.find(pPart->PartID);
        if(itC!=pProduct->parts.end())
            itC->second=count;
        else
            pProduct->parts.insert(std::make_pair(pPart->PartID,count));
    }
    else if(strProduct!=pPart->PartID)
    {
        itP2=m_mapParts.find(strProduct);
        if(itP2!=m_mapParts.end())
        {
            pNew=itP2->second;
            std::map<QString,int>::iterator itC=pNew->parts.find(pPart->PartID);
            if(itC!=pNew->parts.end())
                itC->second=count;
            else
                pNew->parts.insert(std::make_pair(pPart->PartID,count));
        }
    }
    return true;
}

bool HElcSignage::SavePart(Part * pPart)
{
    int nCount=0;
    std::map<QString,Part*>::iterator itP=m_mapParts.find(pPart->PartID);
    if(!(itP!=m_mapParts.end()))
        return false;

    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    QString strSQL=QString("select Count(*) from IntaiWeb_part Where PartID='%1'").arg(pPart->PartID);
    if(!pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) || !pRS->GetValue(L"Count(*)",nCount))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    if(nCount<=0)
        strSQL=QString("insert into IntaiWeb_part(PartID,CName,Specification,Stock,TypeID) Values('%1','%2','%3',%4,'%5')").arg(
                    pPart->PartID).arg(
                    pPart->CName).arg(
                    pPart->Specification).arg(
                    pPart->Stock).arg(
                    pPart->TypeID);
    else
        strSQL=QString("update IntaiWeb_part set CName='%2',Specification='%3',Stock=%4,TypeID='%5' Where PartID='%1'").arg(
                    pPart->PartID).arg(
                    pPart->CName).arg(
                    pPart->Specification).arg(
                    pPart->Stock).arg(
                    pPart->TypeID);
    if(!pDB->ExecuteSQL(strSQL))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    delete pRS;
    pDB->Close();

    delete itP->second;
    m_mapParts.erase(itP);
    if(pPart->PartID.size()>0)
        m_mapParts.insert(std::make_pair(pPart->PartID,pPart));
    else
        delete pPart;

    return true;
}

bool HElcSignage::DeletePart(Part *pPart)
{
    std::map<QString,Part*>::iterator itP;
    HDataBase* pDB;
    (m_pMySQLDB==nullptr)?(pDB=&m_LiteDB):(pDB=m_pMySQLDB);
    if(!pDB->Open())
        return false;
    QString strSQL=QString("delete from IntaiWeb_part Where PartID='%1'").arg(pPart->PartID);
    if(!pDB->ExecuteSQL(strSQL))
    {
        pDB->Close();
        return false;
    }
    strSQL=QString("delete from IntaiWeb_productlink Where ProductID='%1' or PartID='%1'").arg(pPart->PartID);
     if(!pDB->ExecuteSQL(strSQL))
    {
        pDB->Close();
        return false;
    }
    pDB->Close();

    itP=m_mapParts.find(pPart->PartID);
    if(itP!=m_mapParts.end())
    {
        delete itP->second;
        m_mapParts.erase(itP);
    }

    std::map<QString,int>::iterator itMap;
    for(itP=m_mapParts.begin();itP!=m_mapParts.end();itP++)
    {
        itMap=itP->second->parts.find(pPart->PartID);
        if(itMap!=itP->second->parts.end())
            itP->second->parts.erase(itMap);
    }

    std::map<QString,Product*>::iterator itP2;
    for(itP2=m_mapProducts.begin();itP2!=m_mapProducts.end();itP2++)
    {
        itMap=itP2->second->parts.find(pPart->PartID);
        if(itMap!=itP2->second->parts.end())
            itP2->second->parts.erase(itMap);
    }
    return true;
}

bool HElcSignage::SaveNewPart(Part *pPart)
{
    int nCount=0;
    std::map<QString,Part*>::iterator itP=m_mapParts.find(pPart->PartID);
    if(itP!=m_mapParts.end())
        return false;
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(!pDB->Open())
    {
        delete pRS;
        return false;
    }

    QString strSQL=QString("select Count(*) from IntaiWeb_part Where PartID='%1'").arg(pPart->PartID);
    if(!pRS->ExcelSQL(strSQL.toStdWString(),&m_LiteDB) || !pRS->GetValue(L"Count(*)",nCount))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    if(nCount<=0)
        strSQL=QString("insert into IntaiWeb_part(PartID,CName,Specification,Stock,TypeID) Values('%1','%2','%3',%4,'%5')").arg(
                    pPart->PartID).arg(
                    pPart->CName).arg(
                    pPart->Specification).arg(
                    pPart->Stock).arg(
                    pPart->TypeID);
    else
        strSQL=QString("update IntaiWeb_part set CName='%2',Specification='%3',Stock=%4,TypeID='%5' Where PartID='%1'").arg(
                    pPart->PartID).arg(
                    pPart->CName).arg(
                    pPart->Specification).arg(
                    pPart->Stock).arg(
                    pPart->TypeID);
    if(!pDB->ExecuteSQL(strSQL))
    {
        delete pRS;
        pDB->Close();
        return false;
    }
    delete pRS;
    pDB->Close();

    if(pPart->PartID.size()>0)
        m_mapParts.insert(std::make_pair(pPart->PartID,pPart));
    else
        delete pPart;
    return true;
}

bool HElcSignage::RemovePart(QString strProduct, QString strPart)
{
    QString strSQL;
    std::map<QString,Product*>::iterator itP;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
    }
    else
    {
        pDB=m_pMySQLDB;
    }
    if(!pDB->Open())
    {
        return false;
    }

    /*
    strSQL=QString("delete from Part Where PartID='%1'").arg(strPart);
    if(!pDB->ExecuteSQL(strSQL))
    {
        pDB->Close();
        return false;
    }
    */

    strSQL=QString("delete from IntaiWeb_productlink Where ProductID='%1' and PartID='%2'").arg(strProduct).arg(strPart);
    if(!pDB->ExecuteSQL(strSQL))
    {
        pDB->Close();
        return false;
    }
    pDB->Close();

    /*
    std::map<QString,Part*>::iterator itMap=m_mapParts.find(strPart);
    if(itMap!=m_mapParts.end())
    {
        delete itMap->second;
        m_mapParts.erase(itMap);
    }
    */
    std::map<QString,Part*>::iterator itMap;
    std::map<QString,int>::iterator itC;
    itMap=m_mapParts.find(strProduct);
    if(itMap!=m_mapParts.end())
    {
        itC=itMap->second->parts.find(strPart);
        if(itC!=itMap->second->parts.end())
            itMap->second->parts.erase(itC);
    }
    else
    {
        itP=m_mapProducts.find(strProduct);
        if(itP!=m_mapProducts.end())
        {
            itC=itP->second->parts.find(strPart);
            if(itC!=itP->second->parts.end())
                itP->second->parts.erase(itC);
        }
    }

    return true;
}



void HElcSignage::ClearProducts()
{
    std::map<QString,Product*>::iterator itP;
    for(itP=m_mapProducts.begin();itP!=m_mapProducts.end();itP++)
        delete itP->second;
    m_mapProducts.clear();

    std::map<QString,Part*>::iterator itP2;
    for(itP2=m_mapParts.begin();itP2!=m_mapParts.end();itP2++)
        delete itP2->second;
    m_mapParts.clear();

    std::map<QString,Process*>::iterator itP3;
    for(itP3=m_mapProcesses.begin();itP3!=m_mapProcesses.end();itP3++)
        delete itP3->second;
    m_mapProcesses.clear();

}

void HElcSignage::AddNewSystemData(QString name, int value)
{
    SystemData* pNew=new SystemData();
    pNew->name=name;
    pNew->nValue=value;
    pNew->dblValue=0;
    pNew->strValue.clear();
    AddNewSystemData(pNew);
}

void HElcSignage::AddNewSystemData(QString name, double value)
{
    SystemData* pNew=new SystemData();
    pNew->name=name;
    pNew->nValue=0;
    pNew->dblValue=value;
    pNew->strValue.clear();
    AddNewSystemData(pNew);
}

void HElcSignage::AddNewSystemData(QString name, QString value)
{
    SystemData* pNew=new SystemData();
    pNew->name=name;
    pNew->nValue=0;
    pNew->dblValue=0;
    pNew->strValue=value;
    AddNewSystemData(pNew);
}

void HElcSignage::AddNewSystemData(SystemData *pNew)
{
    SystemData *pOld;
    std::map<QString, SystemData*>::iterator itMap=m_mapSystemDatas.find(pNew->name);
    if(itMap!=m_mapSystemDatas.end())
    {
        pOld=itMap->second;
        itMap->second=pNew;
        delete pOld;
    }
    else
    {
        m_mapSystemDatas.insert(std::make_pair(pNew->name,pNew));
    }

}

bool HElcSignage::SaveSystemData(QString name, int value)
{
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
        pDB=&m_LiteDB;
    else
        pDB=m_pMySQLDB;
    QString strSQL;
    if(!pDB->Open())
        return false;
    std::map<QString, SystemData*>::iterator itMap=m_mapSystemDatas.find(name);
    if(itMap!=m_mapSystemDatas.end())
    {
        strSQL=QString("update IntaiWeb_systemset Set IntData=%1 Where DataName='%2'").arg(value).arg(name);
        if(pDB->ExecuteSQL(strSQL))
        {
            itMap->second->nValue=value;
            pDB->Close();
            return true;
        }
    }
    pDB->Close();
    return false;
}

bool HElcSignage::SaveSystemData(QString name, double value)
{
    HDataBase* pDB;
    (m_pMySQLDB==nullptr)?(pDB=&m_LiteDB):(pDB=m_pMySQLDB);
    QString strSQL;
    if(!pDB->Open())
        return false;
    std::map<QString, SystemData*>::iterator itMap=m_mapSystemDatas.find(name);
    if(itMap!=m_mapSystemDatas.end())
    {
        strSQL=QString("update IntaiWeb_systemset Set DblData=%1 Where DataName='%2'").arg(value).arg(name);
        if(pDB->ExecuteSQL(strSQL))
        {
            itMap->second->dblValue=value;
            return true;
        }
    }
    pDB->Close();
    return false;
}

bool HElcSignage::SaveSystemData(QString name, QString value)
{
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
        pDB=&m_LiteDB;
    else
        pDB=m_pMySQLDB;
    QString strSQL;
    if(!pDB->Open())
        return false;
    std::map<QString, SystemData*>::iterator itMap=m_mapSystemDatas.find(name);
    if(itMap!=m_mapSystemDatas.end())
    {
        strSQL=QString("update IntaiWeb_systemset Set StrData='%1' Where DataName='%2'").arg(value).arg(name);
        if(pDB->ExecuteSQL(strSQL))
        {
            itMap->second->strValue=value;
            return true;
        }
    }
    pDB->Close();
    return false;
}

bool HElcSignage::ExportWipTarget(QString strFile,Product* pProduct,Part* pPart)
{
    HRecordset* pRS;
    HDataBase* pDB;
    if(m_TypeSelect.size()<=0 || (pProduct==nullptr && pPart==nullptr))
        return false;

    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetSQLite();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }
    if(pDB==nullptr || !pDB->Open())
    {
        delete pRS;
        return false;
    }


    QString strSQL;
    if(pProduct!=nullptr)
        strSQL=QString("select *,PS.CName as PSName from IntaiWeb_processlink as L,IntaiWeb_product as P,IntaiWeb_process as PS Where L.ProductID=P.ProductID and P.TypeID='%1' and PS.ProcessID=L.ProcessID order by L.theOrder desc").arg(m_TypeSelect);
    else if(pProduct!=nullptr)
        strSQL=QString("select * from IntaiWeb_processlink as L,IntaiWeb_part as P Where L.ProductID=P.PartID and L.ProductID='%1' and P.TypeID='%2' order by L.theOrder desc").arg(pPart->PartID).arg(m_TypeSelect);
    else
    {
        delete pRS;
        return false;
    }
    if(!pRS->ExcelSQL(strSQL.toStdWString(),pDB))
    {
        delete pRS;
        pDB->Close();
        return false;
    }

    QString strID,strProcess,CName,strPSName;
    int nDay;
    while(!pRS->isEOF())
    {
        if(pProduct!=nullptr)
        {
            pRS->GetValue(L"ProductID",strID);
            pRS->GetValue(L"CName",CName);
            pRS->GetValue(L"ProcessID",strProcess);
            pRS->GetValue(L"PSName",strPSName);
            pRS->GetValue(L"DayTarget",nDay);
        }
        else if(pProduct!=nullptr)
        {

        }
        pRS->MoveNext();
    }


    // 創建一個QXlsx::Document對象
    /*
    QXlsx::Document xlsx;

    // 在工作表中寫入數據
    xlsx.write(QXlsx::CellReference(1,1),"Date");
    xlsx.write(QXlsx::CellReference(1,2),"Mes");
    xlsx.write(QXlsx::CellReference(1,3),"Count");
    xlsx.write(QXlsx::CellReference(1,4),"Product");


    Product* pP;
    int nRow=vShipments.size();
    for(int i=0;i<nRow;i++)
    {
        pShip=&vShipments[i];
        xlsx.write(QXlsx::CellReference(2+i,1),pShip->ship.date.toString("yyyy/MM/dd"));
        xlsx.write(QXlsx::CellReference(2+i,2),pShip->ProductID);
        xlsx.write(QXlsx::CellReference(2+i,3),pShip->ship.OutCount);
        gSystem->GetProductStruct(pShip->ProductID,&pP);
        if(pP!=nullptr)
            xlsx.write(QXlsx::CellReference(2+i,4),pP->CName);
    }

    // 保存Excel檔案
    xlsx.saveAs(fileName);

    */

    delete pRS;
    pDB->Close();
    return false;
}

bool HElcSignage::LoadSystemData(QString name, int &value)
{
    std::map<QString, SystemData*>::iterator itMap=m_mapSystemDatas.find(name);
    if(itMap!=m_mapSystemDatas.end())
    {
       value=itMap->second->nValue;
       return true;
    }
    return false;
}

bool HElcSignage::LoadSystemData(QString name, double &value)
{
    std::map<QString, SystemData*>::iterator itMap=m_mapSystemDatas.find(name);
    if(itMap!=m_mapSystemDatas.end())
    {
       value=itMap->second->dblValue;
       return true;
    }
    return false;
}

bool HElcSignage::LoadSystemData(QString name, QString &value)
{
    std::map<QString, SystemData*>::iterator itMap=m_mapSystemDatas.find(name);
    if(itMap!=m_mapSystemDatas.end())
    {
       value=itMap->second->strValue;
       return true;
    }
    return false;
}

bool HElcSignage::LoadStatus()
{
    std::map<QString,Product*>::iterator itProduct; // m_mapProducts;
    std::map<QString,Part*>::iterator itPart;       //    m_mapParts;
    if(m_WebService.isRunning())
        return false;

    Product* pProduct=nullptr;
    Part* pPart=nullptr;

    std::vector<STOCKDATA> stocks;
    for(itProduct=m_mapProducts.begin();itProduct!=m_mapProducts.end();itProduct++)
    {
        pProduct=itProduct->second;

        std::map<QString,double>::iterator itPos;
        std::map<QString,double> temps;  // pos + count
        std::vector<HStatusData> datas;
        while(m_WebService.StcSFCStatus(pProduct->ProductID,datas)<0);
        for(size_t i=0;i<datas.size();i++)
        {
            if(datas[i].status_DESCRIPTION=="Active" ||
               datas[i].status_DESCRIPTION=="InQueue" ||
               datas[i].status_DESCRIPTION=="Hold")
            {
                itPos=temps.find(datas[i].operation);
                if(itPos!=temps.end())
                    itPos->second += datas[i].in_WORK.toDouble();
                else
                    temps.insert(std::make_pair(datas[i].operation,datas[i].in_WORK.toDouble()));
            }
        }

        for(itPos=temps.begin();itPos!=temps.end();itPos++)
        {
            STOCKDATA sData;
            sData.id=pProduct->ProductID;
            sData.type=pProduct->TypeID;
            sData.pos=itPos->first;
            sData.count=itPos->second;
            if(sData.count>0)
                stocks.push_back(sData);
        }

        emit OnUpdateError(m_nRunUpdateIndex++);
    }

    for(itPart=m_mapParts.begin();itPart!=m_mapParts.end();itPart++)
    {
        pPart=itPart->second;

        std::map<QString,double>::iterator itPos;
        std::map<QString,double> temps;  // pos + count
        std::vector<HStatusData> datas;
        while(m_WebService.StcSFCStatus(pPart->PartID,datas)<0);
        for(size_t i=0;i<datas.size();i++)
        {
            if(datas[i].status_DESCRIPTION=="Active" ||
               datas[i].status_DESCRIPTION=="InQueue" ||
               datas[i].status_DESCRIPTION=="Hold")
            {
                itPos=temps.find(datas[i].operation);
                if(itPos!=temps.end())
                    itPos->second += datas[i].in_WORK.toDouble();
                else
                    temps.insert(std::make_pair(datas[i].operation,datas[i].in_WORK.toDouble()));
            }
        }

        for(itPos=temps.begin();itPos!=temps.end();itPos++)
        {
            STOCKDATA sData;
            sData.id=pPart->PartID;
            sData.type=pPart->TypeID;
            sData.pos=itPos->first;
            sData.count=itPos->second;
            if(sData.count>0)
                stocks.push_back(sData);
        }

        emit OnUpdateError(m_nRunUpdateIndex++);
    }
    if(stocks.size()<=0)
        return true;

    while(Save2DB("IntaiWeb_SFCStatus",stocks)<0);

    return stocks.size()>0;
}

bool HElcSignage::LoadHistorys()
{
    std::map<QString,Product*>::iterator itProduct; // m_mapProducts;
    std::map<QString,Part*>::iterator itPart;       //    m_mapParts;
    if(m_WebService.isRunning())
        return false;

    Product* pProduct=nullptr;
    Part* pPart=nullptr;

    std::vector<STOCKDATA> stocks;
    for(itProduct=m_mapProducts.begin();itProduct!=m_mapProducts.end();itProduct++)
    {
        pProduct=itProduct->second;

        std::map<QString,double>::iterator itPos;
        std::map<QString,double> temps;  // pos + count
        std::vector<HHistoryData> datas;
        while(m_WebService.StcSFCHistory(pProduct->ProductID,datas)<0);
        for(size_t i=0;i<datas.size();i++)
        {
            itPos=temps.find(datas[i].operation);
            if(itPos!=temps.end())
                itPos->second += datas[i].qtY_COMPLETED.toDouble();
            else
                temps.insert(std::make_pair(datas[i].operation,datas[i].qtY_COMPLETED.toDouble()));
        }

        for(itPos=temps.begin();itPos!=temps.end();itPos++)
        {
            STOCKDATA sData;
            sData.id=pProduct->ProductID;
            sData.type=pProduct->TypeID;
            sData.pos=itPos->first;
            sData.count=itPos->second;
            if(sData.count>0)
                stocks.push_back(sData);
        }

        emit OnUpdateError(m_nRunUpdateIndex++);
    }

    for(itPart=m_mapParts.begin();itPart!=m_mapParts.end();itPart++)
    {
        pPart=itPart->second;

        std::map<QString,double>::iterator itPos;
        std::map<QString,double> temps;  // pos + count
        std::vector<HHistoryData> datas;
        while(m_WebService.StcSFCHistory(pPart->PartID,datas)<0);
        for(size_t i=0;i<datas.size();i++)
        {
            itPos=temps.find(datas[i].operation);
            if(itPos!=temps.end())
                itPos->second += datas[i].qtY_COMPLETED.toDouble();
            else
                temps.insert(std::make_pair(datas[i].operation,datas[i].qtY_COMPLETED.toDouble()));
        }

        for(itPos=temps.begin();itPos!=temps.end();itPos++)
        {
            STOCKDATA sData;
            sData.id=pPart->PartID;
            sData.type=pPart->TypeID;
            sData.pos=itPos->first;
            sData.count=itPos->second;
            if(sData.count>0)
                stocks.push_back(sData);
        }
        emit OnUpdateError(m_nRunUpdateIndex++);
    }

    while(Save2DB("IntaiWeb_SFCHistory",stocks)<0);

    return true;//stocks.size()>0;
}

bool HElcSignage::LoadWips(int &ercode)
{
    std::map<QString,Product*>::iterator itProduct; // m_mapProducts;
    std::map<QString,Part*>::iterator itPart;       //    m_mapParts;
    ercode=0;
    if(m_WebService.isRunning())
    {
        ercode=1;
        return false;
    }

    Product* pProduct=nullptr;
    Part* pPart=nullptr;

    std::vector<STOCKDATA> stocks;
    for(itProduct=m_mapProducts.begin();itProduct!=m_mapProducts.end();itProduct++)
    {
        pProduct=itProduct->second;

        std::map<QString,double>::iterator itPos;
        std::map<QString,double> temps;  // pos + count
        std::vector<HWipData> datas;
        while(m_WebService.StcWip(pProduct->ProductID,datas)<0);
        for(size_t i=0;i<datas.size();i++)
        {
            itPos=temps.find(datas[i].LGORT);
            if(itPos!=temps.end())
                itPos->second += datas[i].CLAGS.toDouble();
            else
                temps.insert(std::make_pair(datas[i].LGORT,datas[i].CLAGS.toDouble()));
        }

        for(itPos=temps.begin();itPos!=temps.end();itPos++)
        {
            STOCKDATA sData;
            sData.id=pProduct->ProductID;
            sData.type=pProduct->TypeID;
            sData.pos=itPos->first;
            sData.count=itPos->second;
            if(sData.count>0)
                stocks.push_back(sData);
        }

        emit OnUpdateError(m_nRunUpdateIndex++);
    }

    for(itPart=m_mapParts.begin();itPart!=m_mapParts.end();itPart++)
    {
        pPart=itPart->second;

        std::map<QString,double>::iterator itPos;
        std::map<QString,double> temps;  // pos + count
        std::vector<HWipData> datas;
        while(m_WebService.StcWip(pPart->PartID,datas)<0);
        for(size_t i=0;i<datas.size();i++)
        {
            itPos=temps.find(datas[i].LGORT);
            if(itPos!=temps.end())
                itPos->second += datas[i].CLAGS.toDouble();
            else
                temps.insert(std::make_pair(datas[i].LGORT,datas[i].CLAGS.toDouble()));
        }

        for(itPos=temps.begin();itPos!=temps.end();itPos++)
        {
            STOCKDATA sData;
            sData.id=pPart->PartID;
            sData.type=pPart->TypeID;
            sData.pos=itPos->first;
            sData.count=itPos->second;
            if(sData.count>0)
                stocks.push_back(sData);
        }

        emit OnUpdateError(m_nRunUpdateIndex++);
    }

    while(Save2DB("IntaiWeb_ERPStock",stocks)<0);

    if(stocks.size()<=0)
    {
        ercode=-1;
        return false;
    }
    return true;
}

bool HElcSignage::ExportShip(std::map<QString,QString> &ProductInfos)
{
    std::map<QString,Product*>::iterator itProduct;
    //std::map<QString,QString> ProductInfos;
    for(itProduct=m_mapProducts.begin();itProduct!=m_mapProducts.end();itProduct++)
    {
        QString strType=itProduct->second->TypeID;
        if(strType.size()>6)
            ProductInfos.insert(std::make_pair(itProduct->first,strType.left(6)));
        else
            ProductInfos.insert(std::make_pair(itProduct->first,strType));
    }
    GetShipMents(ProductInfos,m_shipments);
    return ExportShip();
}

bool HElcSignage::ExportShip()
{
    std::map<QString,Product*>::iterator itProduct;
    if(m_WebService.isRunning())
        return false;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
        pDB=&m_LiteDB;
    else
        pDB=m_pMySQLDB;

    //m_shipments.clear();
    //HElcSignage::GetShipMents(std::map<QString,QString>& ProductInfos, std::map<QString, Shipment> &outShipments)
    //GetShipMents(ProductInfos,shipments);
    if(m_shipments.size()<=0)
        return false;

    std::vector<ShipTable> vShips;
    std::map<QString, Shipment>::iterator itShip;
    for(itShip=m_shipments.begin();itShip!=m_shipments.end();itShip++)
    {
        int st=itShip->first.indexOf(":");
        if(st>0)
        {
            QString strDate=itShip->first.left(st);
            QString strProduct=itShip->first.right(itShip->first.size()-st-1);
            Product* pProduct=nullptr;
            if(GetProductStruct(strProduct,&pProduct))
            {
                ShipTable tb;
                tb.ship=itShip->second;
                tb.TypeID=pProduct->TypeID;
                tb.ProductID=pProduct->ProductID;
                vShips.push_back(tb);

                emit OnUpdateError(m_nRunUpdateIndex++);
            }
        }
    }

    // save to db
    QString strSQL;
    if(pDB->Open())
    {
        strSQL="Delete from IntaiWeb_shiptable";
        pDB->ExecuteSQL(strSQL);
        for(int i=0;i<vShips.size();i++)
        {
            strSQL=QString("insert into IntaiWeb_shiptable(TypeID,ProductID,ShipDate,TargetCount,WipCount) Values('%1','%2','%3',%4,%5)").arg(
                        vShips[i].TypeID).arg(vShips[i].ProductID).arg(
                        vShips[i].ship.date.toString("yyyy/MM/dd")).arg(
                        vShips[i].ship.OutCount).arg(
                        -1*vShips[i].ship.StockCount);
            pDB->ExecuteSQL(strSQL);

            emit OnUpdateError(m_nRunUpdateIndex++);
        }
        pDB->Close();
    }


    return true;
}

bool HElcSignage::ExportBack()
{
    std::map<QString,QString>::iterator itType,itMainP;
    std::map<QString,QString>   mapDataTypes,mapMainParts;
    std::map<QString,int>::iterator itP;
    m_mapProcess.clear();
    HDataBase* pDB;
    HRecordset* pRS=nullptr;
    if(m_pMySQLDB==nullptr)
    {
        pDB=&m_LiteDB;
        pRS=new HRecordsetMySQL();
    }
    else
    {
        pDB=m_pMySQLDB;
        pRS=new HRecordsetMySQL();
    }

    // Products
    std::map<QString,Product*>::iterator itProduct;
    for(itProduct=m_mapProducts.begin();itProduct!=m_mapProducts.end();itProduct++)
    {
        std::list<ProcessInfo> lInfos;
        if(GetProductProcess(itProduct->first,lInfos))
            m_mapProcess.insert(std::make_pair(itProduct->first,lInfos));

        itType=mapDataTypes.find(itProduct->first);
        if(!(itType!=mapDataTypes.end()))
            mapDataTypes.insert(std::make_pair(itProduct->first,itProduct->second->TypeID));

        for(itP=itProduct->second->parts.begin();itP!=itProduct->second->parts.end();itP++)
        {
            itMainP=mapDataTypes.find(itP->first);
            if(!(itMainP!=mapDataTypes.end()))
            {
                mapDataTypes.insert(std::make_pair(itP->first,itProduct->second->TypeID));

                if(GetPartProcess(itP->first,lInfos))
                    m_mapProcess.insert(std::make_pair(itP->first,lInfos));
            }

        }

        emit OnUpdateError(m_nRunUpdateIndex++);
    }

    // Parts
    /*
    std::map<QString,Part*>::iterator itPart;
    for(itPart=m_mapParts.begin();itPart!=m_mapParts.end();itPart++)
    {
        std::list<ProcessInfo> lInfos;
        if(GetPartProcess(itPart->first,lInfos))
        {
            m_mapProcess.insert(std::make_pair(itPart->first,lInfos));

            itType=mapDataTypes.find(itPart->first);
            if(!(itType!=mapDataTypes.end()))
                mapDataTypes.insert(std::make_pair(itPart->first,itPart->second->TypeID));

        }
        emit OnUpdateError(m_nRunUpdateIndex++);
    }
    */

    // save to db
    std::map<QString,std::list<ProcessInfo>>::iterator itInfo;
    std::list<ProcessInfo>::iterator itL;
    std::list<ProcessInfo>* pLstProcess;
    ProcessInfo* pInfo;
    QString strSQL,PariantID;
    int nCount=0;

    if(pDB->Open())
    {
        //BackRunTable
        strSQL="Delete from IntaiWeb_runningtables";
        pDB->ExecuteSQL(strSQL);
        for(itInfo=m_mapProcess.begin();itInfo!=m_mapProcess.end();itInfo++)
        {
            pLstProcess=&itInfo->second;
            for(itL=pLstProcess->begin();itL!=pLstProcess->end();itL++)
            {
                pInfo=&(*itL);
                itType=mapDataTypes.find(pInfo->ProductID);
                if(itType!=mapDataTypes.end())
                {
                    if(!GetPartPariantID(pInfo->ProductID,PariantID,itInfo->first))
                        PariantID=pInfo->ProductID;
                    strSQL=QString("select count(*) fromIntaiWeb_runningtables Where TypeID='%1' and ProductID='%2' and ProcessIndex=%3").arg(
                                itType->second).arg(
                                PariantID).arg(
                                pInfo->order);
                    nCount=0;
                    if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
                        pRS->GetValue(L"count(*)",nCount);
                    if(nCount<=0)
                    {
                        strSQL=QString("insert into IntaiWeb_runningtables(TypeID,ProductID,ProcessIndex,ProcessID,WipCount,TargetCount,SumCount) Values('%1','%2',%3,'%4',%5,%6,%7)").arg(
                                    itType->second).arg(
                                    PariantID).arg(
                                    pInfo->order).arg(
                                    itL->ProcessID).arg(
                                    itL->Source).arg(
                                    itL->Target).arg(
                                    itL->sum);

                        pDB->ExecuteSQL(strSQL);
                    }
                }
                else
                {
                    if(GetPartPariantID(pInfo->ProductID,PariantID,itInfo->first))
                    {
                        itType=mapDataTypes.find(PariantID);
                        if(itType!=mapDataTypes.end())
                        {
                            strSQL=QString("select count(*) fromIntaiWeb_runningtables Where TypeID='%1' and ProductID='%2' and ProcessIndex=%3").arg(
                                        itType->second).arg(
                                        PariantID).arg(
                                        pInfo->order);
                            nCount=0;
                            if(pRS->ExcelSQL(strSQL.toStdWString(),pDB))
                                pRS->GetValue(L"count(*)",nCount);
                            if(nCount<=0)
                            {
                                strSQL=QString("insert into IntaiWeb_runningtables(TypeID,ProductID,ProcessIndex,ProcessID,WipCount,TargetCount,SumCount) Values('%1','%2',%3,'%4',%5,%6,%7)").arg(
                                            itType->second).arg(
                                            PariantID).arg(
                                            pInfo->order).arg(
                                            itL->ProcessID).arg(
                                            itL->Source).arg(
                                            itL->Target).arg(
                                            itL->sum);

                                pDB->ExecuteSQL(strSQL);
                            }
                        }
                    }
                }
            }

            emit OnUpdateError(m_nRunUpdateIndex++);
        }

        pDB->Close();
    }

    if(pRS!=nullptr) delete pRS;
    return true;
}

bool HElcSignage::CreateTitles(QString strType)
{
    std::vector<Product*>::iterator itVP;
    std::map<QString,Product*>::iterator itProduct;
    std::vector<Product*> vProducts;
    std::vector<Part *> vParts;
    std::map<QString,std::vector<QString>>  mapParts;

    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
        pDB=&m_LiteDB;
    else
        pDB=m_pMySQLDB;



    for(itProduct=m_mapProducts.begin();itProduct!=m_mapProducts.end();itProduct++)
    {
        if(itProduct->second->TypeID==strType)
            vProducts.push_back(itProduct->second);
    }
    if(vProducts.size()<=0)
        return false;

    std::map<QString,std::vector<QString>>::iterator itPart;
    std::vector<Part *>::iterator itP4;
    std::map<QString,int>::iterator itP3;
    std::vector<QString>::iterator itP1;
    std::vector<Product *>::iterator itP2;
    for(itVP=vProducts.begin();itVP!=vProducts.end();itVP++)
    {
        for(itP3=(*itVP)->parts.begin();itP3!=(*itVP)->parts.end();itP3++)
        {
            if(mapParts.size()<=0)
            {
                std::vector<QString> vData;
                vData.push_back(itP3->first);
                mapParts.insert(std::make_pair(itP3->first,vData));
                continue;
            }
            bool bFind=false;
            for(itPart=mapParts.begin();itPart!=mapParts.end();itPart++)
            {
                if(CheckPartProcess(itPart->first,itP3->first))
                {
                    bFind=true;
                    std::vector<QString>* pVParts=&itPart->second;
                    pVParts->push_back(itP3->first);
                    break;
                }
            }
            if(!bFind)
            {
                std::vector<QString> vData;
                vData.push_back(itP3->first);
                mapParts.insert(std::make_pair(itP3->first,vData));
            }
        }
    }

    for(itPart=mapParts.begin();itPart!=mapParts.end();itPart++)
    {
        // 首先排序
        std::sort(itPart->second.begin(), itPart->second.end());

        // 使用unique移除重复元素，并返回新的"尾后"迭代器
        auto last = std::unique(itPart->second.begin(), itPart->second.end());

        // 使用erase擦除重复元素
        itPart->second.erase(last, itPart->second.end());
    }

    if(!pDB->Open())
        return false;

    QString strSQL,strTitle;
    strSQL=QString("delete from IntaiWeb_titles Where TypeID='%1'").arg(strType);
    pDB->ExecuteSQL(strSQL);

    if(vProducts.size()>0)
    {
        strTitle=(*vProducts.begin())->ProductID;
        for(itVP=vProducts.begin();itVP!=vProducts.end();itVP++)
        {
            strSQL=QString("insert into IntaiWeb_titles(TypeID,ProductID,Members) Values('%1','%2','%3')").arg(
                        strType).arg(
                        strTitle).arg(
                        (*itVP)->ProductID);
            pDB->ExecuteSQL(strSQL);
        }

        for(itPart=mapParts.begin();itPart!=mapParts.end();itPart++)
        {
            for(itP1=itPart->second.begin();itP1!=itPart->second.end();itP1++)
            {
                strSQL=QString("insert into IntaiWeb_titles(TypeID,ProductID,Members) Values('%1','%2','%3')").arg(
                            strType).arg(
                            itPart->first).arg(
                            (*itP1));
                pDB->ExecuteSQL(strSQL);
            }
        }
    }
    else
    {
        pDB->Close();
        return false;
    }



    pDB->Close();
    return true;

}


int HElcSignage::Save2DB(QString strDB,std::vector<STOCKDATA> &stocks)
{
    QString strValue,strSQL;
    HDataBase* pDB;
    if(m_pMySQLDB==nullptr)
        pDB=&m_LiteDB;
    else
        pDB=m_pMySQLDB;

    if(!pDB->Open())
        return -1;
    strSQL=QString("delete from %1").arg(strDB);
    pDB->ExecuteSQL(strSQL);

    for(size_t i=0;i<stocks.size();i++)
    {
        if(strDB=="IntaiWeb_SFCHistory" || strDB=="IntaiWeb_SFCStatus")
        {
            strSQL=QString("insert into %5(ProductID,ProcessID,Count,Type) Values('%1','%2',%3,'%4')").arg(
                        stocks[i].id).arg(
                        stocks[i].pos).arg(
                        stocks[i].count).arg(
                        stocks[i].type).arg(
                        strDB);
            pDB->ExecuteSQL(strSQL);
        }
        else if(strDB=="IntaiWeb_ERPStock")
        {
            strSQL=QString("insert into %5(ProductID,StockID,StockCount,Type,MakeDate) Values('%1','%2',%3,'%4','')").arg(
                        stocks[i].id).arg(
                        stocks[i].pos).arg(
                        stocks[i].count).arg(
                        stocks[i].type).arg(
                        strDB);
            pDB->ExecuteSQL(strSQL);
        }
    }

    pDB->Close();
    return 0;
}



SystemData *HElcSignage::GetSystemData(QString name)
{
    std::map<QString, SystemData*>::iterator itMap=m_mapSystemDatas.find(name);
    if(itMap!=m_mapSystemDatas.end())
        return itMap->second;
    return nullptr;
}
