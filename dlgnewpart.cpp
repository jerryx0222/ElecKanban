#include "dlgnewpart.h"
#include "ui_dlgnewpart.h"

dlgNewPart::dlgNewPart(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::dlgNewPart)
{
    ui->setupUi(this);
}

dlgNewPart::~dlgNewPart()
{
    delete ui;
}
