#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"


ReaderName Reader;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    int16_t status = MI_OK;
    Reader.Type = ReaderCDC;  // Set reader type to CDC
    Reader.device = 0;
    status = OpenCOM(&Reader);  // Open communication with the reader device
    status = Version(&Reader);  // Retrieve the version info from the reader
    qDebug() << "Reader Connection :" << status;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_ConnectButton_clicked()
{

}
