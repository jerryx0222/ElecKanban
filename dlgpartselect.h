#ifndef DLGPARTSELECT_H
#define DLGPARTSELECT_H

#include <QDialog>
#include <QTimer>
#include "hproduct.h"

namespace Ui {
class dlgPartSelect;
}

class dlgPartSelect : public QDialog
{
    Q_OBJECT

public:
    explicit dlgPartSelect(QString,QString*,QWidget *parent = nullptr);
    ~dlgPartSelect();

private slots:
    void on_btnNew_clicked();
    void on_btnSearch_clicked();
    void on_btnOK_clicked();
    void on_btnCancel_clicked();
    void OnInit();
    void on_tbPart_cellClicked(int row, int column);
    void on_btnSave_clicked();
    void on_btnDel_clicked();

private:
    void DisplayPart(Part*);
    Part* CreatePart();

private:
    Ui::dlgPartSelect *ui;
    QTimer  m_Timer;
    QString *m_pSelectPart;
    QString m_strType;
    std::map<QString,QString> m_mapParts;
};

#endif // DLGPARTSELECT_H
