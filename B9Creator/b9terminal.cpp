/*************************************************************************************
//
//  LICENSE INFORMATION
//
//  BCreator(tm)
//  Software for the control of the 3D Printer, "B9Creator"(tm)
//
//  Copyright 2011-2012 B9Creations, LLC
//  B9Creations(tm) and B9Creator(tm) are trademarks of B9Creations, LLC
//
//  This work is licensed under the:
//      "Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License"
//
//  To view a copy of this license, visit:
//      http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US
//
//
//  For updates and to download the lastest version, visit:
//      http://github.com/B9Creations or
//      http://b9creator.com
//
//  The above copyright notice and this permission notice shall be
//    included in all copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
*************************************************************************************/
#include <QtDebug>
#include <QMessageBox>
#include "b9terminal.h"
#include "ui_b9terminal.h"

B9Terminal::B9Terminal(QWidget *parent, Qt::WFlags flags) :
    QWidget(parent, flags),
    ui(new Ui::B9Terminal)
{
    m_bWaiverPresented = false;
    m_bWaiverAccepted = false;
    m_bWavierActive = false;

    ui->setupUi(this);
    ui->commStatus->setText("Searching for B9Creator...");

    qDebug() << "Terminal Start";

    // Always set up the B9PrinterComm in the Terminal constructor
    pPrinterComm = new B9PrinterComm;
    pPrinterComm->enableBlankCloning(true); // Allow for firmware update of suspected "blank" B9Creator Arduino's

    // Always set up the B9Projector in the Terminal constructor
    m_pDesktop = QApplication::desktop();
    pProjector = NULL;
    m_bPrimaryScreen = false;
    onScreenCountChanged(); // Determine if/where the projector is connected
    if(m_bPrimaryScreen){
        QMessageBox msg;
        msg.setWindowTitle("Projector Connection?");
        msg.setText("WARNING:  The printer's projector is not connected to a secondary video output?  Please check that all connections (VGA or HDMI) and system display settings are correct.  Disregard this message if your system has one video output and you will utilizes a splitter to provide video output to both monitor and Projector.");
        msg.exec();
    }

    connect(m_pDesktop, SIGNAL(screenCountChanged(int)),this, SLOT(onScreenCountChanged(int)));

    connect(pPrinterComm,SIGNAL(updateConnectionStatus(QString)), this, SLOT(onUpdateConnectionStatus(QString)));
    connect(pPrinterComm,SIGNAL(BC_ConnectionStatusDetailed(QString)), this, SLOT(onBC_ConnectionStatusDetailed(QString)));
    connect(pPrinterComm,SIGNAL(BC_LostCOMM()),this,SLOT(onBC_LostCOMM()));

    connect(pPrinterComm,SIGNAL(BC_RawData(QString)), this, SLOT(onUpdateRAWPrinterComm(QString)));
    connect(pPrinterComm,SIGNAL(BC_Comment(QString)), this, SLOT(onUpdatePrinterComm(QString)));

    connect(pPrinterComm,SIGNAL(BC_ModelInfo(QString)),this,SLOT(onBC_ModelInfo(QString)));
    connect(pPrinterComm,SIGNAL(BC_FirmVersion(QString)),this,SLOT(onBC_FirmVersion(QString)));
    connect(pPrinterComm,SIGNAL(BC_ProjectorRemoteCapable(bool)), this, SLOT(onBC_ProjectorRemoteCapable(bool)));
    connect(pPrinterComm,SIGNAL(BC_HasShutter(bool)), this, SLOT(onBC_HasShutter(bool)));
    connect(pPrinterComm,SIGNAL(BC_ProjectorStatusChanged()), this, SLOT(onBC_ProjStatusChanged()));
    connect(pPrinterComm,SIGNAL(BC_ProjectorFAIL()), this, SLOT(onBC_ProjStatusFAIL()));

    // Z Position Control
    connect(pPrinterComm, SIGNAL(BC_PU(int)), this, SLOT(onBC_PU(int)));
    connect(pPrinterComm, SIGNAL(BC_UpperZLimPU(int)), this, SLOT(onBC_UpperZLimPU(int)));
    m_pResetTimer = new QTimer(this);
    connect(m_pResetTimer, SIGNAL(timeout()), this, SLOT(onMotionResetTimeout()));
    connect(pPrinterComm, SIGNAL(BC_HomeFound()), this, SLOT(onMotionResetComplete()));
    connect(pPrinterComm, SIGNAL(BC_CurrentZPosInPU(int)), this, SLOT(onBC_CurrentZPosInPU(int)));

    m_pVatTimer = new QTimer(this);
    connect(m_pVatTimer, SIGNAL(timeout()), this, SLOT(onMotionVatTimeout()));
    connect(pPrinterComm, SIGNAL(BC_CurrentVatPercentOpen(int)), this, SLOT(onBC_CurrentVatPercentOpen(int)));

    m_pPReleaseCycleTimer = new QTimer(this);
    connect(m_pPReleaseCycleTimer, SIGNAL(timeout()), this, SLOT(onReleaseCycleTimeout()));
    connect(pPrinterComm, SIGNAL(BC_PrintReleaseCycleFinished()), this, SLOT(onBC_PrintReleaseCycleFinished()));
}

B9Terminal::~B9Terminal()
{
    delete ui;
    delete pPrinterComm;
    qDebug() << "Terminal End";
}

void B9Terminal::makeProjectorConnections()
{
    // should be called any time we create a new projector object
    connect(pProjector, SIGNAL(keyReleased(int)),this, SLOT(getKey(int)));
    connect(this, SIGNAL(sendStatusMsg(QString)),pProjector, SLOT(setStatusMsg(QString)));
    connect(this, SIGNAL(sendGrid(bool)),pProjector, SLOT(setShowGrid(bool)));
    connect(this, SIGNAL(sendCPJ(CrushedPrintJob*)),pProjector, SLOT(setCPJ(CrushedPrintJob*)));
    connect(this, SIGNAL(sendXoff(int)),pProjector, SLOT(setXoff(int)));
    connect(this, SIGNAL(sendYoff(int)),pProjector, SLOT(setYoff(int)));
}

void B9Terminal::setEnabledWarned(){
    if(isHidden())return;
    if(!m_bWaiverPresented||m_bWaiverAccepted==false){
        // Present Waiver
        m_bWaiverPresented = true;
        m_bWaiverAccepted = false;
        if(!m_bWavierActive){
            m_bWavierActive = true;
            int ret = QMessageBox::information(this, tr("Enable Terminal Control?"),
                                           tr("Warning: Manual operation can damage the VAT coating.\n"
                                              "If your VAT is installed and empty of resin care must be\n"
                                              "taken to ensure it is not damaged.  Operation is only safe\n"
                                              "with either the VAT removed, or the Sweeper and Build Table removed.\n"
                                              "The purpose of this utility is to assist in troubleshooting.\n"
                                              "Its use is not required during normal printing operations.\n"
                                              "Do you want to enable manual control?"),
                                           QMessageBox::Yes | QMessageBox::No
                                           | QMessageBox::Cancel);
            if(ret==QMessageBox::Cancel){hide();return;}
            else if(ret==QMessageBox::Yes)m_bWaiverAccepted=true;
            m_bWavierActive = false;        }
    }
    ui->groupBoxMain->setEnabled(m_bWaiverAccepted&&pPrinterComm->isConnected());
}

void B9Terminal::hideEvent(QHideEvent *event)
{
    emit eventHiding();
    event->accept();
}

void B9Terminal::sendCommand()
{
    pPrinterComm->SendCmd(ui->lineEditCommand->text());
    ui->lineEditCommand->clear();
}

void B9Terminal::onBC_LostCOMM(){
    //Broadcast an alert
    emit signalAbortPrint("ERROR: Lost Printer Connection.  Possible reasons: Power Loss, USB cord unplugged.");
}

void B9Terminal::onBC_ConnectionStatusDetailed(QString sText)
{
    setEnabledWarned();

    ui->commStatus->setText(sText);
}

void B9Terminal::onUpdateConnectionStatus(QString sText)
{
    emit (updateConnectionStatus(sText));
}

void B9Terminal::onUpdatePrinterComm(QString sText)
{
    QString html = "<font color=\"Black\">" + sText + "</font><br>";
    ui->textEditCommOut->insertHtml(html);
    ui->textEditCommOut->setAlignment(Qt::AlignBottom);
}
void B9Terminal::onUpdateRAWPrinterComm(QString sText)
{
    QString html = "<font color=\"Blue\">" + sText + "</font><br>";
    ui->textEditCommOut->insertHtml(html);
    ui->textEditCommOut->setAlignment(Qt::AlignBottom);
}

void B9Terminal::on_pushButtonProjPower_toggled(bool checked)
{
    // User has changed the commanded projector power setting
    ui->pushButtonProjPower->setChecked(checked);
    if(checked)
        ui->pushButtonProjPower->setText("ON");
    else
        ui->pushButtonProjPower->setText("OFF");
    pPrinterComm->cmdProjectorPowerOn(checked);
    emit(setProjectorPowerCmd(checked));

    // if m_bPrimaryScreen is true, we need to show it before turning on projector!
    if(m_bPrimaryScreen) onScreenCountChanged();
    emit sendStatusMsg("B9Creator - Projector status: CMD ON");
}

void B9Terminal::setProjectorPowerCmd(bool bPwrFlag){
    pPrinterComm->setProjectorPowerCmd(bPwrFlag);
}


void B9Terminal::onBC_ProjStatusFAIL()
{
    // First, update the interface
    on_pushButtonProjPower_toggled(pPrinterComm->isProjectorPowerCmdOn());
    onBC_ProjStatusChanged();  

    // Now let the user know something bad happened...
    QMessageBox msg;
    msg.setIcon(QMessageBox::Warning);
    msg.setFixedWidth(100);
    msg.setWhatsThis("This is a warning message to let you know that the Projector is not performing as expected.");
    msg.setWindowTitle("Projector Error");
    if(pPrinterComm->getProjectorStatus() == B9PrinterStatus::PS_TIMEOUT)
        msg.setText("Timed out while attempting to turn on projector.  Check Projector's Power Cord and RS-232 cable.");
    else if(pPrinterComm->getProjectorStatus() == B9PrinterStatus::PS_FAIL)
        msg.setText("Lost Communications with Projector.  Possible Causes:  Manually powered off, Power Failure, Cord Disconnected or Projector Lamp Failure");
    else
        msg.setText("Unknown Projector FAIL Mode Encountered.  Check all connections.");

    qDebug() << "Projector FAIL broadcast: " << msg.text();
    //msg.exec();
}

void B9Terminal::onBC_ProjStatusChanged()
{
    QString sText = "UNKNOWN";
    switch (pPrinterComm->getProjectorStatus()){
    case B9PrinterStatus::PS_OFF:
        ui->pushButtonProjPower->setEnabled(true);
        sText = "OFF";
        break;
    case B9PrinterStatus::PS_TURNINGON:
        ui->pushButtonProjPower->setEnabled(false);
        sText = "CMD ON";
        break;
    case B9PrinterStatus::PS_WARMING:
        ui->pushButtonProjPower->setEnabled(true);
        sText = "WARM UP";
        break;
    case B9PrinterStatus::PS_ON:
        ui->pushButtonProjPower->setEnabled(true);
        sText = "ON";
        break;
    case B9PrinterStatus::PS_COOLING:
        ui->pushButtonProjPower->setEnabled(false);
        sText = "COOL DN";
        break;
    case B9PrinterStatus::PS_TIMEOUT:
        ui->pushButtonProjPower->setEnabled(false);
        sText = "TIMEOUT";
        emit signalAbortPrint("Timed out while attempting to turn on projector.  Check Projector's Power Cord and RS-232 cable.");
        break;
    case B9PrinterStatus::PS_FAIL:
        ui->pushButtonProjPower->setEnabled(false);
        sText = "FAIL";
        emit signalAbortPrint("Lost Communications with Projector.  Possible Causes:  Manually powered off, Power Failure, Cord Disconnected or Projector Lamp Failure");
        break;
    case B9PrinterStatus::PS_UNKNOWN:
        ui->pushButtonProjPower->setEnabled(true);
        emit signalAbortPrint("Unknown Projector FAIL Mode Encountered.  Check all connections.");
    default:
        sText = "UNKNOWN";
        break;
    }
    emit sendStatusMsg("B9Creator - Projector status: "+sText);

    ui->lineEditProjState->setText(sText);
    sText = "UNKNOWN";
    int iLH = pPrinterComm->getLampHrs();
    if(iLH >= 0 && (pPrinterComm->getProjectorStatus()==B9PrinterStatus::PS_ON||
                    pPrinterComm->getProjectorStatus()==B9PrinterStatus::PS_WARMING||
                    pPrinterComm->getProjectorStatus()==B9PrinterStatus::PS_COOLING))sText = QString::number(iLH);
    ui->lineEditLampHrs->setText(sText);
}


void B9Terminal::on_pushButtonCmdReset_clicked()
{
    /*
    int ret = QMessageBox::information(this, tr("Initialize Home Positions?"),
                                   tr("This command will move the Build Table and VAT to their home locations.\n"
                                      "Are you sure you wish to proceed?"),
                                   QMessageBox::Yes | QMessageBox::Cancel);
    if(ret==QMessageBox::Cancel){return;}
    */

    int iTimeoutEstimate = 80000; // 80 seconds (should never take longer than 75 secs from upper limit)

    ui->groupBoxMain->setEnabled(false);
    ui->lineEditNeedsInit->setText("Seeking");
    // Remote activation of Reset (Find Home) Motion
    m_pResetTimer->start(iTimeoutEstimate);
    pPrinterComm->SendCmd("R");
}

void B9Terminal::onMotionResetComplete()
{    
    ui->groupBoxMain->setEnabled(true);
    if(pPrinterComm->getHomeStatus()==B9PrinterStatus::HS_FOUND) ui->lineEditNeedsInit->setText("No");
    else if(pPrinterComm->getHomeStatus()==B9PrinterStatus::HS_UNKNOWN) ui->lineEditNeedsInit->setText("Yes");
    else ui->lineEditNeedsInit->setText("Seeking");
    ui->lineEditZDiff->setText(QString::number(pPrinterComm->getLastHomeDiff()).toAscii());
    m_pResetTimer->stop();
}

void B9Terminal::onMotionResetTimeout(){
    ui->groupBoxMain->setEnabled(true);
    m_pResetTimer->stop();
    QMessageBox msg;
    msg.setText("ERROR: TIMEOUT attempting to locate home position.  Check connections.");
    msg.exec();
}



void B9Terminal::onBC_ModelInfo(QString sModel){
    ui->lineEditModelInfo->setText(sModel);
}

void B9Terminal::onBC_FirmVersion(QString sVersion){
    ui->lineEditFirmVersion->setText(sVersion);
}


void B9Terminal::onBC_ProjectorRemoteCapable(bool bCapable){
    ui->groupBoxProjector->setEnabled(bCapable);
}
void B9Terminal::onBC_HasShutter(bool bHS){
    ui->groupBoxVAT->setEnabled(bHS);
}

void B9Terminal::onBC_PU(int iPU){
    double dPU = (double)iPU/100000.0;
    ui->lineEditPUinMicrons->setText(QString::number(dPU,'g',8));
}

void B9Terminal::onBC_UpperZLimPU(int iUpZLimPU){
    double dZUpLimMM = (iUpZLimPU * pPrinterComm->getPU())/100000.0;
    ui->lineEditUpperZLimMM->setText(QString::number(dZUpLimMM,'g',8));
    ui->lineEditUpperZLimInches->setText(QString::number(dZUpLimMM/25.4,'g',8));
    ui->lineEditUpperZLimPU->setText(QString::number(iUpZLimPU,'g',8));
}

void B9Terminal::onBC_CurrentZPosInPU(int iCurZPU){
    double dZPosMM = (iCurZPU * pPrinterComm->getPU())/100000.0;
    ui->lineEditCurZPosInMM->setText(QString::number(dZPosMM,'g',8));
    ui->lineEditCurZPosInInches->setText(QString::number(dZPosMM/25.4,'g',8));
    ui->lineEditCurZPosInPU->setText(QString::number(iCurZPU,'g',8));
}

void B9Terminal::on_lineEditZRaiseSpd_editingFinished()
{
    int iValue=ui->lineEditZRaiseSpd->text().toInt();
    if(QString::number(iValue)!=ui->lineEditZRaiseSpd->text()||
            iValue<0 || iValue >100){
        int ret = QMessageBox::information(this, tr("Z Print Raise Speed Out of Range"),
                                       tr("Please enter an integer value between 0-100\n"
                                          "With 0 for minimum and 100 for maximum. \n"
                                          "Factory setting is 85.\n"),
                                       QMessageBox::Ok);
        iValue = 85;
        ui->lineEditZRaiseSpd->setText(QString::number(iValue));
        ui->lineEditZRaiseSpd->setFocus();
        ui->lineEditZRaiseSpd->selectAll();
    }
}

void B9Terminal::on_lineEditZLowerSpd_editingFinished()
{
    int iValue=ui->lineEditZLowerSpd->text().toInt();
    if(QString::number(iValue)!=ui->lineEditZLowerSpd->text()||
            iValue<0 || iValue >100){
        int ret = QMessageBox::information(this, tr("Z Print Lower Speed Out of Range"),
                                       tr("Please enter an integer value between 0-100\n"
                                          "With 0 for minimum and 100 for maximum. \n"
                                          "Factory setting is 85.\n"),
                                       QMessageBox::Ok);
        iValue = 85;
        ui->lineEditZLowerSpd->setText(QString::number(iValue));
        ui->lineEditZLowerSpd->setFocus();
        ui->lineEditZLowerSpd->selectAll();
    }
}

void B9Terminal::on_lineEditVatOpenSpeed_editingFinished()
{
    int iValue=ui->lineEditVatOpenSpeed->text().toInt();
    if(QString::number(iValue)!=ui->lineEditVatOpenSpeed->text()||
            iValue<0 || iValue >100){
        int ret = QMessageBox::information(this, tr("VAT Open Speed Out of Range"),
                                       tr("Please enter an integer value between 0-100\n"
                                          "With 0 for minimum and 100 for maximum. \n"
                                          "Factory setting is 100.\n"),
                                       QMessageBox::Ok);
        iValue = 100;
        ui->lineEditVatOpenSpeed->setText(QString::number(iValue));
        ui->lineEditVatOpenSpeed->setFocus();
        ui->lineEditVatOpenSpeed->selectAll();
    }
}

void B9Terminal::on_lineEditVatCloseSpeed_editingFinished()
{
    int iValue=ui->lineEditVatCloseSpeed->text().toInt();
    if(QString::number(iValue)!=ui->lineEditVatCloseSpeed->text()||
            iValue<0 || iValue >100){
        int ret = QMessageBox::information(this, tr("VAT Close Speed Out of Range"),
                                       tr("Please enter an integer value between 0-100\n"
                                          "With 0 for minimum and 100 for maximum. \n"
                                          "Factory setting is 100.\n"),
                                       QMessageBox::Ok);
        iValue = 100;
        ui->lineEditVatCloseSpeed->setText(QString::number(iValue));
        ui->lineEditVatCloseSpeed->setFocus();
        ui->lineEditVatCloseSpeed->selectAll();
    }
}

void B9Terminal::on_lineEditDelayClosedPos_editingFinished()
{
    int iValue=ui->lineEditDelayClosedPos->text().toInt();
    if(QString::number(iValue)!=ui->lineEditDelayClosedPos->text()||
            iValue<0 || iValue >10000){
        int ret = QMessageBox::information(this, tr("Delay @ Closed Out of Range"),
                                       tr("Please enter an integer value between 0-10000.\n"
                                          "This is a delay time in milliseconds.\n"
                                          "This 'Breathe' delay occurs in the VAT Closed position.\n"
                                          "Factory setting is 0\n"),
                                       QMessageBox::Ok);
        iValue = 0;
        ui->lineEditDelayClosedPos->setText(QString::number(iValue));
        ui->lineEditDelayClosedPos->setFocus();
        ui->lineEditDelayClosedPos->selectAll();
    }
}

void B9Terminal::on_lineEditDelayOpenPos_editingFinished()
{
    int iValue=ui->lineEditDelayOpenPos->text().toInt();
    if(QString::number(iValue)!=ui->lineEditDelayOpenPos->text()||
            iValue<0 || iValue >10000){
        int ret = QMessageBox::information(this, tr("Delay @ Opened Out of Range"),
                                       tr("Please enter an integer value between 0-10000.\n"
                                          "This is a delay time in milliseconds.\n"
                                          "This 'Fill' delay occurs in the VAT Open position.\n"
                                          "Factory setting is 0\n"),
                                       QMessageBox::Ok);
        iValue = 0;
        ui->lineEditDelayOpenPos->setText(QString::number(iValue));
        ui->lineEditDelayOpenPos->setFocus();
        ui->lineEditDelayOpenPos->selectAll();
    }
}

void B9Terminal::on_lineEditOverLift_editingFinished()
{
    int iValue=ui->lineEditOverLift->text().toInt();
    if(QString::number(iValue)!=ui->lineEditOverLift->text()||
            iValue<0 || iValue >1000){
        int ret = QMessageBox::information(this, tr("Overlift Out of Range"),
                                       tr("Please enter an integer value between 0-1000.\n"
                                          "We 'overlift' the Build Table by this amount before opening\n"
                                          "the VAT, then lower to the correct poition before exposure\n"
                                          "Units are in Z Steps, for example: 1000 = 6.35mm or .25\"\n"
                                          "Factory setting is 0\n"),
                                       QMessageBox::Ok);
        iValue = 0;
        ui->lineEditOverLift->setText(QString::number(iValue));
        ui->lineEditOverLift->setFocus();
        ui->lineEditOverLift->selectAll();
    }
}

void B9Terminal::on_lineEditTgtZPU_editingFinished()
{
    int iValue=ui->lineEditTgtZPU->text().toInt();
    if(QString::number(iValue)!=ui->lineEditTgtZPU->text()||
            iValue<0 || iValue >31497){
        int ret = QMessageBox::information(this, tr("Target Level (Z steps) Out of Range"),
                                       tr("Please enter an integer value between 0-31497.\n"
                                          "This will be the altitude for the next layer.\n"),
                                       QMessageBox::Ok);
        iValue = 0;
        ui->lineEditTgtZPU->setText(QString::number(iValue));
        ui->lineEditTgtZPU->setFocus();
        ui->lineEditTgtZPU->selectAll();
    }
    setTgtAltitudePU(iValue);
}

void B9Terminal::on_lineEditTgtZMM_editingFinished()
{
    double dValue=ui->lineEditTgtZMM->text().toDouble();
    double dTest = QString::number(dValue).toDouble();
    if((dTest==0 && ui->lineEditTgtZMM->text().length()>2)|| dTest!=ui->lineEditTgtZMM->text().toDouble()||
            dValue<0 || dValue >200.00595){
        int ret = QMessageBox::information(this, tr("Target Level (Inches) Out of Range"),
                                       tr("Please enter an integer value between 0-7.87425.\n"
                                          "This will be the altitude for the next layer.\n"),
                                       QMessageBox::Ok);
        dValue = 0;
        ui->lineEditTgtZMM->setText(QString::number(dValue));
        ui->lineEditTgtZMM->setFocus();
        ui->lineEditTgtZMM->selectAll();
        return;
    }
    setTgtAltitudeMM(dValue);
}


void B9Terminal::on_lineEditTgtZInches_editingFinished()
{
    double dValue=ui->lineEditTgtZInches->text().toDouble();
    double dTest = QString::number(dValue).toDouble();
    if((dTest==0 && ui->lineEditTgtZInches->text().length()>2)|| dTest!=ui->lineEditTgtZInches->text().toDouble()||
            dValue<0 || dValue >7.87425){
        int ret = QMessageBox::information(this, tr("Target Level (Inches) Out of Range"),
                                       tr("Please enter an integer value between 0-7.87425.\n"
                                          "This will be the altitude for the next layer.\n"),
                                       QMessageBox::Ok);
        dValue = 0;
        ui->lineEditTgtZInches->setText(QString::number(dValue));
        ui->lineEditTgtZInches->setFocus();
        ui->lineEditTgtZInches->selectAll();
        return;
    }
    setTgtAltitudeIN(dValue);
}

void B9Terminal::setTgtAltitudePU(int iTgtPU)
{
    qDebug()<<"TgtPU"<<iTgtPU;
    double dTgtMM = (iTgtPU * pPrinterComm->getPU())/100000.0;
    ui->lineEditTgtZPU->setText(QString::number(iTgtPU));
    ui->lineEditTgtZMM->setText(QString::number(dTgtMM,'g',8));
    ui->lineEditTgtZInches->setText(QString::number(dTgtMM/25.4,'g',8));
}

void B9Terminal::setTgtAltitudeMM(double dTgtMM){
    double dPU = (double)pPrinterComm->getPU()/100000.0;
    setTgtAltitudePU((int)(dTgtMM/dPU));
}

void B9Terminal::setTgtAltitudeIN(double dTgtIN){
    setTgtAltitudeMM(dTgtIN*25.4);
}

void B9Terminal::on_lineEditCurZPosInPU_returnPressed()
{
    int iValue=ui->lineEditCurZPosInPU->text().toInt();
    if(QString::number(iValue)!=ui->lineEditCurZPosInPU->text()|| iValue<0 || iValue >32000){
        // Bad Value, just return
        ui->lineEditCurZPosInPU->setText("Bad Value");
        return;
    }
    pPrinterComm->SendCmd("G"+QString::number(iValue));
    ui->lineEditCurZPosInPU->setText("In Motion...");
    ui->lineEditCurZPosInMM->setText("In Motion...");
    ui->lineEditCurZPosInInches->setText("In Motion...");
}

void B9Terminal::on_lineEditCurZPosInMM_returnPressed()
{
    double dPU = (double)pPrinterComm->getPU()/100000.0;
    double dValue=ui->lineEditCurZPosInMM->text().toDouble();
    if((dValue==0 && ui->lineEditCurZPosInMM->text().length()>1 )||dValue<0 || dValue >203.2){
        // Bad Value, just return
        ui->lineEditCurZPosInMM->setText("Bad Value");
        return;
    }

    pPrinterComm->SendCmd("G"+QString::number((int)(dValue/dPU)));
    ui->lineEditCurZPosInPU->setText("In Motion...");
    ui->lineEditCurZPosInMM->setText("In Motion...");
    ui->lineEditCurZPosInInches->setText("In Motion...");
}

void B9Terminal::on_lineEditCurZPosInInches_returnPressed()
{
    double dPU = (double)pPrinterComm->getPU()/100000.0;
    double dValue=ui->lineEditCurZPosInInches->text().toDouble();
    if((dValue==0 && ui->lineEditCurZPosInMM->text().length()>1 )||dValue<0 || dValue >8.0){
        // Bad Value, just return
        ui->lineEditCurZPosInInches->setText("Bad Value");
        return;
    }

    pPrinterComm->SendCmd("G"+QString::number((int)(dValue*25.4/dPU)));
    ui->lineEditCurZPosInPU->setText("In Motion...");
    ui->lineEditCurZPosInMM->setText("In Motion...");
    ui->lineEditCurZPosInInches->setText("In Motion...");

}

void B9Terminal::on_pushButtonStop_clicked()
{
    pPrinterComm->SendCmd("S");
    onBC_PrintReleaseCycleFinished();
    m_pVatTimer->stop();
    ui->groupBoxVAT->setEnabled(true);
}

void B9Terminal::on_checkBoxVerbose_clicked(bool checked)
{
    if(checked)pPrinterComm->SendCmd("T1"); else pPrinterComm->SendCmd("T0");
}

void B9Terminal::on_pushButtonVOpen_clicked()
{
    ui->lineEditVatPercentOpen->setText("In Motion...");
    ui->groupBoxVAT->setEnabled(false);
    pPrinterComm->SendCmd("V100");
    m_pVatTimer->start(5000); //should never take that long, even at slow speed
}

void B9Terminal::on_pushButtonVClose_clicked()
{
    ui->lineEditVatPercentOpen->setText("In Motion...");
    ui->groupBoxVAT->setEnabled(false);
    pPrinterComm->SendCmd("V0");
    m_pVatTimer->start(5000); //should never take that long, even at slow speed
}

void B9Terminal::on_lineEditVatPercentOpen_returnPressed()
{
    int iValue=ui->lineEditVatPercentOpen->text().toInt();
    if(QString::number(iValue)!=ui->lineEditVatPercentOpen->text()|| iValue<0 || iValue >125){
        // Bad Value, just return
        ui->lineEditVatPercentOpen->setText("Bad Value");
        return;
    }
    pPrinterComm->SendCmd("V"+QString::number(iValue));
    ui->lineEditVatPercentOpen->setText("In Motion...");
    ui->groupBoxVAT->setEnabled(false);
    m_pVatTimer->start(5000); //should never take that long, even at slow speed
 }
void B9Terminal::onMotionVatTimeout(){
    on_pushButtonStop_clicked(); // STOP!
    m_pVatTimer->stop();
    QMessageBox msg;
    ui->lineEditVatPercentOpen->setText("ERROR");
    msg.setText("Vat Timed out");
    msg.exec();
    ui->groupBoxVAT->setEnabled(true);
}
void B9Terminal::onBC_CurrentVatPercentOpen(int iPO){
    int iVPO = iPO;
    if (iVPO>-3 && iVPO<4)iVPO=0;
    if (iVPO>97 && iVPO<104)iVPO=100;
    ui->lineEditVatPercentOpen->setText(QString::number(iVPO));
    m_pVatTimer->stop();
    ui->groupBoxVAT->setEnabled(true);
}

void B9Terminal::onBC_PrintReleaseCycleFinished()
{
    m_pPReleaseCycleTimer->stop();
    ui->lineEditCycleStatus->setText("Cycle Complete.");
    ui->pushButtonPrintBase->setEnabled(true);
    ui->pushButtonPrintNext->setEnabled(true);
    ui->pushButtonPrintFinal->setEnabled(true);
}

void B9Terminal::onReleaseCycleTimeout()
{
    on_pushButtonStop_clicked(); // STOP!
    m_pPReleaseCycleTimer->stop();
    ui->lineEditCycleStatus->setText("ERROR: TimeOut");
    ui->pushButtonPrintBase->setEnabled(true);
    ui->pushButtonPrintNext->setEnabled(true);
    ui->pushButtonPrintFinal->setEnabled(true);
    emit signalAbortPrint("ERROR: Cycle Timed Out.  Possible reasons: Power Loss, Jammed Mechanism.");
}

void B9Terminal::on_pushButtonPrintBase_clicked()
{
    ui->lineEditCycleStatus->setText("Moving to Base...");
    ui->pushButtonPrintBase->setEnabled(false);
    ui->pushButtonPrintNext->setEnabled(false);
    ui->pushButtonPrintFinal->setEnabled(false);

    SetCycleParameters();
    pPrinterComm->SendCmd("B"+ui->lineEditTgtZPU->text());
    int iTimeout = getEstBaseCycleTime(ui->lineEditCurZPosInPU->text().toInt()-ui->lineEditTgtZPU->text().toInt(),
                                       ui->lineEditZLowerSpd->text().toInt(),ui->lineEditVatCloseSpeed->text().toInt(),
                                       ui->lineEditDelayOpenPos->text().toInt());

    m_pPReleaseCycleTimer->start(iTimeout * 1.5); // Timeout after 150% of estimated time required

}


void B9Terminal::on_pushButtonPrintNext_clicked()
{
    ui->lineEditCycleStatus->setText("Cycling to Next...");
    ui->pushButtonPrintBase->setEnabled(false);
    ui->pushButtonPrintNext->setEnabled(false);
    ui->pushButtonPrintFinal->setEnabled(false);

    SetCycleParameters();
    pPrinterComm->SendCmd("N"+ui->lineEditTgtZPU->text());
    int iTimeout = getEstNextCycleTime(ui->lineEditCurZPosInPU->text().toInt()-ui->lineEditTgtZPU->text().toInt(),
                                       ui->lineEditZRaiseSpd->text().toInt(),ui->lineEditZLowerSpd->text().toInt(),
                                       ui->lineEditVatOpenSpeed->text().toInt(),ui->lineEditVatCloseSpeed->text().toInt(),
                                       ui->lineEditDelayClosedPos->text().toInt(),ui->lineEditDelayOpenPos->text().toInt(),
                                       ui->lineEditOverLift->text().toInt());
    m_pPReleaseCycleTimer->start(iTimeout * 4); // Timeout after 400% of estimated time required
}

void B9Terminal::on_pushButtonPrintFinal_clicked()
{
    rcProjectorPwr(false);  // command projector OFF
    ui->lineEditCycleStatus->setText("Final Release...");
    ui->pushButtonPrintBase->setEnabled(false);
    ui->pushButtonPrintNext->setEnabled(false);
    ui->pushButtonPrintFinal->setEnabled(false);

    SetCycleParameters();
    pPrinterComm->SendCmd("F"+ui->lineEditTgtZPU->text());
    int iTimeout = getEstFinalCycleTime(ui->lineEditCurZPosInPU->text().toInt()-ui->lineEditTgtZPU->text().toInt(),
                                       ui->lineEditZRaiseSpd->text().toInt(),ui->lineEditVatCloseSpeed->text().toInt());
    m_pPReleaseCycleTimer->start(iTimeout * 4); // Timeout after 400% of estimated time required
}

void B9Terminal::SetCycleParameters(){
    pPrinterComm->SendCmd("D"+ui->lineEditDelayClosedPos->text()); // Breathe delay time
    pPrinterComm->SendCmd("E"+ui->lineEditDelayOpenPos->text()); // Settle delay time

    pPrinterComm->SendCmd("J"+ui->lineEditOverLift->text()); // Overlift Raise Gap

    pPrinterComm->SendCmd("K"+ui->lineEditZRaiseSpd->text()); // Raise Speed
    pPrinterComm->SendCmd("L"+ui->lineEditZLowerSpd->text()); // Lower Speed

    pPrinterComm->SendCmd("W"+ui->lineEditVatOpenSpeed->text()); // Vat open speed
    pPrinterComm->SendCmd("X"+ui->lineEditVatCloseSpeed->text()); // Vat close speed
}


void B9Terminal::rcProjectorPwr(bool bPwrOn){
    on_pushButtonProjPower_toggled(bPwrOn);
}

void B9Terminal::rcResetHomePos(){
    on_pushButtonCmdReset_clicked();
}

int B9Terminal::getZMoveTime(int iDelta, int iSpd){
    // returns time to travel iDelta distance in milliseconds
    if(iDelta==0)return 0;
    double dPUms; // printer units per millisecond
    dPUms = ((double)iSpd/100.0)*130.0 + 10.0;
    dPUms *= 200.0; // PU per minute
    dPUms /= 60; // PU per second
    dPUms /= 1000; // PU per millisecond
    return (int)(double(iDelta)/dPUms);
}

int B9Terminal::getVatMoveTime(int iSpeed){
    double dPercent = (double)iSpeed/100.0;
    return 900 - dPercent*150.0;
}

int B9Terminal::getEstBaseCycleTime(int iDelta, int iDwnSpd, int iClsSpd, int iSettle){
    // Time to move -iDelta
    int iTimeReq = getZMoveTime(iDelta, iDwnSpd);
    // Plus time to close vat, ~1000 ms
    iTimeReq += getVatMoveTime(iClsSpd);
    // Plus settle time;
    iTimeReq += iSettle;
    return iTimeReq;
}

int B9Terminal::getEstNextCycleTime(int iDelta, int iUpSpd, int iDwnSpd, int iOpnSpd, int iClsSpd, int iBreathe, int iSettle, int iGap){
    // Time to move +iDelta + iGap, up and down
    int iTimeReq = getZMoveTime(iDelta+iGap, iUpSpd);
    iTimeReq += getZMoveTime(iDelta+iGap, iDwnSpd);
    // Plus time to close + open the vat
    iTimeReq += getVatMoveTime(iClsSpd)+getVatMoveTime(iOpnSpd);
    // Plus breathe & settle time;
    iTimeReq += iBreathe + iSettle;
    return iTimeReq;
}

int B9Terminal::getEstFinalCycleTime(int iDelta, int iUpSpd, int iClsSpd){
    // Time to move +iDelta up
    int iTimeReq = getZMoveTime(iDelta, iUpSpd);
    // Plus time to close the vat
    iTimeReq += getVatMoveTime(iClsSpd);
    return iTimeReq;
}

void B9Terminal::onScreenCountChanged(int iCount){
    if(pProjector) delete pProjector;
    pProjector = new B9Projector(true, this);
    makeProjectorConnections();
    int i=0;
    int screenCount = m_pDesktop->screenCount();
    QRect screenGeometry;
    for(i=screenCount-1;i>= 0;i--) {
        screenGeometry = m_pDesktop->screenGeometry(i);
        if(screenGeometry.width() == 1024 && screenGeometry.height() == 768) {
            //Found the projector
            //TODO use the calibrated resolution settings from the user input for xy
            break;
        }
    }
    if(i<=0)m_bPrimaryScreen = true; else m_bPrimaryScreen = false;
    pProjector->setShowGrid(true);
    pProjector->setCPJ(NULL);

    emit sendStatusMsg("B9Creator - Idle");
    pProjector->setGeometry(screenGeometry);
    if(!m_bPrimaryScreen){
        pProjector->showFullScreen(); // Only show it if it is a secondary monitor
        activateWindow(); // if not using primary monitor, take focus back to here.
    }
    else if(pPrinterComm->getProjectorStatus() != B9PrinterStatus::PS_OFF &&
            pPrinterComm->getProjectorStatus() != B9PrinterStatus::PS_UNKNOWN) {
        // if the projector is not turned off, we better put up the blank screen now!
        pProjector->showFullScreen();
    }
}

void B9Terminal::getKey(int iKey)
{
    if(!m_bPrimaryScreen)return; // Ignore keystrokes from the print window unless we're using the primary monitor

//	if(m_iPrintState!=PRINT_NO){
//		if(m_iPrintState==PRINT_WAITFORP && iKey==80){
//			exposeLayer();
//			breatheLayer();
//		}
//		return;
//	}

    QMessageBox msgBox;
    msgBox.setText(QString::number(iKey));
    msgBox.exec();

/*
    switch(iKey){
    case 85:		// 'U' Increase Y Offset
        ui.sliderYoff->setValue(ui.sliderYoff->value()-2);
        break;
    case 68:		// 'D' Decrease Y Offset
        ui.sliderYoff->setValue(ui.sliderYoff->value()+2);
        break;
    case 82:		// 'R' Increase X Offset
        ui.sliderXoff->setValue(ui.sliderXoff->value()+2);
        break;
    case 76:		// 'L' Decrease X Offset
        ui.sliderXoff->setValue(ui.sliderXoff->value()-2);
        break;
    case 66:		// 'B' Blank Screen
        emit sendCPJ(NULL);
        break;
    case 70:		// 'F' Toggle Full Screen
        if(m_bPrimaryScreen){
            if(!pProjector->isFullScreen())
                pProjector->setWindowState(Qt::WindowFullScreen);
            else
                pProjector->setWindowState(Qt::WindowNoState);
                //pProjector->hide();
        }
        break;
    case 71:		// 'G' Toggle Grid
        if(ui.checkBoxShowGrid->isChecked())
            updateGrid(false);
        else
            updateGrid(true);
        break;
    case 16777216:	// Escape Key
            if(m_bPrimaryScreen) hideProjector();
        break;
    case 16777232: // HOME
        ui.SliderCurSlice->setValue(homeIndex());
        break;
    case 16777233: // END
        ui.SliderCurSlice->setValue(endIndex());
        break;
    case 16777238: // Page Up
        ui.SliderCurSlice->setValue(getNextIndex(ui.SliderCurSlice->value(),10));
        break;
    case 16777239: // Page Down
        ui.SliderCurSlice->setValue(getNextIndex(ui.SliderCurSlice->value(),-10));
        break;
    case 16777235: // Arrow Up
    case 16777236: // Arrow Right
        ui.SliderCurSlice->setValue(getNextIndex(ui.SliderCurSlice->value(),1));
        break;
    case 16777237: // Arrow Down
    case 16777234: // Arrow Left
        ui.SliderCurSlice->setValue(getNextIndex(ui.SliderCurSlice->value(),-1));
        break;
    default:
        break;
    }
*/
}
