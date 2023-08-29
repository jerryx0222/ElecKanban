#ifndef HTABBASE_H
#define HTABBASE_H

#include <QObject>
#include <QWidget>
#include <QTimer>

class HTabBase : public QWidget
{
    Q_OBJECT
public:
    explicit HTabBase(QWidget *parent = nullptr);

    virtual void OnInit()=0;
    virtual void OnShowTab(bool);

protected:
    QTimer  m_timer;

signals:

public slots:
    void OnTimeout();



};

#endif // HTABBASE_H
