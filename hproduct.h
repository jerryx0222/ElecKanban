#ifndef HPRODUCT_H
#define HPRODUCT_H

#include <QObject>
#include <QDate>
#include "HDataBase.h"

enum DBFrom
{
    dbfErp,
    dbfMENow,
    dbfMeHis,
    dbManual,
};

struct SystemData
{
    QString name;
    QString strValue;
    int     nValue;
    double  dblValue;
};

struct STOCKDATA
{
    QString id;
    QString pos;
    QString type;
    double  count;
};

struct Shipment // 出貨
{
    QDate   date;       // 出貨日期
    int     OutCount;   // 出貨數量
    int     StockCount; // 存貨數量
};

struct ShipTable // 出貨
{
    Shipment    ship;
    QString     TypeID;
    QString     ProductID;

    double  rate;
    double  diff;
};




struct ERPStock
{
    QString productId;
    QString position;
    double  value;
    QDate makeDate;
};

struct SFCData
{
    QString productId;
    QString ProcessID;
    double  count;
};


struct Process
{
    QString ProcessID;   // 料號
    QString CName;       // 成品倉,最終包裝...
    int     order;       // 順序
    /*
    QString id;         // 3001,P081...


    int DataFrom;       // erp,Me,Manual...
    int Stock;          // 庫存
    int Need;           // 需求

    std::vector<QString>    components;    // 組成
    */
};


struct Product
{
    QString ProductID;
    QString CName;
    QString TypeID;
    double  FirstPass;
    int     Stock;

    std::map<QString,int>   parts;
};


struct Part
{
    QString PartID;
    QString CName;
    QString TypeID;
    QString Specification;
    int     Stock;

    std::map<QString,int>   parts;
};


struct ProcessInfo
{
    QString ProductID;
    QString ProcessID;
    QString ProcessName;
    int order;
    int Target;
    int Source;
    int sum;    // 累計
    int DayTarget;
};


struct ProcessTable
{
    QString type;
    QString ProductName;
    ProcessInfo info;
};



struct ProductCreate
{
    QString typeID,typeName;
    std::map<QString,QString>  mapDatas; // 物料+物料說明

    std::map<QString,Product>   mapProduct;
    std::map<QString,Part>  mapParts;

    std::map<QString,std::map<int,Process>>  mapProcess;
};

/*
class HProduct : public QObject
{
    Q_OBJECT
public:
    explicit HProduct();
    ~HProduct();

    void Clear();


    bool SaveDB(HDataBase* pDB);
    bool LoadDB(HDataBase* pDB);

    static void TransBy2QStrings(QByteArray& byDatas,std::vector<QString>& outStrings);
    static void TransQString2Vector(QString strDatas,std::vector<QString>& outStrings);
    static void TransVector2QString(std::vector<QString>& outStrings,QString &strDatas);

    QString GetProcessPart(int,int);

public:
    QString m_PartNo;                       // 料號
    QString m_Name;                         // 名稱
    double  m_dblFirstPass;                 // 直通率

    std::map<QString,Shipment*>      m_Shipments;   // 出貨資訊
    std::map<int,Process*>          m_Processes;    // 製程



signals:

public slots:
};
*/


#endif // HPRODUCT_H
