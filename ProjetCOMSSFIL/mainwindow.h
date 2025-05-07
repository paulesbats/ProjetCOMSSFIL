#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <iostream>

#include "MfErrNo.h"
#include "Sw_Device.h"
#include "Sw_ISO14443A-3.h"
#include "Sw_Mf_Classic.h"
#include "Tools.h"
#include "TypeDefs.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void on_ConnectButton_clicked();
    void on_ApplicationExit_clicked();
    void on_UpdateButton_clicked();
    void on_BuyButton_clicked();
    void on_LoadButton_clicked();
};
#endif // MAINWINDOW_H
