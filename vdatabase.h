#ifndef VDATABASE_H
#define VDATABASE_H

#include <QWidget>
#include "htabbase.h"
#include <QTableWidget>

namespace Ui {
class vDataBase;
}


enum DBID
{
    dbProduct,
    dbShipment,
    dbPart,
    dbProductLink,
    dbProcess,
    dbProcessLink,
};


class vDataBase : public HTabBase
{
    Q_OBJECT

public:
    explicit vDataBase(QWidget *parent = nullptr);
    ~vDataBase();
    virtual void OnInit();

private:
    void CreateProductDB();
    void CreateShipmentDB();
    void CreatePartDB();
    void CreateProductLinkDB();
    void CreateProcessDB();
    void CreateProcessLinkDB();
    void ReCreateTable(QStringList&);
    void RelistDatas(QString,std::map<QString,QString> &outDatas);
    void RelistProducts();
    void SaveProducts();


private slots:
    void on_cmbDB_activated(int index);
    void on_btnLoad_clicked();
    void on_btnNew_clicked();
    void on_btnSave_clicked();
    void on_btnDelete_clicked();
    void on_tbDataBase_cellClicked(int row, int column);
    void buttonSelect();

private:
    Ui::vDataBase *ui;
};



#endif // VDATABASE_H
