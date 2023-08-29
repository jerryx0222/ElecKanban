#include "dlgdateselect.h"
#include "ui_dlgdateselect.h"

dlgDateSelect::dlgDateSelect(QString *strSelect,int* row,int* col,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgDateSelect)
{
    ui->setupUi(this);
    m_pStrSelect=strSelect;
    m_pRow=row;
    m_pCol=col;


}

dlgDateSelect::~dlgDateSelect()
{
    delete ui;
}


