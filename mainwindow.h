#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <map>
#include "htabbase.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void ConnectSlots();

private slots:
    void OnTableChange();
    void OnUserLogin(bool);
    void OnError(QString);

private:
    Ui::MainWindow *ui;

    QTabWidget *m_pTabMain;

    int m_Index;
    std::map<int,HTabBase*> m_mapTabs;
};
#endif // MAINWINDOW_H
