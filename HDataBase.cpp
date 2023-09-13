#include "HDataBase.h"
#include <vector>
#include <QLatin1String>
#include <QString>
#include <QSqlQuery>
#include <QImageReader>

/**************************************************************/

HRecordsetSQLite::HRecordsetSQLite()

{

}

HRecordsetSQLite::~HRecordsetSQLite()
{
    if(m_pQuery!=nullptr) delete m_pQuery;

    //if(m_pRecord!=0) delete m_pRecord;
}


bool HRecordsetSQLite::ExcelSQL(std::wstring  strSQL, HDataBase* pDB)
{
    if(pDB==nullptr) return false;

    HDataBaseSQLite* pDbLite=static_cast<HDataBaseSQLite*>(pDB);

    if(m_pQuery!=nullptr) delete m_pQuery;
    m_pQuery=new QSqlQuery(pDbLite->m_db);

    QString strQ=QString::fromStdWString(strSQL);
    m_pQuery->prepare(strQ);
    if(m_pQuery->exec())
    {
        m_pQuery->first();
        return true;
    }
    return false;
}



/************************************************************/
HDataBase::HDataBase()
{
}


HDataBase::~HDataBase()
{
}

QImage* HDataBase::TransBlob2Image(QByteArray &BData)
{
    if(BData.size()<=0)
        return nullptr;
    QBuffer bufImage(&BData);
    bufImage.open(QIODevice::ReadWrite);
    QImageReader reader(&bufImage,"bmp");
    QImage* pNew=new QImage(reader.read());
    if(pNew->isNull())
    {
        delete pNew;
        return nullptr;
    }
    return pNew;
}

QByteArray* HDataBase::TransImage2Blob(QImage *pImage)
{
    QBuffer bufImg;
    if(pImage==nullptr || pImage->isNull())
        return nullptr;
    bufImg.open(QIODevice::ReadWrite);
    pImage->save(&bufImg,"bmp");
    QByteArray *pBData=new QByteArray();
    pBData->append(bufImg.data());
    return pBData;
}

QFile* HDataBase::TransBlob2File(QByteArray &BData,QString fName)
{
    if(BData.size()<=0)
        return nullptr;

    QFile *pFile=new QFile(fName);
    if(pFile->open(QIODevice::WriteOnly))
    {
        pFile->write(BData,BData.size());
        pFile->close();
       return pFile;
    }
    delete pFile;
    return nullptr;
}



bool HDataBase::TransFile2Blob(QFile *pFile, QByteArray &BData)
{
    if(pFile==nullptr || !pFile->open(QIODevice::ReadWrite))
        return false;
    BData=pFile->readAll();
    pFile->close();
    return BData.size()>0;
}

void HDataBase::Close()
{
    try {
        m_db.close();
    } catch (...) {

    }

}

bool HDataBase::ExecuteSQL(QString strSQL)
{
    QSqlQuery *pQuery = new QSqlQuery(m_db);
    QString strMsg;

    try{
    if(pQuery->exec(strSQL))
    {
        delete pQuery;
        return true;
    }
    }
    catch(QException& e)
    {
        strMsg=e.what();
    }

    delete pQuery;
    return false;
}


bool HDataBase::CheckTableExist(QString  strTBName)
{
    std::string strTemp=strTBName.toStdString();
    QStringList list=m_db.tables();
    if (list.contains(QLatin1String(strTemp.c_str())))
        return true;
    strTemp=strTBName.toLower().toStdString();
    if (list.contains(QLatin1String(strTemp.c_str())))
        return true;
    return false;
}

bool HDataBase::CheckFieldExist(QString strTBName, QString strFieldName)
{
    std::string strTemp=strTBName.toStdString();
    if (m_db.tables().contains(QLatin1String(strTemp.c_str())))
        return false;
    strTemp=strTBName.toLower().toStdString();
        if (m_db.tables().contains(QLatin1String(strTemp.c_str())))
            return false;
    return false;
}


bool HDataBase::SetValue(QString strSQL,QString strField, QByteArray &value)
{
    bool ret=false;
    try {
        QSqlQuery query(m_db);
        query.prepare(strSQL);
        query.bindValue(strField,value);
        ret=query.exec();
    } catch (...) {
    }
    return ret;
}

bool HDataBase::SetValue2(QString strSQL, QString strValue, QByteArray &value)
{
    QSqlQuery query(m_db);
    query.prepare(strSQL);
    query.bindValue(strValue,value);
    return query.exec();
}


/*****************************************/
HDataBaseSQLite::HDataBaseSQLite()

{
}


HDataBaseSQLite::~HDataBaseSQLite()
{

}


bool HDataBaseSQLite::Open()
{
    if (m_strDBFile.size() <= 0) return false;
	return Open(m_strDBFile);
}

bool HDataBaseSQLite::Open(QString  strDBFile)
{
    try {
        if(strDBFile.isEmpty())
            return false;

        m_db=QSqlDatabase::addDatabase("QSQLITE");
        m_db.setDatabaseName(strDBFile);
        if(!m_db.open())
            return false;
        m_strDBFile=strDBFile;
    } catch (...) {
        return false;
    }
    return true;
}






/*****************************************/

HDataBaseMySQL::HDataBaseMySQL()

{
}


HDataBaseMySQL::~HDataBaseMySQL()
{

}

bool HDataBaseMySQL::Open(QString  strDBFile)
{
    #ifdef USE_MYSQL
    try {
        if(strDBFile.isEmpty())
            return false;

        /*
        bool bFind=false;
        qDebug() << "Available drivers:";
        QStringList drivers = QSqlDatabase::drivers();
        foreach(QString driver, drivers)
        {
            qDebug() << driver;
            if(driver=="QMYSQL")
            {
                bFind=true;
                //break;
            }
        }
        if(!bFind)
            return false;
        */
        m_db=QSqlDatabase::addDatabase("QMYSQL");
        m_db.setHostName("localhost");
        m_db.setPort(3306);
        m_db.setDatabaseName(strDBFile);
        m_db.setUserName("root");
        //m_db.setPassword("password");
        m_db.setPassword("hcs09871234");


        if(!m_db.open())
            return false;
        m_strDBFile=strDBFile;
    } catch (...) {
        return false;
    }
    return true;
    #else
    return false;
#endif
}

bool HDataBaseMySQL::Open()
{
    if (m_strDBFile.size() <= 0) return false;
    return Open(m_strDBFile);
}







/****************************************************************/
HRecordset::HRecordset()
    :m_pQuery(nullptr)
{

}

HRecordset::~HRecordset()
{

}


bool HRecordset::GetValue(std::wstring name, int &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    QVariant ret=m_pQuery->value(strName);
    if(ret.isValid())
    {
        value=ret.toInt();
        return true;
    }
    return false;
}

bool HRecordset::GetValue(std::wstring name, uint &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    QVariant ret=m_pQuery->value(strName);
    if(ret.isValid())
    {
        value=ret.toUInt();
        return true;
    }
    return false;
}

bool HRecordset::GetValue(std::wstring  name, double &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    value=m_pQuery->value(strName).toDouble();
    return true;
}

bool HRecordset::GetValue(std::wstring  name, float &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    value=m_pQuery->value(strName).toFloat();
    return true;
}

bool HRecordset::GetValue(std::wstring name, bool &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    QVariant v=m_pQuery->value(strName);
    if(v.type()==QVariant::Type::Bool)
    {
        value=v.toBool();
        return true;
    }
    return false;
}

bool HRecordset::GetValue(std::wstring  name, QString  &value)
{
    std::wstring strTemp;
    if(GetValue(name,strTemp))
    {
        value=QString::fromStdWString(strTemp);
        return true;
    }
    return false;
}

bool HRecordset::GetValue(std::wstring  name, std::wstring  &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    value=m_pQuery->value(strName).toString().toStdWString();
    return true;
}

bool HRecordset::GetValue(std::wstring  name, QByteArray  &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    value=m_pQuery->value(strName).toByteArray();
    return true;
}

bool HRecordset::GetValue(QString name, QVariant &value)
{
    if(m_pQuery==nullptr) return false;
    value=m_pQuery->value(name);
    return true;
}

bool HRecordset::GetType(QString name,QVariant::Type& type)
{
    if(m_pQuery==nullptr) return false;

    type=m_pQuery->value(name).type();
    return true;
}


bool HRecordset::isEOF()
{
    if(m_pQuery==nullptr)
        return true;
    if(!m_pQuery->isActive())
        return true;
    if(!m_pQuery->isSelect())
        return false;

    QSqlRecord recode = m_pQuery->record();
    //int count=recode.count();
    //return count<=0;

    return recode.isEmpty();
}

void HRecordset::MoveNext()
{
    if(m_pQuery==nullptr)
        return;
    if(!m_pQuery->isActive())
        return;
    if(!m_pQuery->isSelect())
        return;
    if(!m_pQuery->next())
    {
        delete m_pQuery;
        m_pQuery=nullptr;
    }
}


bool HRecordset::SetValue(std::wstring  name, QByteArray  &value)
{
    if(m_pQuery==nullptr) return false;

    QByteArray dataOut;
    QString strName=QString::fromStdWString(name);
    QSqlRecord rs=m_pQuery->record();
    QSqlField filed;
    if(rs.count()>0)
    {
        filed=rs.field(strName);

        if(filed.name()==strName)
        {
            filed.setValue(value);
            dataOut=filed.value().toByteArray();
            return true;
        }
    }
    return false;
}

bool HRecordset::SetValue(std::wstring  name, int  value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);
    QSqlRecord rs=m_pQuery->record();
    QSqlField filed;
    if(rs.count()>0)
    {
        filed=rs.field(strName);

        if(filed.name()==strName)
        {
            QVariant v=value;
            if(!filed.isReadOnly())
            {
                filed.setValue(v);
                return true;
            }
        }
    }
    return false;
}


/********************************************************************/
HRecordsetMySQL::HRecordsetMySQL()
{

}

HRecordsetMySQL::~HRecordsetMySQL()
{

}

bool HRecordsetMySQL::ExcelSQL(std::wstring strSQL, HDataBase *pDB)
{
    if(pDB==nullptr) return false;

    HDataBaseMySQL* pMySQLDB=static_cast<HDataBaseMySQL*>(pDB);

    if(m_pQuery!=nullptr) delete m_pQuery;
    m_pQuery=new QSqlQuery(pMySQLDB->m_db);

    QString strQ=QString::fromStdWString(strSQL);
    m_pQuery->prepare(strQ);
    if(m_pQuery->exec())
    {
        m_pQuery->first();
        return true;
    }
    return false;
}
