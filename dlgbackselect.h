#ifndef DLGBACKSELECT_H
#define DLGBACKSELECT_H

#include <QDialog>
#include <QTableWidgetItem>
#include <QTimer>

namespace Ui {
class dlgBackSelect;
}

class dlgBackSelect : public QDialog
{
    Q_OBJECT

public:
    explicit dlgBackSelect(QString *strType,QString *strProduct,std::vector<QString>* vProducts,QWidget *parent = nullptr);
    ~dlgBackSelect();

protected slots:
    void on_btnCan_clicked();
    void on_btnOK_clicked();
    void on_tbProduct_itemDoubleClicked(QTableWidgetItem *item);
    void on_cmbType_activated(int index);
    void OnReloadTypes();

protected:
    void ReloadTypes();
    virtual void RelistProducts();
    void ReCheckPoints(QString);

    QString *m_pStrType;
    QString *m_pStrProduct;
    std::vector<QString>* m_pVProducts;
    std::map<QString,std::map<QString,QString>> m_mapTotalProducts;
    QTimer  m_Timer;

protected:
    Ui::dlgBackSelect *ui;
};

/**********************************************************************************************************/
class dlgBackSelectEx : public dlgBackSelect
{
    Q_OBJECT

public:
    explicit dlgBackSelectEx(QString *strType,QString *strProduct,std::vector<QString>* vProducts,QWidget *parent = nullptr);
    ~dlgBackSelectEx();

protected:
     virtual void RelistProducts();
};


#endif // DLGBACKSELECT_H
