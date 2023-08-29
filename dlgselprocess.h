#ifndef DLGSELPROCESS_H
#define DLGSELPROCESS_H

#include <QDialog>
#include <map>

namespace Ui {
class dlgSelProcess;
}

class dlgSelProcess : public QDialog
{
    Q_OBJECT

public:
    explicit dlgSelProcess(QString* pID,QString* pName,QWidget *parent = nullptr);
    ~dlgSelProcess();

    QString strProcessID;

private slots:
    void on_buttonBox_accepted();
    void on_btnSearch_clicked();
    void on_btnSave_clicked();
    void on_tbProcess_cellDoubleClicked(int row, int column);
    void on_tbProcess_cellClicked(int row, int column);
    void on_btnImport_clicked();

private:
    void CreateTable();
    void RelistProcess(QString strKey);
    void RelistProcess();

    Ui::dlgSelProcess *ui;
    std::map<QString,QString> m_mapProcess;

    QString* m_pStrSelID;
    QString* m_pStrSelName;
};

#endif // DLGSELPROCESS_H
