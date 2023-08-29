#ifndef VBACKTABLE_H
#define VBACKTABLE_H

#include <QWidget>
#include "htabbase.h"
#include "hproduct.h"
#include <QTableWidget>

namespace Ui {
class VBackTable;
}

class VBackTable : public HTabBase
{
    Q_OBJECT

public:
    explicit VBackTable(QWidget *parent = nullptr);
    ~VBackTable();

    virtual void OnInit();
    virtual void OnShowTab(bool);

private slots:
    void on_rdoWip1_clicked();
    void on_rdoInfo1_clicked();
    void on_rdoWip2_clicked();
    void on_rdoInfo2_clicked();
    void on_btnSelect1_clicked();
    void on_btnSelect2_clicked();
    void on_btnExport1_clicked();
    void on_btnExport2_clicked();

private:
    void SwitchRadio(int id,bool wip);
    void DisplayTable(int id);
    void RelistProducts(QTableWidget* pTable,int index,QString id,QString name,bool bWip);
    void ExportExcel(int id);
    void DisplayValues(QTableWidget* pTable,bool bWip);

private:
    Ui::VBackTable *ui;

    std::map<int,std::map<int, ProcessTable>>   m_mapProTables[2];
};

#endif // VBACKTABLE_H
