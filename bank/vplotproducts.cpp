#include "vplotproducts.h"
#include "ui_vplotproducts.h"
#include "helcsignage.h"

extern HElcSignage* gSystem;

vPlotProducts::vPlotProducts(QWidget *parent) :
    HTabBase(parent),
    ui(new Ui::vPlotProducts)
{
    ui->setupUi(this);
}

vPlotProducts::~vPlotProducts()
{
    delete ui;
}

void vPlotProducts::OnInit()
{

}
