#include "hproduct.h"

/*
HProduct::HProduct()
{
    m_dblFirstPass=0.9;
}

HProduct::~HProduct()
{

}

void HProduct::Clear()
{
    std::map<QString,Shipment*>::iterator itS;
    for(itS=m_Shipments.begin();itS!=m_Shipments.end();itS++)
        delete itS->second;
    m_Shipments.clear();

    std::map<int,Process*>::iterator itP;
    for(itP=m_Processes.begin();itP!=m_Processes.end();itP++)
        delete itP->second;
    m_Processes.clear();



}



bool HProduct::SaveDB(HDataBase *pDB)
{

    HRecordsetSQLite rs;
    if(pDB==nullptr || !pDB->Open())
        return false;


    QString strSQL,strValue;
    int nValue;

    // 出貨資訊
    std::map<QString,Shipment*>::iterator itS;
    for(itS=m_Shipments.begin();itS!=m_Shipments.end();itS++)
    {
        Shipment *pShip=itS->second;
        strValue=pShip->date.toString("yyyy-MM-dd");
        strSQL=QString("Select Count(*) from Shipment Where ID='%1' and OutDate='%2'").arg(
                    m_PartNo).arg(strValue);
        if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
        {
            nValue=0;
            if(!rs.GetValue(L"Count(*)",nValue) || nValue<=0)
            {
                strSQL=QString("insert into Shipment(ID,OutDate,OutCount,StockCount) Values('%1','%2',%3,%4)").arg(
                            m_PartNo).arg(strValue).arg(pShip->OutCount).arg(pShip->StockCount);
            }
            else
            {
                strSQL=QString("update Shipment set OutCount=%3,StockCount=%4 Where ID='%1' and OutDate='%2'").arg(
                            m_PartNo).arg(strValue).arg(pShip->OutCount).arg(pShip->StockCount);
            }
            pDB->ExecuteSQL(strSQL);
        }
    }

    // 製程
    std::map<int,Process*>::iterator itP;
    Process* pProcess;
    for(itP=m_Processes.begin();itP!=m_Processes.end();itP++)
    {
        pProcess=itP->second;
        QString strPart;
        TransVector2QString(pProcess->components,strPart);

        strSQL=QString("Select Count(*) from MyProcess Where ID='%1' and PartNo='%2'").arg(
                    pProcess->id).arg(m_PartNo);
        if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
        {
            nValue=0;
            if(!rs.GetValue(L"Count(*)",nValue) || nValue<=0)
            {
                strSQL=QString("insert into MyProcess(ID,PartNo,CName,DataFrom,Stock,Need,theOrder,PartID) Values('%1','%2','%3',%4,%5,%6,%7,'%8')").arg(
                            pProcess->id).arg(m_PartNo).arg(pProcess->name).arg(pProcess->DataFrom).arg(pProcess->Stock).arg(
                            pProcess->Need).arg(itP->first).arg(strPart);
            }
            else
            {
                strSQL=QString("update MyProcess set CName='%3',DataFrom=%4,Stock=%5,Need=%6,theOrder=%7,PartID='%8' Where ID='%1' and PartNo='%2'").arg(
                            pProcess->id).arg(m_PartNo).arg(pProcess->name).arg(pProcess->DataFrom).arg(pProcess->Stock).arg(
                            pProcess->Need).arg(itP->first).arg(strPart);
            }
            pDB->ExecuteSQL(strSQL);
        }
    }

    // 組成
    std::map<QString,HProduct*>::iterator itC;
    strSQL=QString("Select Count(*) from MyProduct Where ID='%1'").arg(m_PartNo);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        nValue=0;
        if(!rs.GetValue(L"Count(*)",nValue) || nValue<=0)
        {
            strSQL=QString("insert into MyProduct(ID,CName,FirstPass) Values('%1','%2',%3)").arg(
                        m_PartNo).arg(m_Name).arg(m_dblFirstPass);
        }
        else
        {
            strSQL=QString("update MyProduct set CName='%2',FirstPass=%3 Where ID='%1'").arg(
                         m_PartNo).arg(m_Name).arg(m_dblFirstPass);
        }
        pDB->ExecuteSQL(strSQL);
    }







    pDB->Close();

    return true;
}

bool HProduct::LoadDB(HDataBase *pDB)
{
    /*
    HRecordsetSQLite rs;
    if(pDB==nullptr || !pDB->Open())
        return false;
    Clear();

    QString strID,strDate,strSQL;

    // 出貨資訊
    strSQL=QString("Select * from Shipment Where ID='%1'").arg(m_PartNo);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!rs.isEOF())
        {
            if(rs.GetValue(L"OutDate",strDate) && strDate.size()>0)
            {
                Shipment *pShip=new Shipment();
                pShip->date=QDate::fromString(strDate,"yyyy-MM-dd");
                rs.GetValue(L"OutCount",pShip->OutCount);
                rs.GetValue(L"StockCount",pShip->StockCount);
                m_Shipments.insert(std::make_pair(strDate,pShip));
            }
            rs.MoveNext();
        }
    }


    // 製程
    int nOrder;
    strSQL=QString("Select * from MyProcess Where PartNo='%1'  order by theOrder").arg(m_PartNo);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        while(!rs.isEOF())
        {
            strID="";
            if(rs.GetValue(L"ID",strID) && strID.size()>0)
            {
                Process *pProc=new Process();
                pProc->id=strID;
                pProc->PartNo=m_PartNo;
                rs.GetValue(L"CName",pProc->name);
                rs.GetValue(L"DataFrom",pProc->DataFrom);
                rs.GetValue(L"Stock",pProc->Stock);
                rs.GetValue(L"Need",pProc->Need);
                rs.GetValue(L"theOrder",nOrder);
                rs.GetValue(L"PartID",strDate);
                TransQString2Vector(strDate,pProc->components);

                m_Processes.insert(std::make_pair(nOrder,pProc));
            }
            rs.MoveNext();
        }
    }





    pDB->Close();

    return false;
}


void HProduct::TransBy2QStrings(QByteArray &byteArray, std::vector<QString> &outStrings)
{
    QString currentString;
    outStrings.clear();
    for (int i = 0; i < byteArray.size(); ++i)
    {
        char c = byteArray.at(i);
        if (c == '\0')
        {
            outStrings.push_back(currentString);
            currentString.clear();
        } else
            currentString.append(c);
    }
}

void HProduct::TransQString2Vector(QString strDatas, std::vector<QString> &outStrings)
{
    QString strSub;
    int st=0;
    int len=strDatas.indexOf(",");
    while(st>=0 && len>0)
    {
        outStrings.push_back(strDatas.mid(st,len));
        st=len+1;
        len=strDatas.indexOf(",",st);
    }
    if(st<strDatas.size())
        outStrings.push_back(strDatas.mid(st,strDatas.size()-st));
}

void HProduct::TransVector2QString(std::vector<QString> &outStrings, QString &strDatas)
{
    for(size_t i=0;i<outStrings.size();i++)
    {
        if(i!=0)
            strDatas=QString("%1,%2").arg(strDatas).arg(outStrings[i]);
        else
            strDatas=outStrings[i];
    }
}

QString HProduct::GetProcessPart(int id, int part)
{
    QString strPartID;
    std::map<int,Process*>::iterator itMap=m_Processes.find(id);
    if(itMap!=m_Processes.end())
    {
        if(part<itMap->second->components.size() && part>=0)
            strPartID=itMap->second->components[part];
    }
    if(strPartID.size()>0)
        return strPartID;

    return "";
}

*/
