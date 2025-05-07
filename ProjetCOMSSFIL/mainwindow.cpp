#include "mainwindow.h"
#include "ui_mainwindow.h"

// Create Reader object
ReaderName Reader;

// Value to be incremented/decremented in the card wallet
uint32_t walletIncreDecremValue=0;

// Wallet sold
uint32_t walletValue=0;

// Wallet backup sold
uint32_t walletBackupValue=0;

// Max wallet sold
uint32_t maxWalletValue=100;

// Transaction status
int16_t status = MI_OK;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Reader.Type = ReaderCDC;  // Set reader type to CDC
    Reader.device = 0;
    status = OpenCOM(&Reader);  // Open communication with the reader device
    status = Version(&Reader);  // Retrieve the version info from the reader
    RF_Power_Control(&Reader, TRUE, 0); // Turn on RF power on the reader
    qDebug() << "Reader Connection :" << status;

    ui->StatusWindow->setText("No Problem");
    ui->StatusWindow->update();

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
    status = ISO14443_3_A_PollCard(&Reader,&atq,&sak,&uid,&uid_len); // Poll for a card using ISO14443-3 Type A
    qDebug() << "Card Connection Status : " << status;

    // Read wallet sold
    status = Mf_Classic_Read_Value	(&Reader,TRUE,14,&walletValue,AuthKeyA ,3);
    // 14 is the block number for the wallet | Use key A for reading | Use key A of the sector 3
    if(status==MI_OK){
        qDebug() << "Reading wallet success" << status;
    }
    else{
        qDebug() << "Reading wallet fail" << status;
    }

    // Read wallet backup sold
    status = Mf_Classic_Read_Value	(&Reader,TRUE,13,&walletBackupValue,AuthKeyA ,3);
    // 13 is the block number for the wallet backup | Use key A for reading | Use key A of the sector 3
    if(status==MI_OK){
        qDebug() << "Reading wallet backup success" << status;
    }
    else{
        qDebug() << "Reading wallet backup fail" << status;
    }

    // Display it on the HMI if wallet sold equals wallet backup sold else restore wallet sold then display it on the HMI
    if(walletValue==walletBackupValue){
        ui->WalletValue->setText(QString::number(walletValue));
    }
    else{
        // Restore wallet sold with the wallet backup sold
        status = Mf_Classic_Restore_Value(&Reader,TRUE,13,14,AuthKeyA ,3);
        if(status==MI_OK){
            qDebug() << "Restoring wallet success" << status;
        }
        else{
            qDebug() << "Restoring wallet fail" << status;
        }
        ui->WalletValue->setText(QString::number(walletBackupValue));
    }
    ui->WalletValue->update();
}

void MainWindow::on_ApplicationExit_clicked()
{
    RF_Power_Control(&Reader, FALSE, 0);  // Turn off RF power on the reader
    status = CloseCOM(&Reader);  // Close communication with the reader
    qApp->quit();  // Exit the Qt application
}

// Increment the value of the wallet
void MainWindow::on_LoadButton_clicked()
{

    // Retrieve the value to be incremented in the card wallet
    walletIncreDecremValue = ui->IncremDecremSpin->value();
    qDebug() << "Number of units to increment :" << walletIncreDecremValue;

    // Permit to avoid overflow of the wallet sold
    if( (int(walletValue)+int(walletIncreDecremValue))>maxWalletValue ){
        ui->StatusWindow->setText("Error : You cannot further increase the wallet sold");
        ui->StatusWindow->update();
    }
    else if(walletIncreDecremValue == 0) {
        ui->StatusWindow->setText("Error : Please enter a non-zero value.");
        ui->StatusWindow->update();
    }
    else{

        // Increment wallet value into NFC Card block
        status = Mf_Classic_Increment_Value	(&Reader,TRUE,14,walletIncreDecremValue,13,AuthKeyB ,3);
        // 14 is the block number to retrieve the wallet value | 13 is the block number to write the new wallet value | Use key B for incrementing | Use key B of the sector 3
        if(status==MI_OK){
            qDebug() << "Incrementing wallet success" << status;
        }
        else{
            qDebug() << "Incrementing wallet fail" << status;
        }

        // Restore wallet sold with the wallet backup
        status = Mf_Classic_Restore_Value(&Reader,TRUE,13,14,AuthKeyA ,3);
        if(status==MI_OK){
            qDebug() << "Restoring wallet success" << status;
        }
        else{
            qDebug() << "Restoring wallet fail" << status;
        }

        // Update wallet sold on HMI
        status = Mf_Classic_Read_Value	(&Reader,TRUE,14,&walletValue,AuthKeyA ,3);
        // 14 is the block number for the wallet | Use key A for reading | Use key A of the sector 3
        if(status==MI_OK){
            qDebug() << "Reading wallet success" << status;
        }
        else{
            qDebug() << "Reading wallet fail" << status;
        }

        ui->WalletValue->setText(QString::number(walletValue));
        ui->WalletValue->update();

        // Update status window
        ui->StatusWindow->setText("No Problem");
        ui->StatusWindow->update();
    }
}

// Decrement the value of the wallet
void MainWindow::on_BuyButton_clicked(){

    // Retrieve the value to be decremented in the card wallet
    walletIncreDecremValue = ui->IncremDecremSpin->value();
    qDebug() << "Number of units to Decrement :" << walletIncreDecremValue;

    // Permit to avoid underflow of the wallet sold
    if( (int(walletValue)-int(walletIncreDecremValue))<0 ){
        ui->StatusWindow->setText("Error : You cannot further decrease the wallet sold");
        ui->StatusWindow->update();
    }
    else if(walletIncreDecremValue == 0) {
        ui->StatusWindow->setText("Error : Please enter a non-zero value.");
        ui->StatusWindow->update();
    }
    else{

        // Decrement wallet value into NFC Card block
        status = Mf_Classic_Decrement_Value	(&Reader,TRUE,14,walletIncreDecremValue,13,AuthKeyA ,3);
        // 14 is the block number to retrieve the wallet value | 13 is the block number to write the new wallet value | Use key B for decrementing | Use key B of the sector 3
        if(status==MI_OK){
            qDebug() << "Decrementing wallet success" << status;
        }
        else{
            qDebug() << "Decrementing wallet fail" << status;
        }

        // Restore wallet sold with the wallet backup
        status = Mf_Classic_Restore_Value(&Reader,TRUE,13,14,AuthKeyA ,3);
        if(status==MI_OK){
            qDebug() << "Restoring wallet success" << status;
        }
        else{
            qDebug() << "Restoring wallet fail" << status;
        }

        // Update wallet sold on HMI
        status = Mf_Classic_Read_Value	(&Reader,TRUE,14,&walletValue,AuthKeyA ,3);
        // 14 is the block number for the wallet | Use key A for reading | Use key A of the sector 3
        if(status==MI_OK){
            qDebug() << "Reading wallet success" << status;
        }
        else{
            qDebug() << "Reading wallet fail" << status;
        }

        ui->WalletValue->setText(QString::number(walletValue));
        ui->WalletValue->update();

        // Update status window
        ui->StatusWindow->setText("No Problem");
        ui->StatusWindow->update();
    }
}
