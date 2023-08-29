#ifndef VPARTPROCESS_H
#define VPARTPROCESS_H

#include <QWidget>
#include "htabbase.h"
#include <QTreeWidgetItem>
#include "hproduct.h"

namespace Ui {
class vPartProcess;
}

class vPartProcess : public HTabBase
{
    Q_OBJECT

public:
    explicit vPartProcess(QWidget *parent = nullptr);
    ~vPartProcess();

     virtual void OnInit();
     virtual void OnShowTab(bool show);

private slots:
    void on_tbProducts_cellClicked(int row, int column);
    void on_trProduct_itemClicked(QTreeWidgetItem *item, int column);
    void on_btnPlot_clicked();
    void on_btnInsert_clicked();
    void on_btnRemove_clicked();
    void on_btnSave_clicked();
    void on_btnRelist_clicked();
    void on_btnTypeSelect_clicked();
    void on_trProduct_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_btnInsert2_clicked();
    void on_btnRemove2_clicked();
    void on_btnSave2_clicked();
    void on_btnRelist2_clicked();
    void on_tbProcess_cellDoubleClicked(int row, int column);

    void on_btnProcessImport_clicked();

private:
    void CreateTable();
    void RelistProducts();
    void RelistChilds();

    void RelistProcess(QString strProductID,QString strPartID);
    void RelistProcess(QString strProductID);

    void RelistProcess();
    void RelistProcess(std::list<ProcessInfo>&);

private:
    Ui::vPartProcess *ui;

    Product*    m_selProduct;
    Part*       m_selPart;

    std::list<ProcessInfo> m_Processes;
};

#endif // VPARTPROCESS_H
