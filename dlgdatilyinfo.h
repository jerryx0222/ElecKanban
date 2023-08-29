#ifndef DLGDATILYINFO_H
#define DLGDATILYINFO_H

#include <QDialog>
#include "hproduct.h"
#include "qcustomplot.h"

namespace Ui {
class dlgDatilyInfo;
}

class dlgDatilyInfo : public QDialog
{
    Q_OBJECT

public:
    explicit dlgDatilyInfo(QWidget *parent = nullptr);
    ~dlgDatilyInfo();

    Product* m_pProduct;
    Part* m_pPart;

private:
    Ui::dlgDatilyInfo *ui;
    QTimer      m_timer;
    QStringList m_ProcessIDs;
    QCPLayoutElement* m_pElecTitle;

    QCPBars* m_pBarWip;
    QCPBars* m_pBarTarget;
    QCPBars* m_pBarSum;

    QColor  m_Color[3];

public slots:
    void OnInit();


private slots:
    void on_btnSave_clicked();

private:
    void DisplayTable(Product*);
    void DisplayTable(Product* pP1,Part*);
    void DisplayTable(std::list<ProcessInfo>& infos);
    //void setupBarChartDemo(int nMax,QVector<double>& v1,QVector<double>& v2,QVector<double>& v3);
    void setupBarChart(int nMax,QVector<double>& v1,QVector<double>& v2,QVector<double>& v3);
};

#endif // DLGDATILYINFO_H
