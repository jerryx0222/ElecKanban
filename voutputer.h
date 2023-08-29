#ifndef VOUTPUTER_H
#define VOUTPUTER_H

#include <QWidget>
#include "htabbase.h"
#include <QTableWidget>
#include <QLabel>

namespace Ui {
class vOutputer;
}

class vOutputer : public HTabBase
{
    Q_OBJECT

public:
    explicit vOutputer(QWidget *parent = nullptr);
    ~vOutputer();

    virtual void OnInit();
    virtual void OnShowTab(bool);

private:
    void RelistProducts();
    void RelistShips(size_t);
    void RelistShipsFromDB(size_t);

private slots:
    void on_btnSelect_clicked();

private:
    Ui::vOutputer *ui;

    std::map<QString,QString> m_Products;
    std::vector<QTableWidget*> m_vTables;
    std::vector<QLabel*> m_vLabels;
    QStringList  m_lstDates;
};

#endif // VOUTPUTER_H
