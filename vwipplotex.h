#ifndef VWIPPLOTEX_H
#define VWIPPLOTEX_H

#include <QWidget>
#include "htabbase.h"
#include "hproduct.h"
#include "qcustomplot.h"

namespace Ui {
class vWipPlotEx;
}

class vWipPlotEx : public HTabBase
{
    Q_OBJECT

public:
    explicit vWipPlotEx(QWidget *parent = nullptr);
    ~vWipPlotEx();

    virtual void OnInit();
    virtual void OnShowTab(bool);

private slots:
    void on_btnSelect_clicked();

    void on_btnSave_clicked();

    void on_chkGroup_clicked();

    void on_btnExport_clicked();

private:
    void RelistProducts();
    void DisplayTable(Product*);
    void DisplayProductTables();
    void DisplayTable(Product* pP1,Part*);
    void DisplayPartTables();
    void DisplayTable(std::list<ProcessInfo>& infos);
    void setupBarChart(int nMax, QVector<double> &v1, QVector<double> &v2, QVector<double> &v3);

private:
    Ui::vWipPlotEx *ui;
    QColor m_Color[3];
    QStringList m_ProcessIDs;
    QCPBars* m_pBarWip;
    QCPBars* m_pBarTarget;
    QCPBars* m_pBarSum;
    QCPTextElement* m_pElecTitle;

    Product*    m_pProduct;
    Part*       m_pPart;
    std::vector<Product*>   m_pVProducts;
    std::vector<Part*>   m_pVParts;
};

#endif // VWIPPLOTEX_H
