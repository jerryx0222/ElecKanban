#ifndef VOUTPUTER_H
#define VOUTPUTER_H

#include <QWidget>
#include "htabbase.h"
#include <QTableWidget>
#include <QLabel>

namespace Ui {
class vOutputerEx;
}

class vOutputerEx : public HTabBase
{
    Q_OBJECT

public:
    explicit vOutputerEx(QWidget *parent = nullptr);
    ~vOutputerEx();

    virtual void OnInit();
    virtual void OnShowTab(bool);

private:
    void RelistProducts();
    void RelistProductsByIndex(int);
    void RelistShips(size_t);
    void RelistShipsFromDB(size_t);

private slots:
    void on_btnSelect_clicked();
    void on_btnUpdate_clicked();
    void on_btnExport_clicked();
    void on_VBar_valueChanged(int value);

private:
    Ui::vOutputerEx *ui;

    std::map<QString,QString> m_Products;
    std::vector<QTableWidget*> m_vTables;
    std::vector<QLabel*> m_vLabels;
    QStringList  m_lstDates;
};

#endif // VOUTPUTER_H
