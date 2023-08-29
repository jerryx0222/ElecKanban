#ifndef VWEBSERVICE_H
#define VWEBSERVICE_H

#include <QWidget>
#include "htabbase.h"
#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>


namespace Ui {
class VWebService;
}

class VWebService : public HTabBase
{
    Q_OBJECT

public:
    explicit VWebService(QWidget *parent = nullptr);
    ~VWebService();

    virtual void OnInit();
    virtual void OnShowTab(bool);

signals:
    void OnUserLogin(bool);

private:
    QString m_strBearerToken;
    QNetworkAccessManager m_manager;
    QStringList m_HistoryHeader,m_StatusHeader,m_WipHeader;



private:
    //bool GetSFC(QString strLink,QJsonArray &jsonArray);
    //void RelistDatas(QStringList& names,QJsonArray &jsonArray);
    void SetTableItem(int row,int col,QString value);

private slots:
    void OnAutoUpdate(bool);
    void  OnHistory();
    void  OnStatus();
    void  OnWips();
    void  OnError();
    void OnUpdateError(int);
    void on_btnStatus_clicked();
    void on_btnHistory_clicked();
    void on_btnERP_clicked2();
    void on_btnERP_clicked();
    void OnCheckLogin(QNetworkReply*, QAuthenticator*);
    void OnFinished();
    void SetRequire();
    void on_btnUpdate_clicked();
    void on_btnLogin_clicked();
    void on_btnSetPitch_clicked();
    void OnUpdateCheck();
    void on_btnLoadProduct_clicked();
    void on_edtPwd_returnPressed();

    void on_btnLoadProductEx_clicked();

private:
    Ui::VWebService *ui;


     QNetworkAccessManager  m_networkManager;
     QNetworkRequest        *m_pRequest;
     QNetworkReply          *m_pReply;



     QTimer                 m_tmUpdate;
};

#endif // VWEBSERVICE_H
