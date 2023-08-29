#ifndef VPLOTPRODUCTS_H
#define VPLOTPRODUCTS_H

#include <QWidget>
#include "htabbase.h"

namespace Ui {
class vPlotProducts;
}

class vPlotProducts : public HTabBase
{
    Q_OBJECT

public:
    explicit vPlotProducts(QWidget *parent = nullptr);
    ~vPlotProducts();

    virtual void OnInit();

private:
    Ui::vPlotProducts *ui;
};

#endif // VPLOTPRODUCTS_H
