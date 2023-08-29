#ifndef DLGTYPESELECT_H
#define DLGTYPESELECT_H

#include <QDialog>
#include <QTimer>
#include <map>

namespace Ui {
class dlgTypeSelect;
}

class dlgTypeSelect : public QDialog
{
    Q_OBJECT

public:
    explicit dlgTypeSelect(QString* pTypeSelect,QWidget *parent = nullptr);
    ~dlgTypeSelect();

    QTimer      m_timer;
    QString     *m_pStrTypeSelect;

    std::map<QString,QString> m_datas;

public slots:
    void OnInit();

private slots:
    void on_btnHistoryLoad_2_clicked();
    void on_btnNew_clicked();
    void on_btnSave_clicked();
    void on_btnDel_clicked();
    void on_btnSelect_clicked();
    void on_tbType_cellDoubleClicked(int row, int column);

private:
    Ui::dlgTypeSelect *ui;

private:
    void ReloadTypes();
};

#endif // DLGTYPESELECT_H
