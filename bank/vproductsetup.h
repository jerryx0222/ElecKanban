#ifndef VPRODUCTSETUP_H
#define VPRODUCTSETUP_H

#include <QWidget>
#include <htabbase.h>

namespace Ui {
class vProductSetup;
}

class vProductSetup : public HTabBase
{
    Q_OBJECT

public:
    explicit vProductSetup(QWidget *parent = nullptr);
    ~vProductSetup();

     virtual void OnInit();

private slots:
    void on_btnPLoad_clicked();
    void on_btnPNew_clicked();
    void on_btnPSave_clicked();
    void on_btnP2New_clicked();
    void on_btnP2Del_clicked();
    void on_btnPDel_clicked();
    void on_btnP2Load_clicked();
    void on_btnP2Save_clicked();
    void on_btnP3Load_clicked();
    void on_btnP3New_clicked();
    void on_btnP3Save_clicked();
    void on_btnP3Del_clicked();

private:
    void CreateTable();
    void InsertProducts();
    void InsertProcess();
    void InsertPart();

private:
    Ui::vProductSetup *ui;
};

#endif // VPRODUCTSETUP_H
