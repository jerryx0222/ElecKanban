#ifndef VSHIPMENT_H
#define VSHIPMENT_H

#include <QWidget>
#include "htabbase.h"
#include <QTableWidgetItem>
#include <QListWidgetItem>

namespace Ui {
class vShipment;
}

class vShipment : public HTabBase
{
    Q_OBJECT

public:
    explicit vShipment(QWidget *parent = nullptr);
    ~vShipment();

    virtual void OnInit();
    virtual void OnShowTab(bool);

private:
    void CreateTable();
    void RelistDates();
    void RelistProducts();
    void RelistShips();
    void RelistProcess(QString);


private slots:
    void on_btnRelist_clicked();
    void on_lstDate_itemClicked(QListWidgetItem *item);
    void on_btnShipRelist_clicked();
    void on_btnShiipSave_clicked();
    void on_btnShiipLoad_clicked();
    //void on_btnInsertDate_clicked();
    void on_btnRemoveDate_clicked();
    void on_btnSave_clicked();
    void on_btnRemove_clicked();
    void on_btnInsert_clicked();
    void on_tbProducts_cellClicked(int row, int column);
    void on_btnStatusLoad_clicked();
    void on_btnHistoryLoad_clicked();
    void on_tbShip2_cellClicked(int row, int column);
    void on_btnTypeSelect_clicked();
    void on_btnTest_clicked();
    void on_btnT1_clicked();
    void on_btnLogin_clicked();

private:
    Ui::vShipment *ui;
    QStringList m_listDates;
};

#endif // VSHIPMENT_H
