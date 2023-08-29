#ifndef VBACKCALINFO_H
#define VBACKCALINFO_H

#include <QWidget>
#include "htabbase.h"
#include "hproduct.h"

namespace Ui {
class vBackCalInfo;
}





class vBackCalInfo : public HTabBase
{
    Q_OBJECT

public:
    explicit vBackCalInfo(QWidget *parent = nullptr);
    ~vBackCalInfo();

    virtual void OnInit();
    virtual void OnShowTab(bool);

private:
    void RelistProducts();
    void RelistPartInfos(QString strP);
    void RelistPartInfos2(QString strP);
    void ReCheckPoints();

signals:


private slots:
    void on_btnSelect_clicked();
    void on_cmdPart_activated(int index);
    void on_btnLogin_clicked();

private:
    Ui::vBackCalInfo *ui;


    std::map<QString,std::list<ProcessInfo>> m_mapProcess;
    //std::map<QString,std::list<ProcessInfo>> m_mapPartProcess;

    std::map<QString,QString> m_mapProducts;
    std::map<QString,std::vector<QString>> m_mapParts;


    QColor m_colorWip,m_colorTarget;

};

#endif // VBACKCALINFO_H
