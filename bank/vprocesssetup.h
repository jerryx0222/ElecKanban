#ifndef VPROCESSSETUP_H
#define VPROCESSSETUP_H

#include <QWidget>
#include <QPushButton>
#include <htabbase.h>
#include "hproduct.h"

namespace Ui {
class vProcessSetup;
}

class vProcessSetup : public HTabBase
{
    Q_OBJECT

public:
    explicit vProcessSetup(QWidget *parent = nullptr);
    ~vProcessSetup();

     virtual void OnInit();

    void CreateTable();
    void InsertProducts();
    void InsertProcess();


private:
    void RelistProduct(QString);
    void DisplayProductInfo();

private:
    std::map<QString,QString> m_mapProducts;
    int         m_nInsertIndex;
    QPushButton *m_pButton;

private slots:
    void on_btnSearch_clicked();
    void on_tbFinalProduct_cellClicked(int row, int column);
    void on_btnNewP_clicked();
    void OnBtnSelectProcess();
    void on_btnSave_clicked();
    void on_btnDelete_clicked();
    void on_btnAdd_clicked();

private:
    Ui::vProcessSetup *ui;
};

#endif // VPROCESSSETUP_H
