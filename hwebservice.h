#ifndef HWEBSERVICE_H
#define HWEBSERVICE_H

#include <QThread>
#include <QDate>
#include <QReadWriteLock>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QDomElement>
#include <vector>

struct HHistoryData
{
    QString site;
    QString shop_Order;
    QString sfc;
    QString batch;
    QString item;
    QString item_TYPE;
    QString item_DESC;
    QString operation;
    QString op_DESC;
    QString resource;
    QString start_TIME;
    QString end_TIME;
    QString worK_MINUTE;
    QString qtY_STARTED;
    QString qtY_COMPLETED;
    QString qtY_SCRAPPED;
};


struct HStatusData
{
    QString site;
    QString item;
    QString shop_Order;
    QString sfc;
    QString step_SEQUENCE;
    QString operation;
    QString op_DESC;
    QString in_WORK;
    QString status_DESCRIPTION;
};

struct HWipData
{
    QString WERKS;      // 型態：CHAR,長度： 4,說明：工廠
    QString MATNR;      // 型態：CHAR,長度：40,說明：物料
    QString CLAGS;      // 型態：CHAR,長度：10,說明：批次
    QString LGORT;      // 型態：CHAR,長度： 4,說明：儲存地點
};

class HWebService : public QThread
{
    Q_OBJECT

public:
    HWebService();

    void run() override;


    bool    RunSFCHistory(QString item);
    bool    RunSFCStatus(QString item);
    bool    RunWip(QString item);

    bool    CopyHistorys(std::vector<HHistoryData>&);
    bool    CopyStatuses(std::vector<HStatusData>&);
    bool    CopyWips(std::vector<HWipData>&);

    bool    IsRunning(){return m_bRunning;}


    int    StcSFCHistory(QString,std::vector<HHistoryData>&);
    int    StcSFCStatus(QString,std::vector<HStatusData>&);
    int    StcWip(QString,std::vector<HWipData>&);


signals:
    void  OnHistory();
    void  OnStatus();
    void  OnWips();
    void  OnError();

private:
    bool GetSFC(QString strLink,QJsonArray &jsonArray);
    bool GetStatus(QString strItem);
    bool GetHistory(QString strItem);
    void RelistStatusDatas(QJsonArray &jsonArray);
    void RelistHistoryDatas(QJsonArray &jsonArray);
    void RelistWipDatas(QString strItem,QByteArray &soapData);

    bool GetWip(QString strItem);

private slots:


private:
    std::vector<HHistoryData>*  m_pHistorys;
    std::vector<HStatusData>*   m_pStatuses;
    std::vector<HWipData>*      m_pWipDatas;

    //QStringList m_lstStatuses; //m_lstHistorys

    QString m_strRunItem;
    QReadWriteLock  m_lockData;

    bool    m_bRunning;

public:
    QString m_strBearerToken;
    QString m_strStatusLink,m_strHistoryLink,m_strWipLink;
    QString m_strERPName,m_strERPPwd;

};

#endif // HWEBSERVICE_H
