#include "vwebservice.h"
#include "ui_vwebservice.h"
#include "helcsignage.h"

#include <QtCore/QCoreApplication>
#include <QDebug>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QDomElement>
#include <QFileDialog>
#include <QMessageBox>

extern HElcSignage* gSystem;
extern int gStep;
extern uint gUpdateCount;

VWebService::VWebService(QWidget *parent) :
    HTabBase(parent),
    ui(new Ui::VWebService)
{
    ui->setupUi(this);

    m_strBearerToken = "69cfa069bba26b8bcfd0913501b172f8";

    m_StatusHeader.push_back("site");
    m_StatusHeader.push_back("item");
    m_StatusHeader.push_back("shop_Order");
    m_StatusHeader.push_back("sfc");
    m_StatusHeader.push_back("step_SEQUENCE");
    m_StatusHeader.push_back("operation");
    m_StatusHeader.push_back("op_DESC");
    m_StatusHeader.push_back("in_WORK");
    m_StatusHeader.push_back("status_DESCRIPTION");

    m_HistoryHeader.push_back("site");
    m_HistoryHeader.push_back("shop_Order");
    m_HistoryHeader.push_back("sfc");
    m_HistoryHeader.push_back("batch");
    m_HistoryHeader.push_back("item");
    m_HistoryHeader.push_back("item_TYPE");
    m_HistoryHeader.push_back("item_DESC");
    m_HistoryHeader.push_back("operation");
    m_HistoryHeader.push_back("op_DESC");
    m_HistoryHeader.push_back("resource");
    m_HistoryHeader.push_back("start_TIME");
    m_HistoryHeader.push_back("end_TIME");
    m_HistoryHeader.push_back("worK_MINUTE");
    m_HistoryHeader.push_back("qtY_STARTED");
    m_HistoryHeader.push_back("qtY_COMPLETED");
    m_HistoryHeader.push_back("qtY_SCRAPPED");

    m_WipHeader.push_back("WERKS"); // 工廠
    m_WipHeader.push_back("LGORT"); // 儲存地點
    m_WipHeader.push_back("MATNR"); // 物料
    //m_WipHeader.push_back("CHARG"); // 批次
    m_WipHeader.push_back("CLABS"); // 數量




    ui->edtItem->setText("0244D02138P03");


    m_pRequest=nullptr;
    m_pReply=nullptr;

    // 连接身份验证信号
    QObject::disconnect(&m_networkManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),this,SLOT(VWebService::OnCheckLogin(QNetworkReply*,QAuthenticator*)));


}

VWebService::~VWebService()
{
    delete ui;
}



void VWebService::OnInit()
{
    if(gSystem==nullptr)
        return;
    connect(&gSystem->m_WebService,SIGNAL(OnHistory()),this,SLOT(OnHistory()));
    connect(&gSystem->m_WebService,SIGNAL(OnStatus()),this,SLOT(OnStatus()));
    connect(&gSystem->m_WebService,SIGNAL(OnWips()),this,SLOT(OnWips()));
    connect(&gSystem->m_WebService,SIGNAL(OnError()),this,SLOT(OnError()));
    connect(&gSystem->m_WebService,SIGNAL(OnAutoUpdate(bool,uint)),this,SLOT(OnAutoUpdate(bool,uint)));
    connect(gSystem,SIGNAL(OnUpdateError(int)),this,SLOT(OnUpdateError(int)));

    connect(&m_tmUpdate,SIGNAL(timeout()),this,SLOT(OnUpdateCheck()));
    m_tmUpdate.start(10000);
}

void VWebService::OnShowTab(bool show)
{
    SystemData* pData;
    QString strDataTime="---";

    if(show)
    {
        strDataTime="---";
        pData=gSystem->GetSystemData("UpdateDateTime");
        if(pData!=nullptr)
            strDataTime=pData->strValue;
        ui->edtDateTime->setText(strDataTime);

        strDataTime="---";
        pData=gSystem->GetSystemData("NextUdDateTime");
        if(pData!=nullptr)
            strDataTime=pData->strValue;
        ui->edtDateTime_2->setText(strDataTime);

        double dblPitch=0;
        if(gSystem->LoadSystemData("AutoUpdate",dblPitch))
            ui->edtPitch->setText(QString("%1").arg(dblPitch));
        else
            ui->edtPitch->setText("---");
    }
}

void VWebService::on_btnStatus_clicked()
{
    ui->tableWidget->setRowCount(0);
    bool bRet=gSystem->IsRunning();
    if(!bRet)
        bRet=gSystem->m_WebService.RunSFCStatus(ui->edtItem->text());
    ui->btnERP->setEnabled(!bRet);
    ui->btnStatus->setEnabled(!bRet);
    ui->btnUpdate->setEnabled(!bRet);
    ui->btnHistory->setEnabled(!bRet);

    ui->btnLogin->setEnabled(!bRet);
}

void VWebService::on_btnHistory_clicked()
{
    ui->tableWidget->setRowCount(0);
    bool bRet=gSystem->IsRunning();
    if(!bRet)
        bRet=gSystem->m_WebService.RunSFCHistory(ui->edtItem->text());
    ui->btnERP->setEnabled(!bRet);
    ui->btnStatus->setEnabled(!bRet);
    ui->btnUpdate->setEnabled(!bRet);
    ui->btnHistory->setEnabled(!bRet);

    ui->btnLogin->setEnabled(!bRet);
}

void VWebService::on_btnERP_clicked()
{
    ui->tableWidget->setRowCount(0);
    bool bRet=gSystem->IsRunning();
    if(!bRet)
        bRet=gSystem->m_WebService.RunWip(ui->edtItem->text());
    ui->btnERP->setEnabled(!bRet);
    ui->btnStatus->setEnabled(!bRet);
    ui->btnUpdate->setEnabled(!bRet);
    ui->btnHistory->setEnabled(!bRet);

    ui->btnLogin->setEnabled(!bRet);
}



void VWebService::SetTableItem(int row, int col, QString value)
{
    QTableWidgetItem* pItem=ui->tableWidget->item(row,col);
    if(pItem!=nullptr)
        pItem->setText(value);
    else
    {
        pItem=new QTableWidgetItem(value);
        ui->tableWidget->setItem(row,col,pItem);
    }
}

void VWebService::OnHistory()
{
    std::vector<HHistoryData>::iterator itV;
    std::vector<HHistoryData> datas;
    while(!gSystem->m_WebService.CopyHistorys(datas))
    {

    }

    if(ui->tableWidget->columnCount()!=m_HistoryHeader.size())
        ui->tableWidget->setColumnCount(m_HistoryHeader.size());
    ui->tableWidget->setHorizontalHeaderLabels(m_HistoryHeader);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    int nDataSize=static_cast<int>(datas.size());
    ui->tableWidget->setRowCount(nDataSize);

    QString strValue;
    int nRow=0,nCol=0;
    for (itV=datas.begin();itV!=datas.end();itV++)
    {
        nCol=0;
        foreach (const QString& name , m_HistoryHeader)
        {
            if(name=="sfc")
                SetTableItem(nRow,nCol,itV->sfc);
            else if(name=="item")
                SetTableItem(nRow,nCol,itV->item);
            else if(name=="site")
                SetTableItem(nRow,nCol,itV->site);
            else if(name=="batch")
                SetTableItem(nRow,nCol,itV->batch);
            else if(name=="op_DESC")
                SetTableItem(nRow,nCol,itV->op_DESC);
            else if(name=="end_TIME")
                SetTableItem(nRow,nCol,itV->end_TIME);
            else if(name=="resource")
                SetTableItem(nRow,nCol,itV->resource);
            else if(name=="item_DESC")
                SetTableItem(nRow,nCol,itV->item_DESC);
            else if(name=="item_TYPE")
                SetTableItem(nRow,nCol,itV->item_TYPE);
            else if(name=="operation")
                SetTableItem(nRow,nCol,itV->operation);
            else if(name=="shop_Order")
                SetTableItem(nRow,nCol,itV->shop_Order);
            else if(name=="start_TIME")
                SetTableItem(nRow,nCol,itV->start_TIME);
            else if(name=="qtY_STARTED")
                SetTableItem(nRow,nCol,itV->qtY_STARTED);
            else if(name=="worK_MINUTE")
                SetTableItem(nRow,nCol,itV->worK_MINUTE);
            else if(name=="qtY_SCRAPPED")
                SetTableItem(nRow,nCol,itV->qtY_SCRAPPED);
            else if(name=="qtY_COMPLETED")
                SetTableItem(nRow,nCol,itV->qtY_COMPLETED);
            else
            {
                QString strTemp=name;
            }

            nCol++;
        }
        nRow++;
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();

    if(!gSystem->IsRunning())
    {
        ui->btnERP->setEnabled(true);
        ui->btnStatus->setEnabled(true);
        ui->btnUpdate->setEnabled(true);
        ui->btnHistory->setEnabled(true);

        ui->btnLogin->setEnabled(true);
    }

}

void VWebService::OnStatus()
{
    std::vector<HStatusData>::iterator itV;
    std::vector<HStatusData> datas;
    while(!gSystem->m_WebService.CopyStatuses(datas))
    {

    }

    if(ui->tableWidget->columnCount()!=m_StatusHeader.size())
        ui->tableWidget->setColumnCount(m_StatusHeader.size());
    ui->tableWidget->setHorizontalHeaderLabels(m_StatusHeader);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    int nDataSize=static_cast<int>(datas.size());
    ui->tableWidget->setRowCount(nDataSize);

    QString strValue;
    int nRow=0,nCol=0;
    for (itV=datas.begin();itV!=datas.end();itV++)
    {
        nCol=0;
        foreach (const QString& name , m_StatusHeader)
        {
            if(name=="sfc")
                SetTableItem(nRow,nCol,itV->sfc);
            else if(name=="item")
                SetTableItem(nRow,nCol,itV->item);
            else if(name=="site")
                SetTableItem(nRow,nCol,itV->site);
            else if(name=="in_WORK")
                SetTableItem(nRow,nCol,itV->in_WORK);
            else if(name=="op_DESC")
                SetTableItem(nRow,nCol,itV->op_DESC);
            else if(name=="operation")
                SetTableItem(nRow,nCol,itV->operation);
            else if(name=="shop_Order")
                SetTableItem(nRow,nCol,itV->shop_Order);
            else if(name=="step_SEQUENCE")
                SetTableItem(nRow,nCol,itV->step_SEQUENCE);
            else if(name=="status_DESCRIPTION")
                SetTableItem(nRow,nCol,itV->status_DESCRIPTION);
            else
            {
                QString strTemp=name;
            }

            nCol++;
        }
        nRow++;
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();

    if(!gSystem->IsRunning())
    {
        ui->btnERP->setEnabled(true);
        ui->btnStatus->setEnabled(true);
        ui->btnUpdate->setEnabled(true);
        ui->btnHistory->setEnabled(true);

        ui->btnLogin->setEnabled(true);
    }
}

void VWebService::OnWips()
{
    std::vector<HWipData>::iterator itV;
    std::vector<HWipData> datas;
    while(!gSystem->m_WebService.CopyWips(datas))
    {

    }

    if(ui->tableWidget->columnCount()!=m_WipHeader.size())
        ui->tableWidget->setColumnCount(m_WipHeader.size());
    ui->tableWidget->setHorizontalHeaderLabels(m_WipHeader);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    int nDataSize=static_cast<int>(datas.size());
    ui->tableWidget->setRowCount(nDataSize);

    QString strValue;
    int nRow=0,nCol=0;
    for (itV=datas.begin();itV!=datas.end();itV++)
    {
        nCol=0;
        foreach (const QString& name , m_WipHeader)
        {
            if(name=="WERKS")   // 工廠
                SetTableItem(nRow,nCol,itV->WERKS);
            else if(name=="LGORT")  // 儲存地點
                SetTableItem(nRow,nCol,itV->LGORT);
            else if(name=="MATNR")  // 物料
                SetTableItem(nRow,nCol,itV->MATNR);
            else if(name=="CLABS")    // 數量
                SetTableItem(nRow,nCol,itV->CLAGS);
            else
            {
                QString strTemp=name;
            }

            nCol++;
        }
        nRow++;
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();

    if(!gSystem->IsRunning())
    {
        ui->btnERP->setEnabled(true);
        ui->btnStatus->setEnabled(true);
        ui->btnUpdate->setEnabled(true);
        ui->btnHistory->setEnabled(true);

        ui->btnLogin->setEnabled(true);
    }
}

void VWebService::OnError()
{
    if(!gSystem->IsRunning())
    {
        ui->btnERP->setEnabled(true);
        ui->btnStatus->setEnabled(true);
        ui->btnUpdate->setEnabled(true);
        ui->btnHistory->setEnabled(true);

        ui->btnLogin->setEnabled(true);
    }
}

void VWebService::OnUpdateError(int error)
{
    if(!gSystem->IsRunning())
    {
        ui->btnERP->setEnabled(true);
        ui->btnStatus->setEnabled(true);
        ui->btnUpdate->setEnabled(true);
        ui->btnHistory->setEnabled(true);
        ui->btnLogin->setEnabled(true);

        SystemData* pData;
        QString strDataTime="---";
        pData=gSystem->GetSystemData("UpdateDateTime");
        if(pData!=nullptr)
            strDataTime=pData->strValue;
        ui->edtDateTime->setText(strDataTime);
        ui->progressBar->setValue(ui->progressBar->maximum());

        strDataTime="---";
        pData=gSystem->GetSystemData("NextUdDateTime");
        if(pData!=nullptr)
            strDataTime=pData->strValue;
        ui->edtDateTime_2->setText(strDataTime);
    }
    else
        ui->progressBar->setValue(error);


    QString strStep=QString("Step:%1(%2/%3)").arg(gStep).arg(error).arg(ui->progressBar->maximum());
    ui->lblStep->setText(strStep);
}



void VWebService::SetRequire()
{
    QString strURL= "http://erpqasap.intai-corp.com:8000/sap/bc/srt/wsdl/flv_10002A111AD1/bndg_url/sap/bc/srt/rfc/sap/zmm036f1/288/zmm036f1/zmm036f1?sap-client=288";

    QUrl url(strURL);
    QNetworkRequest request(url);

    QString username = "A110110";
    QString password = "sap1234";

    QByteArray authHeader = "Basic " + QByteArray(QString("%1:%2").arg(username).arg(password).toLatin1()).toBase64();
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
        qDebug() << "Response:" << jsonString;
    }
    else
    {
        qDebug() << "Request error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    //<wsdl:service name="ZMM036F1">
    //<soap:address location="https://erpqasap.intai-corp.com:8100/sap/bc/srt/rfc/sap/zmm036f1/288/zmm036f1/zmm036f1"/>
    QDomDocument  doc;
    doc.setContent(jsonString);

    // 解析 WSDL
    QDomElement root = doc.documentElement();

    // 獲取命名空間
    QString namespaceUri = root.namespaceURI();

    // 獲取 Service 元素
    QDomNodeList serviceNodes = root.elementsByTagName("xsd:complexType");
    if (serviceNodes.isEmpty())
    {
        qDebug() << "WSDL 檔案中找不到 Service 元素";
        return;
    }

    int nCount=serviceNodes.size();
    for(int i=0;i<nCount;i++)
    {
        QDomElement serviceElement = serviceNodes.at(i).toElement();
        QString serviceName = serviceElement.attribute("name");
        if(serviceName.size()>0)
        {
            qDebug() << "命名空間: " << namespaceUri;
            qDebug() << "Service 名稱: " << serviceName;
        }
    }



    QString strSoap="https://erpqasap.intai-corp.com:8100/sap/bc/srt/rfc/sap/zmm036f1/288/zmm036f1/zmm036f1";
    // 建立 SOAP 請求內容
    QString soapXml = "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:ns=\"namespace-uri\">"
                      "<soap:Header></soap:Header>"
                      "<soap:Body>"
                          "<ns:MethodName>"
                              "<ns:param1>value1</ns:param1>"
                              "<ns:param2>value2</ns:param2>"
                          "</ns:MethodName>"
                      "</soap:Body>"
                      "</soap:Envelope>";
    QString soapXml2 =  "<soapenv:Envelope"
                            "xmlns:soapenv=http://schemas.xmlsoap.org/soap/envelope/"
                            "xmlns:xsd=http://www.w3.org/2001/XMLSchema"
                            "xmlns:xsi=http://www.w3.org/2001/XMLSchema-instance"
                            "<soapenv:Body>"
                                "<req:echo xmlns:req=http://localhost:8080/wxyc/login.do"
                                    "<req:category>classifieds</req:category>"
                                "</req:echo>"
                            "</soapenv:Body>"
                        "</soapenv:Envelope>";

    QString soapXml3 =  "<?xml version='1.0' encoding='utf-8'?>";
                        "<soap:Envelope xmlns:soap=http://schemas.xmlsoap.org/soap/envelope/ xmlns:xsi=http://www.w3.org/2001/XMLSchema-instance xmlns:xsd=http://www.w3.org/2001/XMLSchema>";
                        "<soap:Body>"
                            "<getSupportCity xmlns=http://WebXml.com.cn/>"
                                "<byProvinceName>广东</byProvinceName>"
                            "</getSupportCity>"
                        "</soap:Body>"
                        "</soap:Envelope>";


}


void VWebService::on_btnERP_clicked2()
{
    QString strSoap="http://erpqasap.intai-corp.com:8000/sap/bc/srt/rfc/sap/zmm036f1/288/zmm036f1/zmm036f1";
    // 設定 Web service 的 URL
    QUrl serviceUrl(strSoap);

    // 建立 SOAP 訊息
    QDomDocument soapMessage;
    // 建立 SOAP 訊息

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
    matnr1.appendChild(soapMessage.createTextNode("02480090003"));
    item1.appendChild(matnr1);

    // 建立 <CHARG> 元素，並設定內容
    QDomElement charg1 = soapMessage.createElement("CHARG");
    item1.appendChild(charg1);

    // 建立 <LGORT> 元素，並設定內容
    QDomElement lgort1 = soapMessage.createElement("LGORT");
    lgort1.appendChild(soapMessage.createTextNode("5002"));
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
    qDebug() << "Request Data:" << requestData;

    // 建立 QNetworkRequest
    if(m_pRequest!=nullptr)
        delete m_pRequest;
    m_pRequest=new QNetworkRequest(serviceUrl);
    m_pRequest->setHeader(QNetworkRequest::ContentTypeHeader, "text/xml;charset=UTF-8");
    m_pRequest->setHeader(QNetworkRequest::ContentLengthHeader, QString::number(requestData.size()));

    // 添加帳號和密碼的身份驗證
    QString username = "A110110";
    QString password = "sap1234";
    QString concatenated = username + ":" + password;
    QByteArray data = concatenated.toUtf8().toBase64();
    QString headerData = "Basic " + data;
    m_pRequest->setRawHeader("Authorization", headerData.toUtf8());

    // 發送 SOAP 請求
    if(m_pReply!=nullptr)
    {
        QObject::disconnect(m_pReply,SIGNAL(finished),this,SLOT(OnFinished));
        delete m_pReply;
        m_pReply=nullptr;
    }
    m_pReply = m_networkManager.post(*m_pRequest, requestData);
    QObject::connect(m_pReply,SIGNAL(finished),this,SLOT(OnFinished));

    // ... 省略處理回應的程式碼 ...
    // 監聽請求完成事件

    QEventLoop loop;
   QObject::connect(m_pReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
   loop.exec();
    /*
    // 檢查回應狀態
    if (m_pReply->error() == QNetworkReply::NoError) {
        // 讀取回應的內容
        QByteArray responseData = m_pReply->readAll();

        // 在此處理回應的 SOAP 資料
        // ...

        // 輸出回應的內容
        qDebug() << responseData;
    } else {
        // 在此處理錯誤情況
        qDebug() << "Error: " << m_pReply->errorString();
    }
    m_pReply->deleteLater();
    delete m_pReply;
    m_pReply=nullptr;
    */
}


void VWebService::OnCheckLogin(QNetworkReply *, QAuthenticator *authenticator)
{
    QString username = "A110110"; // 替换为实际的用户名
    QString password = "sap1234"; // 替换为实际的密码
    authenticator->setUser(username);
    authenticator->setPassword(password);
}

void VWebService::OnFinished()
{
    if(m_pReply==nullptr)
        return;
    if (m_pReply->error() == QNetworkReply::NoError)
    {
        // 读取响应内容
        QByteArray response = m_pReply->readAll();
        qDebug() << "WSDL Response:" << response;
        // 在这里解析 WSDL 并执行相关操作
    }
    else
    {
        qDebug() << "Error: " << m_pReply->errorString();
    }

    m_pReply->deleteLater();
    if(m_pReply!=nullptr)
    {
        QObject::disconnect(m_pReply,SIGNAL(finished),this,SLOT(OnFinished));
        delete m_pReply;
        m_pReply=nullptr;
    }
}


void VWebService::OnAutoUpdate(bool OK)
{
    ui->btnERP->setEnabled(!OK);
    ui->btnStatus->setEnabled(!OK);
    ui->btnUpdate->setEnabled(!OK);
    ui->btnHistory->setEnabled(!OK);
    ui->btnSetPitch->setEnabled(!OK);
    ui->btnLogin->setEnabled(!OK);
    ui->btnLoadProduct->setEnabled(!OK);
    ui->btnLoadProductEx->setEnabled(!OK);


    ui->progressBar->setMaximum(gUpdateCount);
}

void VWebService::on_btnUpdate_clicked()
{
    if(!gSystem->RunUpdateDataBase())
        OnAutoUpdate(false);
}

void VWebService::on_btnLogin_clicked()
{
    QString strPws=ui->edtPwd->text();
    if(strPws.size()<=0)
        return;

    SystemData* pData=gSystem->GetSystemData("password");
    bool bEnable=false;
    if(pData!=nullptr)
        bEnable=(strPws==pData->strValue);
    else
        bEnable=(strPws=="1234");
    emit OnUserLogin(bEnable);
    ui->edtPwd->setText("");

    ui->btnERP->setEnabled(bEnable);
    ui->btnStatus->setEnabled(bEnable);
    ui->btnHistory->setEnabled(bEnable);
    ui->btnUpdate->setEnabled(bEnable);
    ui->btnSetPitch->setEnabled(bEnable);
    ui->btnLogin->setEnabled(true);

    ui->btnLoadProduct->setEnabled(bEnable);
    ui->btnLoadProductEx->setEnabled(bEnable);

}

void VWebService::on_btnSetPitch_clicked()
{
    bool bResetTimer=false;
    double dblPitch=ui->edtPitch->text().toDouble();
    if(dblPitch>=0 && dblPitch<24)
        bResetTimer=gSystem->SaveSystemData("AutoUpdate",dblPitch);

    if(gSystem->LoadSystemData("AutoUpdate",dblPitch))
        ui->edtPitch->setText(QString("%1").arg(dblPitch));
    else
        ui->edtPitch->setText("---");

    if(bResetTimer)
    {
        gSystem->ResetUpdateTime();
        OnShowTab(true);
    }
}

void VWebService::OnUpdateCheck()
{
    if(gSystem->IsReady2Update() && !gSystem->isRunning())
    {
        if(!gSystem->RunUpdateDataBase())
            OnAutoUpdate(false);
    }
}

void VWebService::on_btnLoadProduct_clicked()
{
    QString strPath="D:/";
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        strPath,
        tr("Excel Files (*.XLSX *.xlsx);;All Files (*)"));
    if (fileName.isEmpty())
        return;


    if(!gSystem->LoadFile2CreateProduct(fileName))
        QMessageBox::warning(nullptr,tr("Warm"),tr("Import Failed!"));
    else
        QMessageBox::warning(nullptr,tr("Info"),tr("Import Sucess!"));
}



void VWebService::on_edtPwd_returnPressed()
{
    on_btnLogin_clicked();
}

void VWebService::on_btnLoadProductEx_clicked()
{
    QString strPath="D:/";
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        strPath,
        tr("Excel Files (*.XLSX *.xlsx);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    QString selectedDirectory = QFileDialog::getExistingDirectory(
            nullptr,
            "Select a directory to save the file",
            QDir::homePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
    if (selectedDirectory.isEmpty())
    {
           QMessageBox::warning(nullptr,tr("Warm"),tr("Export Failed!"));
           return;
    }
    if(!gSystem->LoadFile2CreateProductEx(fileName,selectedDirectory))
        QMessageBox::warning(nullptr,tr("Warm"),tr("Export Failed!"));
    else
        QMessageBox::warning(nullptr,tr("Info"),tr("Export Sucess!"));

}
