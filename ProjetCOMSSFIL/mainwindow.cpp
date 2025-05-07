#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"


ReaderName Reader;
const uint8_t BlockName = 10; // Block 10 for Name (on the sector 2)
const uint8_t BlockFirstName = 9; // Block 9 for Firstname (on the sector 2)


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

    // Variables declaration
    int16_t statusName = MI_OK;
    int16_t statusFirst = MI_OK;
    uint8_t Name[16]={0};
    uint8_t FirstName[16]={0};

    statusName = Mf_Classic_Read_Block(&Reader,TRUE,BlockName,Name,AuthKeyA,2); //Read on sector 2, Block 10, with KeyA
    statusFirst = Mf_Classic_Read_Block(&Reader,TRUE,BlockFirstName,FirstName,AuthKeyA,2); //Read on sector 2, Block 9, with KeyA

    // Debug feedback
    if (statusFirst == 0 && statusName == 0){
        qDebug() << "Name and FirstName Read OK" ;
    }else{
        qDebug() << "Name and FirstName Read ERROR" ;
    }

    // Convert Int into QString
    QString NameString;
    QString FirstNameString;

    for(int i=0;i<16 && QChar::isPrint(Name[i]); ++i){
        NameString += QChar(Name[i]);
    }

    for(int i=0;i<16 && QChar::isPrint(FirstName[i]); ++i){
        FirstNameString += QChar(FirstName[i]);
    }

    // Update UI
    ui->NameEntered->setText(NameString);
    ui->FirstNameEntered->setText(FirstNameString);

    ui->NameEntered->update();
    ui->FirstNameEntered->update();
}

void MainWindow::on_UpdateButton_clicked()
{
    // Read user entry
    QString NameString = ui->NameEntered->toPlainText();
    QString FirstNameString = ui->FirstNameEntered->toPlainText();

    // Condition for user to not exceed 16 characters
    if (NameString.length() > 16 || FirstNameString.length() > 16) {
        qDebug() << "ERROR: Name or FirstName exceeds 16 characters";
        return;
    }

    // Variables declaration
    int16_t statusName = MI_OK;
    int16_t statusFirst = MI_OK;
    uint8_t Name[16]={0};
    uint8_t FirstName[16]={0};

    // Convert QString into Int
    QByteArray byteName = NameString.toUtf8();
    for (int i = 0; i < byteName.size() && i < 16; ++i) {
        Name[i] = static_cast<uint8_t>(byteName.at(i));
    }
    QByteArray byteFirst = FirstNameString.toUtf8();
    for (int i = 0; i < byteFirst.size() && i < 16; ++i) {
        FirstName[i] = static_cast<uint8_t>(byteFirst.at(i));
    }

    statusName = Mf_Classic_Write_Block(&Reader,TRUE,BlockName,Name,AuthKeyB,2); //Write on sector 2, Block 10, with KeyB
    statusFirst = Mf_Classic_Write_Block(&Reader,TRUE,BlockFirstName,FirstName,AuthKeyB,2); //Write on sector 2, Block 9, with KeyB

    // Debug feedback
    if (statusFirst == 0 && statusName == 0){
        qDebug() << "Name and FirstName Update OK" ;
    }else{
        qDebug() << "Name and FirstName Update ERROR" ;
    }
}

void MainWindow::on_ApplicationExit_clicked()
{
    int16_t status = MI_OK;
    RF_Power_Control(&Reader, FALSE, 0);  // Turn off RF power on the reader
    status = CloseCOM(&Reader);  // Close communication with the reader
    qApp->quit();  // Exit the Qt application
}


