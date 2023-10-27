#ifndef HELCSIGNAGE_H
#define HELCSIGNAGE_H

#include <QObject>
#include "HDataBase.h"
#include "hproduct.h"
#include <QStandardItemModel>
#include "hwebservice.h"




enum RUNSTEP
{
    stepIdle,
    stepLoadHistory,
    stepLoadStatus,
    stepLoadERP,

    stepExportShip,

    stepExportBack,

    stepSaveDateTime,

    stepReStartUpdateTimer,


    stepCheckShipFile = 100,
    stepCheckExportFile,

};




class HElcSignage : public QThread
{
    Q_OBJECT
public:
    explicit HElcSignage(QObject *parent = nullptr);
    ~HElcSignage();

    void run() override;

    bool CreateDB();
    bool CopyDBs();
    bool LoadSysatem();
    bool LoadDB();
    bool LoadProducts();
    bool LoadParts();
    bool SaveDB();

    bool ResetUpdateTime();
    bool IsReady2Update();

    bool LoadFile2CreateProductEx(QString file,QString strPath);
    bool LoadFile2CreateProduct(QString file);
    bool LoadProcessIDFromFile(QString file);
    bool LoadProductLink();
    bool SaveProducts(QString type,std::map<QString,Product*>& datas);
    bool NewProducts(std::map<QString,QString>&);
    bool CopyProducts(QString type,std::map<QString,QString>&);
    bool DeleteProduct(QString id);
    bool SaveProcessDayCount(QString strPID,std::map<QString,int>&);

    bool GetProductsFromType(QString,std::vector<QString>&);
    bool GetProductStruct(QString pID,Product** pP);
    bool GetProductStruct(QString pID,std::vector<Product*>&);
    bool GetPartStruct(QString pID,Part** pP);
    bool GetPartStruct(QString pID,std::vector<Part*>&);
    bool GetProductProcess(QString pID,std::list<ProcessInfo> &datas);
    bool SavetProductProcess(QString pID,std::list<ProcessInfo> &datas);
    bool RemoveProductProcess(QString pID,QString Product,QString Part);
    bool GetPartProcess(QString pID,std::list<ProcessInfo> &datas);
    //bool GetPartProcess(QString ProductID,QString pID,std::list<ProcessInfo> &datas);
    bool GetPartProcess(QString pID,std::list<QString> &datas);
    bool GetPartPariantID(QString partID,QString& PariantID,QString pariDefault);

    bool GetPartCountSum(QString strPart,int& needed);
    bool CopyProcess(std::map<QString,QString>&);
    bool SaveProcess(QString id,QString name);
    //bool NewProcess(std::map<QString,QString>&);
    //bool DeleteProcess(QString id);
    bool CheckProcess(std::list<ProcessInfo>& info1,std::list<ProcessInfo>& info2);
    bool CheckProcess(std::list<QString>& info1,std::list<QString>& info2);
    bool CheckPartProcess(QString,QString);
    bool CheckProductProcess(QString,QString);
    QString GetProcessName(QString);

    bool CopyProductProcess(QString target,QString source);
    bool CopyPartProcess(QString target,QString source);
    bool CopyParts(QString,std::map<QString,QString>&);
    //bool NewParts(std::map<QString,QString>&);
    //bool DeletePart(QString id);

    //QString GetPart(QString id);
    void    GetDatesFromShipments(QString strType,QStringList& list);
    bool    AddNewShipment(QString,QDate&);
    bool    DelShipment(QDate&);
    void    GetShipMents(QStringList &Products,std::map<QString,Shipment>& outputs);    // 目前型號找出貨
    void    GetShipMents(std::map<QString,QString>& ProductInfos,std::map<QString,Shipment>& outputs); // 所有型號找出貨
    void    GetShipMentsFromDB(QString type,QString strDate,std::map<QString,ShipTable>&);        // 由型號日期找出貨
    void    GetShipMentsFromDB(QString type,std::vector<ShipTable>&);      // 由型號找出貨
    void    GetShipMentsFromDB(std::vector<ShipTable>&);      // 由型號找出貨

    void    GetBacksFromDB(QString type,std::vector<QString>& Products,std::map<int,ProcessTable>&);
    void    GetRunningsFromDB(QString type,QString Product,QString &ProductName,std::map<int,ProcessTable>&);

    bool    GetStockCount(QString type,QString productId,QString position,double&);
    bool    SaveShipMents(std::map<QString,Shipment>& );
    bool    ImportERPFile(QString type,QString);
    bool    ImportSFCStatusFile(QString,QString);
    bool    ImportSFCHistoryFile(QString,QString);
    bool    ImportTypeFile(QString);
    bool    ImportShipmentDateFile(QString);
    bool    ImportShipmentDateFile();

    bool    ExportRunnintTable(QString,QString,QString,bool);

    bool    SavePart(QString,QString,Part*,int);
    bool    SavePart(Part*);
    bool    DeletePart(Part*);
    bool    SaveNewPart(Part*);
    bool    RemovePart(QString,QString);
    bool    SaveParts(QString,std::map<QString, Part *> &datas);
    bool    GetTypeLists(std::map<QString,QString>&);
    bool    SaveTypeLists(std::map<QString,QString>&);
    bool    DeleteType(QString type);
    QString GetTypeFullName(QString type);


    bool GetSFC();


    bool    RunUpdateDataBase();
    bool    IsRunning();

    SystemData* GetSystemData(QString);

    bool ExportShip(std::map<QString,QString> &ProductInfos);

    bool LoadSystemData(QString,int&);
    bool LoadSystemData(QString,double&);
    bool LoadSystemData(QString,QString&);
    bool SaveSystemData(QString,int);
    bool SaveSystemData(QString,double);
    bool SaveSystemData(QString,QString);

    bool ExportWipTarget(QString,Product*,Part*);

private:
    void ClearProducts();
    void AddNewSystemData(QString,int);
    void AddNewSystemData(QString,double);
    void AddNewSystemData(QString,QString);
    void AddNewSystemData(SystemData*);

    bool LoadStatus();
    bool LoadHistorys();
    bool LoadWips(int &ercode);
    bool ExportShip();
    bool ExportBack();
    bool CreateTitles(QString);

    int Save2DB(QString,std::vector<STOCKDATA>&);

    bool CopyDBs(QString strTable,QStringList strFields);



public:
    HDataBaseSQLite m_LiteDB;
    HDataBaseMySQL  *m_pMySQLDB;

    std::map<QString,SystemData*> m_mapSystemDatas;
    std::map<QString,Product*> m_mapProducts;
    std::map<QString,Part*>    m_mapParts;
    std::map<QString,Process*> m_mapProcesses;

    std::map<QString, Shipment> m_shipments;
    HWebService m_WebService;
    QString m_TypeSelect;
    uint    m_nRunUpdateIndex;


private:
    QString m_strAppPath;
    std::map<QString,std::list<ProcessInfo>> m_mapProcess;
    QTimer  m_tmCheckShipDataFile;


signals:
    void OnUpdateError(int);
    void OnAutoUpdate(bool);
    void OnError(QString);

public slots:
    void OnCheckShipDataFile();
};

#endif // HELCSIGNAGE_H
