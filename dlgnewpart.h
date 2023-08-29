#ifndef DLGNEWPART_H
#define DLGNEWPART_H

#include <QWidget>

namespace Ui {
class dlgNewPart;
}

class dlgNewPart : public QWidget
{
    Q_OBJECT

public:
    explicit dlgNewPart(QWidget *parent = nullptr);
    ~dlgNewPart();

private:
    Ui::dlgNewPart *ui;
};

#endif // DLGNEWPART_H
