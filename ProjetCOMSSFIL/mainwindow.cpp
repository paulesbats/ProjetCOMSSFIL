#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "QGraphicsScene"
#include "QFileDialog"

// Create Reader object
ReaderName Reader;
const uint8_t BlockName = 10; // Block 10 for Name (on the sector 2)
const uint8_t BlockFirstName = 9; // Block 9 for Firstname (on the sector 2)

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

    qDebug() << "-------------------------------------------------------";
    qDebug() << "------ Starting NFC reading/writing application -------";
    qDebug() << "-------------------------------------------------------";
    qDebug() << "---------- May the force be with the Reader -----------";
    qDebug() << "-------------------------------------------------------";
    qDebug() << "";

    Reader.Type = ReaderCDC;  // Set reader type to CDC
    Reader.device = 0;
    status = OpenCOM(&Reader);  // Open communication with the reader device

    if(status==MI_OK){
        qDebug() << "Reader Connection sucess";
    }
    else{
        qDebug() << "Reader Connection fail";

    }

    RF_Power_Control(&Reader, TRUE, 0); // Turn on RF power on the reader
    status = Version(&Reader);  // Retrieve the version info from the reader

    // Initialize status window on the HMI
    ui->StatusWindow->setText(QString("No Problem | Reader version : %1").arg(QString::fromUtf8(Reader.version)));
    ui->StatusWindow->update();

    // Initialize connection status on the HMI
    ui->ConnectionStatus->setText("Card not connected");
    ui->ConnectionStatus->update();


    //Style Settings
    ui->groupBox->setStyleSheet("QGroupBox { border: 0; }");
    ui->groupBox_2->setStyleSheet("QGroupBox { border: 0; }");
    ui->groupBox_3->setStyleSheet("QGroupBox { border: 0; }");

    qApp->setStyleSheet("QTextEdit { border-radius: 10px; border: 1px solid #aaa; padding: 5px; }");
    qApp->setStyleSheet(R"(
        QPushButton {
            border: 2px solid #B8860B;
            border-radius: 12px;
            padding: 6px;
            background-color: #FFD700;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #FFF380;
        }
        QPushButton:pressed {
            background-color: #E6C200;
        }
        QTextEdit {
            border: 1px solid #aaa;
            border-radius: 10px;
            padding: 5px;
            background-color: #ffffff;
        }
    )");

    qDebug() << "";
    qDebug() << "";
    qDebug() << "";
    qDebug() << "";
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Connection to the card
void MainWindow::on_ConnectButton_clicked()
{
    uint8_t atq=0;
    uint8_t sak=0;
    uint8_t uid=0;
    uint16_t uid_len=0;

    qDebug() << "------------- Connection button clicked -------------";
    qDebug() << "";

    status = ISO14443_3_A_PollCard(&Reader,&atq,&sak,&uid,&uid_len); // Poll for a card using ISO14443-3 Type A

    // Update connection status on the HMI
    if(status==MI_OK){
        ui->ConnectionStatus->setText("Card connected !");
        ui->ConnectionStatus->update();
        LEDBuzzer(&Reader,LED_RED_ON);
        qDebug() << "Card Connection success";
    }
    else{
        ui->ConnectionStatus->setText("Card not connected");
        ui->ConnectionStatus->update();
        LEDBuzzer(&Reader,LED_GREEN_ON);
        qDebug() << "Card Connection fail";
    }
    qDebug() << "";

    // Variables declaration
    int16_t statusName = MI_OK;
    int16_t statusFirst = MI_OK;
    uint8_t Name[16]={0};
    uint8_t FirstName[16]={0};

    statusName = Mf_Classic_Read_Block(&Reader,TRUE,BlockName,Name,AuthKeyA,2); //Read on sector 2, Block 10, with KeyA
    statusFirst = Mf_Classic_Read_Block(&Reader,TRUE,BlockFirstName,FirstName,AuthKeyA,2); //Read on sector 2, Block 9, with KeyA

    // Debug feedback
    if (statusFirst == MI_OK && statusName == MI_OK){
        qDebug() << "Name and FirstName Reading success" ;
    }else{
        qDebug() << "Name and FirstName Reading fail" ;
    }
    qDebug() << "";

    // Convert Int into QString
    QString NameString;
    QString FirstNameString;

    for(int i=0;i<16 && QChar::isPrint(Name[i]); ++i){
        NameString += QChar(Name[i]);
    }

    for(int i=0;i<16 && QChar::isPrint(FirstName[i]); ++i){
        FirstNameString += QChar(FirstName[i]);
    }


    // Read wallet sold
    status = Mf_Classic_Read_Value	(&Reader,TRUE,14,&walletValue,AuthKeyA ,3);
    // 14 is the block number for the wallet | Use key A for reading | Use key A of the sector 3
    if(status==MI_OK){
        qDebug() << "Reading wallet success";
    }
    else{
        qDebug() << "Reading wallet fail";
    }
    qDebug() << "";

    // Read wallet backup sold
    status = Mf_Classic_Read_Value	(&Reader,TRUE,13,&walletBackupValue,AuthKeyA ,3);
    // 13 is the block number for the wallet backup | Use key A for reading | Use key A of the sector 3
    if(status==MI_OK){
        qDebug() << "Reading wallet backup success";
    }
    else{
        qDebug() << "Reading wallet backup fail";
    }
    qDebug() << "";

    // Display it on the HMI if wallet sold equals wallet backup sold else restore wallet sold then display it on the HMI
    if(walletValue==walletBackupValue){
        ui->WalletValue->setText(QString::number(walletValue));
        qDebug() << "Wallet sold is equal to wallet backup sold";
        qDebug() << "";
    }
    else{
        qDebug() << "Wallet sold is not equal to wallet backup sold";
        qDebug() << "";

        // Restore wallet sold with the wallet backup sold
        status = Mf_Classic_Restore_Value(&Reader,TRUE,13,14,AuthKeyA ,3);
        // 13 is the block number to read the wallet backup | 14 is the block number to restore the wallet sold | Use key A for restore | Use key A of the sector 3

        if(status==MI_OK){
            qDebug() << "Restoring wallet success";
        }
        else{
            qDebug() << "Restoring wallet fail";
        }
        ui->WalletValue->setText(QString::number(walletBackupValue));
        qDebug() << "";
    }
    ui->WalletValue->update();

    // Update identity UI
    ui->NameEntered->setText(NameString);
    ui->FirstNameEntered->setText(FirstNameString);

    ui->NameEntered->update();
    ui->FirstNameEntered->update();

    // Buzzer and LED Commands
    uint8_t	commandYLEDBuzzON = BUZZER_ON | LED_YELLOW_ON;
    LEDBuzzer(&Reader,commandYLEDBuzzON);

    usleep(100000);
    uint8_t	commandOFF = 0x00;
    LEDBuzzer(&Reader,commandOFF);
    LEDBuzzer(&Reader,LED_RED_ON);

    // Initialize connection status on the HMI
    ui->StatusWindow->setText("");
    ui->StatusWindow->update();

    qDebug() << "-------- End of connection button function --------";
    qDebug() << "";
    qDebug() << "";
    qDebug() << "";
    qDebug() << "";
}

// Update the identity in the card
void MainWindow::on_UpdateButton_clicked()
{
    qDebug() << "------------- Update button clicked -----------";
    qDebug() << "";

    // Read user entry
    QString NameString = ui->NameEntered->toPlainText();
    QString FirstNameString = ui->FirstNameEntered->toPlainText();

    // Condition for user to not exceed 16 characters
    if (NameString.length() > 16 || FirstNameString.length() > 16) {
        qDebug() << "ERROR: Name or FirstName exceeds 16 characters";
        qDebug() << "";
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
    if (statusFirst == MI_OK && statusName == MI_OK){
        qDebug() << "Name and FirstName Update success" ;
    }else{
        qDebug() << "Name and FirstName Update fail" ;
    }
    qDebug() << "";

    // Buzzer and LED Commands
    uint8_t	commandYLEDBuzzON = BUZZER_ON | LED_YELLOW_ON;
    LEDBuzzer(&Reader,commandYLEDBuzzON);

    usleep(100000);
    uint8_t	commandOFF = 0x00;
    LEDBuzzer(&Reader,commandOFF);
    LEDBuzzer(&Reader,LED_RED_ON);

    qDebug() << "-------- End of update button function --------";
    qDebug() << "";
    qDebug() << "";
    qDebug() << "";
    qDebug() << "";
}

// End of the Qt application
void MainWindow::on_ApplicationExit_clicked()
{

    RF_Power_Control(&Reader, FALSE, 0);  // Turn off RF power on the reader
    status = CloseCOM(&Reader);  // Close communication with the reader

    qDebug() << "-------------------------------------------------------";
    qDebug() << "------- Ending NFC reading/writing application --------";
    qDebug() << "-------------------------------------------------------";
    qDebug() << "--------- The Force will be with you. Always ----------";
    qDebug() << "-------------------------------------------------------";
    qDebug() << "";

    qApp->quit();  // Exit the Qt application
}


// Increment the value of the wallet
void MainWindow::on_LoadButton_clicked()
{

    qDebug() << "------------- Load button clicked -------------";
    qDebug() << "";

    // Retrieve the value to be incremented in the card wallet
    walletIncreDecremValue = ui->IncremDecremSpin->value();
    qDebug() << "Number of units to increment :" << walletIncreDecremValue;
    qDebug() << "";

    // Permit to avoid overflow of the wallet sold
    if( (int(walletValue)+int(walletIncreDecremValue))>maxWalletValue ){
        ui->StatusWindow->setText("Error : You cannot further increase the wallet sold");
        ui->StatusWindow->update();
        qDebug() << "Wallet sold overflow";
        qDebug() << "";
    }
    else if(walletIncreDecremValue == 0) {
        ui->StatusWindow->setText("Error : Please enter a non-zero value.");
        ui->StatusWindow->update();
        qDebug() << "Nothing to increment";
        qDebug() << "";
    }
    else{

        // Increment wallet value into NFC Card block
        status = Mf_Classic_Increment_Value	(&Reader,TRUE,14,walletIncreDecremValue,13,AuthKeyB ,3);
        // 14 is the block number to retrieve the wallet value | 13 is the block number to write the new wallet value | Use key B for incrementing | Use key B of the sector 3
        if(status==MI_OK){
            qDebug() << "Incrementing wallet success";
        }
        else{
            qDebug() << "Incrementing wallet fail";
        }
        qDebug() << "";

        // Restore wallet sold with the wallet backup
        status = Mf_Classic_Restore_Value(&Reader,TRUE,13,14,AuthKeyA ,3);
        // 13 is the block number to read the wallet backup | 14 is the block number to restore the wallet sold | Use key A for restore | Use key A of the sector 3
        if(status==MI_OK){
            qDebug() << "Restoring wallet success";
        }
        else{
            qDebug() << "Restoring wallet fail";
        }
        qDebug() << "";

        // Update wallet sold on HMI
        status = Mf_Classic_Read_Value	(&Reader,TRUE,14,&walletValue,AuthKeyA ,3);
        // 14 is the block number for the wallet | Use key A for reading | Use key A of the sector 3
        if(status==MI_OK){
            qDebug() << "Reading wallet success";
        }
        else{
            qDebug() << "Reading wallet fail";
        }
        qDebug() << "";

        ui->WalletValue->setText(QString::number(walletValue));
        ui->WalletValue->update();

        // Update status window
        ui->StatusWindow->setText("No Problem");
        ui->StatusWindow->update();

        // Buzzer and LED Commands
        uint8_t	commandYLEDBuzzON = BUZZER_ON | LED_YELLOW_ON;
        LEDBuzzer(&Reader,commandYLEDBuzzON);

        usleep(100000);
        uint8_t	commandOFF = 0x00;
        LEDBuzzer(&Reader,commandOFF);
        LEDBuzzer(&Reader,LED_RED_ON);
        }
    qDebug() << "-------- End of load button function --------";
    qDebug() << "";
    qDebug() << "";
    qDebug() << "";
    qDebug() << "";
}

// Decrement the value of the wallet
void MainWindow::on_BuyButton_clicked(){

    qDebug() << "------------- Buy button clicked -------------";
    qDebug() << "";

    // Retrieve the value to be decremented in the card wallet
    walletIncreDecremValue = ui->IncremDecremSpin->value();
    qDebug() << "Number of units to Decrement :" << walletIncreDecremValue;
    qDebug() << "";

    // Permit to avoid underflow of the wallet sold
    if( (int(walletValue)-int(walletIncreDecremValue))<0 ){
        ui->StatusWindow->setText("Error : You cannot further decrease the wallet sold");
        ui->StatusWindow->update();
        qDebug() << "Wallet sold underflow";
        qDebug() << "";
    }
    else if(walletIncreDecremValue == 0) {
        ui->StatusWindow->setText("Error : Please enter a non-zero value.");
        ui->StatusWindow->update();
        qDebug() << "Nothing to decrement";
        qDebug() << "";
    }
    else{

        // Decrement wallet value into NFC Card block
        status = Mf_Classic_Decrement_Value	(&Reader,TRUE,14,walletIncreDecremValue,13,AuthKeyA ,3);
        // 14 is the block number to retrieve the wallet value | 13 is the block number to write the new wallet value | Use key B for decrementing | Use key B of the sector 3
        if(status==MI_OK){
            qDebug() << "Decrementing wallet success";
        }
        else{
            qDebug() << "Decrementing wallet fail";
        }
        qDebug() << "";

        // Restore wallet sold with the wallet backup
        status = Mf_Classic_Restore_Value(&Reader,TRUE,13,14,AuthKeyA ,3);
        // 13 is the block number to read the wallet backup | 14 is the block number to restore the wallet sold | Use key A for restore | Use key A of the sector 3
        if(status==MI_OK){
            qDebug() << "Restoring wallet success";
        }
        else{
            qDebug() << "Restoring wallet fail";
        }
        qDebug() << "";

        // Update wallet sold on HMI
        status = Mf_Classic_Read_Value	(&Reader,TRUE,14,&walletValue,AuthKeyA ,3);
        // 14 is the block number for the wallet | Use key A for reading | Use key A of the sector 3
        if(status==MI_OK){
            qDebug() << "Reading wallet success";
        }
        else{
            qDebug() << "Reading wallet fail";
        }
        qDebug() << "";

        ui->WalletValue->setText(QString::number(walletValue));
        ui->WalletValue->update();

        // Update status window
        ui->StatusWindow->setText("No Problem");
        ui->StatusWindow->update();

        // Buzzer and LED Commands
        uint8_t	commandYLEDBuzzON = BUZZER_ON | LED_YELLOW_ON;
        LEDBuzzer(&Reader,commandYLEDBuzzON);

        usleep(100000);
        uint8_t	commandOFF = 0x00;
        LEDBuzzer(&Reader,commandOFF);
        LEDBuzzer(&Reader,LED_RED_ON);
    }
    qDebug() << "-------- End of buy button function --------";
    qDebug() << "";
    qDebug() << "";
    qDebug() << "";
    qDebug() << "";
}

// Reset identity and wallet in the card
void MainWindow::on_ResetButton_clicked(){

    qDebug() << "------------- Reset button clicked -------------";
    qDebug() << "";

    status = Mf_Classic_Write_Block	(&Reader,TRUE,8, (uint8_t *) "Identity",AuthKeyB ,2);
    status += Mf_Classic_Write_Block	(&Reader,TRUE,9, (uint8_t *) "Nothing",AuthKeyB ,2);
    status += Mf_Classic_Write_Block	(&Reader,TRUE,10, (uint8_t *) "Nothing",AuthKeyB ,2);
    // 8-9-10 are the block number for the identity | Use key B for writing | Use key B of the sector 2

    if(status==MI_OK){
        qDebug() << "Reset identity success";
    }
    else{
        qDebug() << "Reset identity fail";
    }
    qDebug() << "";

    status = Mf_Classic_Write_Block	(&Reader,TRUE,12, (uint8_t *) "Porte Monnaie",AuthKeyB ,3);
    status += Mf_Classic_Write_Value	(&Reader,TRUE,13, (uint32_t) 0,AuthKeyB ,3);
    status += Mf_Classic_Write_Value	(&Reader,TRUE,14, (uint32_t) 0,AuthKeyB ,3);
    // 12-13-14 are the block number for the wallet | Use key B for writing | Use key B of the sector 3

    if(status==MI_OK){
        qDebug() << "Reset wallet success";
    }
    else{
        qDebug() << "Reset wallet fail";
    }
    qDebug() << "";

    // Update wallet in the HMI
    ui->WalletValue->setText(QString::number(0));
    ui->WalletValue->update();

    // Update identity in the HMI
    ui->NameEntered->setText("Nothing");
    ui->FirstNameEntered->setText("Nothing");

    ui->NameEntered->update();
    ui->FirstNameEntered->update();

    // Buzzer and LED Commands
    uint8_t	commandYLEDBuzzON = BUZZER_ON | LED_YELLOW_ON;
    LEDBuzzer(&Reader,commandYLEDBuzzON);

    usleep(100000);
    uint8_t	commandOFF = 0x00;
    LEDBuzzer(&Reader,commandOFF);
    LEDBuzzer(&Reader,LED_RED_ON);

    qDebug() << "-------- End of reset button function --------";
    qDebug() << "";
    qDebug() << "";
    qDebug() << "";
    qDebug() << "";

}
