#ifndef VWIPPLOT_H
#define VWIPPLOT_H

#include <QWidget>
#include "htabbase.h"
#include "hproduct.h"
#include "qcustomplot.h"

namespace Ui {
class vWipPlot;
}

class vWipPlot : public HTabBase
{
    Q_OBJECT

public:
    explicit vWipPlot(QWidget *parent = nullptr);
    ~vWipPlot();

    virtual void OnInit();
    virtual void OnShowTab(bool);

private:
    void RelistProducts();
    void RelistParts();
    void DisplayTable(Product*);
    void DisplayTable(Product* pP1,Part*);
    void DisplayTable(std::list<ProcessInfo>& infos);
    void setupBarChart(int nMax,QVector<double>& v1,QVector<double>& v2,QVector<double>& v3);
    void RelistParts2ComboBox();

private slots:
    void on_btnSelect_clicked();
    void on_cmdPart_activated(int index);
    void on_cmdProducts_activated(int index);
    void on_btnSave_clicked();
    void OnReflash();

private:
    Ui::vWipPlot *ui;
    QColor  m_Color[3];
    QStringList m_ProcessIDs;

    std::map<QString,QString> m_mapProducts;
    std::map<QString,QString> m_mapParts;
    Product* m_pProduct;
    Part* m_pPart;

    //QCPLayoutElement* m_pElecTitle;
    QCPTextElement* m_pElecTitle;

    QCPBars* m_pBarWip;
    QCPBars* m_pBarTarget;
    QCPBars* m_pBarSum;

    QTimer      m_timer;
};

#endif // VWIPPLOT_H
