#ifndef DLGDATESELECT_H
#define DLGDATESELECT_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class dlgDateSelect;
}

class dlgDateSelect : public QDialog
{
    Q_OBJECT

public:
    explicit dlgDateSelect(QString *strSelect,int* row,int* col,QWidget *parent = nullptr);
    ~dlgDateSelect();

    QString *m_pStrSelect;
    int *m_pRow,*m_pCol;
private:
    Ui::dlgDateSelect *ui;



};

#endif // DLGDATESELECT_H
