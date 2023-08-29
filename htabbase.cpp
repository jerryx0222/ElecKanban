#include "htabbase.h"
#include"helcsignage.h"
#include <QApplication>

extern HElcSignage* gSystem;

HTabBase::HTabBase(QWidget *parent)
    : QWidget(parent)
{


    connect(&m_timer,SIGNAL(timeout()),this,SLOT(OnTimeout()));
    m_timer.start(1000);
}

void HTabBase::OnShowTab(bool)
{

}

void HTabBase::OnTimeout()
{
    if(gSystem!=nullptr)
    {
        //connect(gSystem,SIGNAL(OnInitional()),this,SLOT(OnInitional()));
        m_timer.stop();
        OnInit();
    }
}

