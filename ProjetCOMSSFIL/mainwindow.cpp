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
    RF_Power_Control(&Reader, TRUE, 0); // Turn on RF power on the reader
    qDebug() << "Reader Connection :" << status;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_ConnectButton_clicked()
{
    uint8_t atq=0;
    uint8_t sak=0;
    uint8_t uid=0;
    uint16_t uid_len=0;
    int16_t status = MI_OK;
    status = ISO14443_3_A_PollCard(&Reader,&atq,&sak,&uid,&uid_len); // Poll for a card using ISO14443-3 Type A
    qDebug() << "Card Connection Status : " << status;
}

void MainWindow::on_ApplicationExit_clicked()
{
    int16_t status = MI_OK;
    RF_Power_Control(&Reader, FALSE, 0);  // Turn off RF power on the reader
    status = CloseCOM(&Reader);  // Close communication with the reader
    qApp->quit();  // Exit the Qt application
}
