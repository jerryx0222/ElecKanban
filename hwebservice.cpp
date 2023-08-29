#include "hwebservice.h"
#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>

HWebService::HWebService()
    :m_pHistorys(nullptr),m_pStatuses(nullptr),m_pWipDatas(nullptr)

{
    m_strBearerToken = "69cfa069bba26b8bcfd0913501b172f8";


    m_bRunning=false;
}

void HWebService::run()
{
    if(m_pHistorys!=nullptr && m_pStatuses==nullptr && m_pWipDatas==nullptr)
    {
        if(GetHistory(m_strRunItem))
        {
            emit OnHistory();
            m_bRunning=false;
            return;
        }
    }
    else if(m_pStatuses!=nullptr && m_pHistorys==nullptr && m_pWipDatas==nullptr)
    {
        if(GetStatus(m_strRunItem))
        {
            emit OnStatus();
            m_bRunning=false;
            return;
        }
    }
    else if(m_pStatuses==nullptr && m_pHistorys==nullptr && m_pWipDatas!=nullptr)
    {
        if(GetWip(m_strRunItem))
        {
            emit OnWips();
            m_bRunning=false;
            return;
        }
    }
    emit OnError();
    m_bRunning=false;
}


bool HWebService::RunSFCHistory(QString item)
{
    if(m_bRunning)
        return false;

    if(!m_lockData.tryLockForWrite())
        return false;

    m_bRunning=true;
    m_strRunItem=item;
    if(m_pHistorys!=nullptr) delete m_pHistorys;
    if(m_pStatuses!=nullptr) delete m_pStatuses;
    if(m_pWipDatas!=nullptr) delete m_pWipDatas;
    m_pHistorys=new std::vector<HHistoryData>();
    m_pStatuses=nullptr;
    m_pWipDatas=nullptr;
    m_lockData.unlock();
    start();
    return true;
}

int HWebService::StcSFCHistory(QString item,std::vector<HHistoryData>& datas)
{
    if(m_bRunning)
        return -1;

    if(!m_lockData.tryLockForWrite())
        return -2;

    if(m_pHistorys!=nullptr) delete m_pHistorys;
    if(m_pStatuses!=nullptr) delete m_pStatuses;
    if(m_pWipDatas!=nullptr) delete m_pWipDatas;
    m_pHistorys=new std::vector<HHistoryData>();
    m_pStatuses=nullptr;
    m_pWipDatas=nullptr;
    m_lockData.unlock();

    if(!GetHistory(item))
        return 0;

    if(!CopyHistorys(datas))
        return 0;

    return static_cast<int>(datas.size());
}


bool HWebService::RunSFCStatus(QString item)
{
    if(m_bRunning)
        return false;

    if(!m_lockData.tryLockForWrite())
        return false;
    m_bRunning=true;
    m_strRunItem=item;
    if(m_pHistorys!=nullptr) delete m_pHistorys;
    if(m_pStatuses!=nullptr) delete m_pStatuses;
    if(m_pWipDatas!=nullptr) delete m_pWipDatas;
    m_pHistorys=nullptr;
    m_pWipDatas=nullptr;
    m_pStatuses=new std::vector<HStatusData>();
    m_lockData.unlock();
    start();
    return true;
}

int HWebService::StcSFCStatus(QString item,std::vector<HStatusData>& datas)
{
    if(m_bRunning)
        return -1;

    if(!m_lockData.tryLockForWrite())
        return -2;
    if(m_pHistorys!=nullptr) delete m_pHistorys;
    if(m_pStatuses!=nullptr) delete m_pStatuses;
    if(m_pWipDatas!=nullptr) delete m_pWipDatas;
    m_pHistorys=nullptr;
    m_pStatuses=new std::vector<HStatusData>();
    m_pWipDatas=nullptr;
    m_lockData.unlock();

    if(!GetStatus(item))
        return 0;

    if(!CopyStatuses(datas))
        return 0;

    return static_cast<int>(datas.size());
}

bool HWebService::RunWip(QString item)
{
    if(m_bRunning)
        return false;

    if(!m_lockData.tryLockForWrite())
        return false;
    m_bRunning=true;
    m_strRunItem=item;
    if(m_pHistorys!=nullptr) delete m_pHistorys;
    if(m_pStatuses!=nullptr) delete m_pStatuses;
    if(m_pWipDatas!=nullptr) delete m_pWipDatas;
    m_pHistorys=nullptr;
    m_pStatuses=nullptr;
    m_pWipDatas=new std::vector<HWipData>();
    m_lockData.unlock();
    start();
    return true;
}

bool HWebService::CopyHistorys(std::vector<HHistoryData> &datas)
{
    if(!m_lockData.tryLockForRead())
        return false;
    if(m_pHistorys==nullptr)
    {
        m_lockData.unlock();
        return false;
    }
    for(size_t i=0;i<m_pHistorys->size();i++)
        datas.push_back((*m_pHistorys)[i]);
    m_lockData.unlock();
    return datas.size()>0;
}

bool HWebService::CopyStatuses(std::vector<HStatusData> &datas)
{
    if(!m_lockData.tryLockForRead())
        return false;
    if(m_pStatuses==nullptr)
    {
        m_lockData.unlock();
        return false;
    }
    for(size_t i=0;i<m_pStatuses->size();i++)
        datas.push_back((*m_pStatuses)[i]);
    m_lockData.unlock();
    return datas.size()>0;
}

bool HWebService::CopyWips(std::vector<HWipData> &datas)
{
    if(!m_lockData.tryLockForRead())
        return false;
    if(m_pWipDatas==nullptr)
    {
        m_lockData.unlock();
        return false;
    }
    for(size_t i=0;i<m_pWipDatas->size();i++)
        datas.push_back((*m_pWipDatas)[i]);
    m_lockData.unlock();
    return datas.size()>0;
}

int HWebService::StcWip(QString item,std::vector<HWipData>& datas)
{
    if(m_bRunning)
        return -1;
    if(!m_lockData.tryLockForWrite())
        return -2;

    if(m_pHistorys!=nullptr) delete m_pHistorys;
    if(m_pStatuses!=nullptr) delete m_pStatuses;
    if(m_pWipDatas!=nullptr) delete m_pWipDatas;
    m_pHistorys=nullptr;
    m_pStatuses=nullptr;
    m_pWipDatas=new std::vector<HWipData>();
    m_lockData.unlock();

    if(!GetWip(item))
        return 0;

    if(!CopyWips(datas))
        return 0;

    return static_cast<int>(datas.size());
}





bool HWebService::GetStatus(QString strItem)
{
    // SFC 現況
    //QString strLink="https://win2016-testvm.intai-corp.com:5004/api/me/getsfcwip?site=1002&item=0264D32173P01
    //QString strLink="https://win2016-testvm.intai-corp.com:5004/api/me/getsfcwip?site=1002";
    QString strLink=m_strStatusLink;
    QString strOrder="";
    QString strOP="";

    if(strOrder.size()>0)
        strLink+= QString("&order=%1").arg(strOrder);
    if(strOP.size()>0)
        strLink+= QString("&OP=%1").arg(strOP);
    if(strItem.size()>0)
        strLink+= QString("&item=%1").arg(strItem);
    else
        return false;

    QJsonArray jsonArray;
    if(GetSFC(strLink,jsonArray))
    {
        //qDebug() << jsonArray;
        RelistStatusDatas(jsonArray);
        return m_pStatuses!=nullptr && m_pStatuses->size()>0;
    }
    return false;
}

bool HWebService::GetHistory(QString strItem)
{
    // SFC歷程
    //QString strLink="https://win2016-testvm.intai-corp.com:5004/api/me/getsfchis?site=1002";
    QString strLink=this->m_strHistoryLink;
    QString strOrder="";
    QString strOP="";


    if(strOrder.size()>0)
        strLink+= QString("&order=%1").arg(strOrder);
    if(strOP.size()>0)
        strLink+= QString("&OP=%1").arg(strOP);
    if(strItem.size()>0)
        strLink+= QString("&item=%1").arg(strItem);
    else
        return false;

    QDate date=QDate::currentDate();
    QDate dateOld=date.addDays(-1);
    strLink+= QString("&starttime=%1 00:00:00").arg(dateOld.toString("yyyy-MM-dd"));
    strLink+= QString("&eddtime=%1 23:59:59").arg(date.toString("yyyy-MM-dd"));


    QJsonArray jsonArray;
    if(GetSFC(strLink,jsonArray))
    {
        //qDebug() << jsonArray;
        RelistHistoryDatas(jsonArray);
        return m_pHistorys!=nullptr && m_pHistorys->size()>0;
    }

    return false;
}



bool HWebService::GetSFC(QString strLink,QJsonArray &jsonArray)
{
    QUrl url(strLink);
    QNetworkRequest request(url);

    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone); // Disable SSL
    request.setSslConfiguration(sslConfig);


    QByteArray authHeader = "Bearer " + m_strBearerToken.toUtf8();
    request.setRawHeader("Authorization", authHeader);

    QNetworkAccessManager m_manager;
    QNetworkReply *reply = m_manager.get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString jsonString;
    if (reply->error() == QNetworkReply::NoError)
    {
        jsonString = reply->readAll();
        //qDebug() << "Response:" << jsonString;
    }
    else
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
            jsonArray = jsonDoc.array();
            reply->deleteLater();
            return true;
        }
        else
        {
            qDebug() << "jsonDoc is not Array.";
        }
    }
    else
    {
        qDebug() << "Failed to parse JSON.";
    }



    reply->deleteLater();
    return false;
}


void HWebService::RelistStatusDatas(QJsonArray &jsonArray)
{
    QStringList keys;
    QString strValue;
    m_lockData.lockForWrite();
    if(m_pStatuses==nullptr)
    {
        m_lockData.unlock();
        return;
    }


    foreach (const QJsonValue& jValue , jsonArray)
    {
        if (jValue.isObject())
        {
            HStatusData sData;
            QJsonObject jsonObj = jValue.toObject();
            keys = jsonObj.keys();
           foreach (const QString& key, keys)
            {
                strValue="---";
                switch(jsonObj[key].type())
                {
                case QJsonValue::Type::Null:
                    strValue="";
                    break;
                case QJsonValue::Type::Bool:
                    (jsonObj[key].toBool())?(strValue="true"):(strValue="false");
                    break;
                case QJsonValue::Type::Double:
                    strValue=QString("%1").arg(jsonObj[key].toDouble(),0,'f',3);
                    break;
                case QJsonValue::Type::String:
                    strValue=jsonObj[key].toString();
                    break;
                case QJsonValue::Type::Array:
                case QJsonValue::Type::Object:
                case QJsonValue::Type::Undefined:
                    strValue="---";
                    break;
                }
                QString strName=key.toLower();
                if(strName=="site")
                    sData.site=strValue;
                else if(strName=="item")
                    sData.item=strValue;
                else if(strName=="shop_order")
                    sData.shop_Order=strValue;
                else if(strName=="sfc")
                    sData.sfc=strValue;
                else if(strName=="step_sequence")
                    sData.step_SEQUENCE=strValue;
                else if(strName=="operation")
                    sData.operation=strValue;
                else if(strName=="op_desc")
                    sData.op_DESC=strValue;
                else if(strName=="in_work")
                    sData.in_WORK=strValue;
                else if(strName=="status_description")
                    sData.status_DESCRIPTION=strValue;
            }
            m_pStatuses->push_back(sData);
        }
    }
    m_lockData.unlock();
}

void HWebService::RelistHistoryDatas(QJsonArray &jsonArray)
{
    QStringList keys;
    QString strValue;
    m_lockData.lockForWrite();
    if(m_pHistorys==nullptr)
    {
        m_lockData.unlock();
        return;
    }

    QDate dateEnd,dateNow=QDate::currentDate();

    foreach(const QJsonValue& jValue , jsonArray)
    {
        if (jValue.isObject())
        {
            HHistoryData sData;
            QJsonObject jsonObj = jValue.toObject();
            keys = jsonObj.keys();
           foreach (const QString& key, keys)
            {
                strValue="---";

                switch(jsonObj[key].type())
                {
                case QJsonValue::Type::Null:
                    strValue="";
                    break;
                case QJsonValue::Type::Bool:
                    (jsonObj[key].toBool())?(strValue="true"):(strValue="false");
                    break;
                case QJsonValue::Type::Double:
                    strValue=QString("%1").arg(jsonObj[key].toDouble(),0,'f',3);
                    break;
                case QJsonValue::Type::String:
                    strValue=jsonObj[key].toString();
                    break;
                case QJsonValue::Type::Array:
                case QJsonValue::Type::Object:
                case QJsonValue::Type::Undefined:
                    strValue="---";
                    break;
                }

                QString strName=key.toLower();
                if(strName=="site")
                    sData.site=strValue;
                else if(strName=="item")
                    sData.item=strValue;
                else if(strName=="shop_order")
                    sData.shop_Order=strValue;
                else if(strName=="sfc")
                    sData.sfc=strValue;
                else if(strName=="batch")
                    sData.batch=strValue;
                else if(strName=="operation")
                    sData.operation=strValue;
                else if(strName=="op_desc")
                    sData.op_DESC=strValue;
                else if(strName=="item_type")
                    sData.item_TYPE=strValue;
                else if(strName=="item_desc")
                    sData.item_DESC=strValue;
                else if(strName=="operation")
                    sData.operation=strValue;
                else if(strName=="op_desc")
                    sData.op_DESC=strValue;
                else if(strName=="resource")
                    sData.resource=strValue;
                else if(strName=="start_time")
                    sData.start_TIME=strValue;
                else if(strName=="end_time")
                    sData.end_TIME=strValue;
                else if(strName=="work_minute")
                    sData.worK_MINUTE=strValue;
                else if(strName=="qty_started")
                    sData.qtY_STARTED=strValue;
                else if(strName=="qty_completed")
                    sData.qtY_COMPLETED=strValue;
                else if(strName=="qty_scrapped")
                    sData.qtY_SCRAPPED=strValue;
                else
                {
                    QString strTemp=strName;
                }
            }

            int pos=sData.end_TIME.indexOf("T");
            if(pos>0)
            {
                strValue=sData.end_TIME.left(pos);
                strValue += " ";
                strValue += sData.end_TIME.right(sData.end_TIME.size()-pos-1);
                //dateEnd=QDate::fromString(strValue,"yyyy-MM-dd HH:mm::ss");
                dateEnd=QDate::fromString(sData.end_TIME,Qt::ISODate);
                if(dateEnd>=dateNow)
                    m_pHistorys->push_back(sData);
            }
            else
                m_pHistorys->push_back(sData);
        }
    }
    m_lockData.unlock();
}

void HWebService::RelistWipDatas(QString strItem, QByteArray &soapData)
{
    QDomDocument doc;
    if (!doc.setContent(soapData))
    {
        // 解析XML失敗
        qDebug() << "Failed to parse XML response";
        return;
    }

    QDomElement root = doc.documentElement();
    // 在此處理根元素及其子元素

    // 檢查根元素名稱是否為 "soap-env:Envelope"
    if (root.tagName() != "soap-env:Envelope")
        return;

    // 找到<soap-env:Body>元素
    QDomElement bodyElement = root.firstChildElement("soap-env:Body");
    if (bodyElement.isNull())
        return;

    // 找到<n0:ZMM036F1Response>元素
    QDomElement responseElement = bodyElement.firstChildElement("n0:ZMM036F1Response");
    if (responseElement.isNull())
        return;

    // 找到<GT_OUTPUT>元素
    QDomElement outputElement = responseElement.firstChildElement("GT_OUTPUT");
    if (outputElement.isNull())
        return;

    std::map<QString,double>::iterator itMap;
    std::map<QString,double> mapLGORT;
    // 遍歷<item>元素
    QDomNodeList itemNodes = outputElement.elementsByTagName("item");
    for (int i = 0; i < itemNodes.size(); ++i)
    {
        QDomElement itemElement = itemNodes.at(i).toElement();
        // 擷取子元素的資料
        QString messageType = itemElement.firstChildElement("MESSAGE_TYPE").text();
        if(messageType!="S")
            continue;
        QString werks = itemElement.firstChildElement("WERKS").text();
        QString lgort = itemElement.firstChildElement("LGORT").text();
        QString matnr = itemElement.firstChildElement("MATNR").text();
        //QString charg = itemElement.firstChildElement("CHARG").text();
        QString clabs = itemElement.firstChildElement("CLABS").text();
        if(werks=="1002" && matnr==strItem)
        {
            itMap=mapLGORT.find(lgort);
            if(itMap!=mapLGORT.end())
                itMap->second+=clabs.toDouble();
            else
                mapLGORT.insert(std::make_pair(lgort,clabs.toDouble()));
        }
    }

    for(itMap=mapLGORT.begin();itMap!=mapLGORT.end();itMap++)
    {
        HWipData wData;
        wData.WERKS="1002";
        wData.MATNR=strItem;
        wData.LGORT=itMap->first;
        wData.CLAGS=QString("%1").arg(itMap->second);
        m_pWipDatas->push_back(wData);
    }
}



bool HWebService::GetWip(QString strItem)
{
    // ERP 庫存
    //測試版:QString strSoap="http://erpqasap.intai-corp.com:8000/sap/bc/srt/rfc/sap/zmm036f1/288/zmm036f1/zmm036f1";
    //QString strSoap="http://erpprdap01.intai-corp.com:8000/sap/bc/srt/rfc/sap/zmm036f1/888/zmm036f1/zmm036f1";
    QString strSoap=m_strWipLink;

    // 設定 Web service 的 URL
    QUrl serviceUrl(strSoap);

    QNetworkAccessManager  m_networkManager;

    // 建立 SOAP 訊息
    QDomDocument soapMessage;

    QDomElement envelope = soapMessage.createElement("soapenv:Envelope");//soapMessage.createElementNS("http://schemas.xmlsoap.org/soap/envelope/", "soap:Envelope");
    envelope.setAttribute("xmlns:soapenv", "http://schemas.xmlsoap.org/soap/envelope/");
    envelope.setAttribute("xmlns:urn", "urn:sap-com:document:sap:rfc:functions");
    soapMessage.appendChild(envelope);

    QDomElement body = soapMessage.createElement("soapenv:Body");
    envelope.appendChild(body);

    // 建立 <urn:ZMM036F1> 元素
    QDomElement zmm036f1 = soapMessage.createElement("urn:ZMM036F1");
    body.appendChild(zmm036f1);

    // 建立 <GT_INPUT> 元素
    QDomElement gtInput = soapMessage.createElement("GT_INPUT");
    zmm036f1.appendChild(gtInput);

    // 建立第一個 <item> 元素
    QDomElement item1 = soapMessage.createElement("item");
    gtInput.appendChild(item1);

    // 建立 <MATNR> 元素，並設定內容
    QDomElement matnr1 = soapMessage.createElement("MATNR");
    matnr1.appendChild(soapMessage.createTextNode(strItem));
    item1.appendChild(matnr1);

    // 建立 <CHARG> 元素，並設定內容
    QDomElement charg1 = soapMessage.createElement("CHARG");
    item1.appendChild(charg1);

    // 建立 <LGORT> 元素，並設定內容
    QDomElement lgort1 = soapMessage.createElement("LGORT");
    item1.appendChild(lgort1);

    /*
    // 建立第二個 <item> 元素
    QDomElement item2 = soapMessage.createElement("item");
    gtInput.appendChild(item2);

    // 建立 <MATNR> 元素，並設定內容
    QDomElement matnr2 = soapMessage.createElement("MATNR");
    matnr2.appendChild(soapMessage.createTextNode("02480090001"));
    item2.appendChild(matnr2);

    // 建立 <CHARG> 元素，並設定內容
    QDomElement charg2 = soapMessage.createElement("CHARG");
    item2.appendChild(charg2);

    // 建立 <LGORT> 元素，並設定內容
    QDomElement lgort2 = soapMessage.createElement("LGORT");
    lgort2.appendChild(soapMessage.createTextNode("3001"));
    item2.appendChild(lgort2);
    */

    // 建立 <GT_OUTPUT> 元素
    QDomElement gtOutput = soapMessage.createElement("GT_OUTPUT");
    zmm036f1.appendChild(gtOutput);

    // 建立 <GV_WERKS> 元素，並設定內容
    QDomElement gvWerks = soapMessage.createElement("GV_WERKS");
    gvWerks.appendChild(soapMessage.createTextNode("1002"));
    zmm036f1.appendChild(gvWerks);

    // 將 QDomDocument 轉換為 QByteArray
    QByteArray requestData = soapMessage.toByteArray();
    //qDebug() << "Request Data:" << requestData;

    // 建立 QNetworkRequest
    QNetworkRequest m_Request(serviceUrl);
    m_Request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml;charset=UTF-8");
    m_Request.setHeader(QNetworkRequest::ContentLengthHeader, QString::number(requestData.size()));

    // 添加帳號和密碼的身份驗證
    QString username = m_strERPName;    //"A110110";
    //QString password = "sap1234";     // 測試版
    //QString password = "1234ABcd#";
    QString password = m_strERPPwd;

    QString concatenated = username + ":" + password;
    QByteArray data = concatenated.toUtf8().toBase64();
    QString headerData = "Basic " + data;
    m_Request.setRawHeader("Authorization", headerData.toUtf8());

    // 發送 SOAP 請求
    QNetworkReply *m_pReply = m_networkManager.post(m_Request, requestData);

    // 監聽請求完成事件
    QEventLoop loop;
    QObject::connect(m_pReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // 檢查回應狀態
    QByteArray responseData;
    if (m_pReply->error() == QNetworkReply::NoError) {
        // 讀取回應的內容
        responseData = m_pReply->readAll();

        // 在此處理回應的 SOAP 資料
        // ...

        // 輸出回應的內容
        //qDebug() << responseData;
    } else
    {
        // 在此處理錯誤情況
        qDebug() << "Error: " << m_pReply->errorString();
        m_pReply->deleteLater();
        delete m_pReply;
        m_pReply=nullptr;
        return false;
    }
    m_pReply->deleteLater();
    delete m_pReply;
    m_pReply=nullptr;

    RelistWipDatas(strItem,responseData);
    return m_pWipDatas!=nullptr && m_pWipDatas->size()>0;
}

