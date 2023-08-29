#pragma once
#include <string>
#include <QString>
#include <QtSql>



/********************************************************/
class HDataBase
{
public:
	HDataBase();
    virtual ~HDataBase();
    virtual bool Open(QString strDBFile) = 0;
	virtual bool Open() = 0;
    virtual void Close();
    virtual bool ExecuteSQL(QString strSQL);
    virtual bool CheckTableExist(QString strTBName);
    virtual bool CheckFieldExist(QString strTBName,QString strField);

    virtual bool SetValue(QString strSQL,QString strField, QByteArray &value);
    virtual bool SetValue2(QString strSQL,QString strValue, QByteArray &value);


    QImage* TransBlob2Image(QByteArray& BData);
    QFile* TransBlob2File(QByteArray& BData,QString fName);

    QByteArray* TransImage2Blob(QImage* pImage);
    bool TransFile2Blob(QFile* pFile,QByteArray& BData);

public:
    QString         m_strDBName;
    QString         m_strDBFile;
    QSqlDatabase    m_db;

};


/**********************************************/
class HDataBaseSQLite :public HDataBase
{
public:
	HDataBaseSQLite();
	~HDataBaseSQLite();

    virtual bool Open(QString strDBFile);
	virtual bool Open();

};


/**********************************************/
class HDataBaseMySQL :public HDataBase
{
public:
    HDataBaseMySQL();
    ~HDataBaseMySQL();

    virtual bool Open(QString strDBFile);
    virtual bool Open();



};

/********************************************************/
class HRecordset
{
public:
    HRecordset();
    virtual ~HRecordset();

    virtual bool isEOF();
    virtual void MoveNext();
    virtual bool GetValue(std::wstring name, int &value);
    virtual bool GetValue(std::wstring name, uint &value);
    virtual bool GetValue(std::wstring name, double &value);
    virtual bool GetValue(std::wstring name, float &value);
    virtual bool GetValue(std::wstring name, bool &value);
    virtual bool GetValue(std::wstring name, std::wstring &value);
    virtual bool GetValue(std::wstring name, QString &value);
    virtual bool GetValue(std::wstring name, QByteArray &value);
    virtual bool GetValue(QString name,QVariant &value);
    virtual bool GetType(QString name,QVariant::Type& type);


    virtual bool SetValue(std::wstring name, int value);
    virtual bool SetValue(std::wstring name, QByteArray &value);

    virtual bool ExcelSQL(std::wstring strSQL, HDataBase* pDB) = 0;

    QSqlQuery       *m_pQuery;
};

/********************************************************/
class HRecordsetSQLite :public HRecordset
{
public:
	HRecordsetSQLite();
	virtual ~HRecordsetSQLite();

    virtual bool ExcelSQL(std::wstring strSQL, HDataBase* pDB);



};

/********************************************************/
class HRecordsetMySQL :public HRecordset
{
public:
    HRecordsetMySQL();
    virtual ~HRecordsetMySQL();

    virtual bool ExcelSQL(std::wstring strSQL, HDataBase* pDB);
};
