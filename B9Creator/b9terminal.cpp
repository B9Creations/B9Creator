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

    ui->setupUi(this);
    ui->commStatus->setText("Searching for B9Creator...");

    qDebug() << "Terminal Start";

    // Always set up the B9PrinterComm in the Terminal constructor
    pPrinterComm = new B9PrinterComm;
    pPrinterComm->enableBlankCloning(true); // Allow for firmware update of suspected "blank" B9Creator Arduino's

    connect(pPrinterComm,SIGNAL(updateConnectionStatus(QString)), this, SLOT(onUpdateConnectionStatus(QString)));
    connect(pPrinterComm,SIGNAL(BC_ConnectionStatusDetailed(QString)), this, SLOT(onBC_ConnectionStatusDetailed(QString)));
    connect(pPrinterComm,SIGNAL(BC_LostCOMM()),this,SLOT(onBC_LostCOMM()));

    //connect(pPrinterComm,SIGNAL(BC_RawData(QString)), this, SLOT(onUpdatePrinterComm(QString)));
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
    m_bResetInProgress = false;
    m_pResetTimer = new QTimer(this);
    connect(m_pResetTimer, SIGNAL(timeout()), this, SLOT(onMotionResetTimeout()));
    connect(pPrinterComm, SIGNAL(BC_HomeFound()), this, SLOT(onMotionResetComplete()));
    connect(pPrinterComm, SIGNAL(BC_CurrentZPosInPU(int)), this, SLOT(onBC_CurrentZPosInPU(int)));
    connect(pPrinterComm, SIGNAL(BC_CurrentVatPercentOpen(int)), this, SLOT(onBC_CurrentVatPercentOpen(int)));
}

B9Terminal::~B9Terminal()
{
    delete ui;
    delete pPrinterComm;
    qDebug() << "Terminal End";
}

void B9Terminal::setEnabledWarned(){
    if(isHidden())return;
    if(!m_bWaiverPresented){
        // Present Waiver
        m_bWaiverPresented = true;
        m_bWaiverAccepted = false;
        int ret = QMessageBox::information(this, tr("Enable Terminal Control?"),
                                       tr("Warning: Manual operation can damage the VAT coating.\n"
                                          "If your VAT is installed and empty of resin care must be\n"
                                          "taken to ensure it is not damaged.  Operation is only safe\n"
                                          "with either the VAT removed, or the Sweeper and Build Table removed.\n"
                                          "Do you want to enable manual control?"),
                                       QMessageBox::Yes | QMessageBox::No
                                       | QMessageBox::Cancel);
        if(ret==QMessageBox::Cancel){hide();return;}
        else if(ret==QMessageBox::Yes)m_bWaiverAccepted=true;
    }
    ui->groupBoxMain->setEnabled(m_bWaiverAccepted&&pPrinterComm->isConnected());
}

void B9Terminal::hideEvent(QHideEvent *event)
{
    emit eventHiding();
    event->accept();
    //m_bWaiverPresented = false;
}

void B9Terminal::sendCommand()
{
    pPrinterComm->SendCmd(ui->lineEditCommand->text());
    ui->lineEditCommand->clear();
}

void B9Terminal::onBC_LostCOMM(){
    //TODO handle the loss of comm if we are printing.
    //Broadcast an alert

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
    ui->textEditCommOut->insertPlainText(sText);
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
    msg.exec();
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
        break;
    case B9PrinterStatus::PS_FAIL:
        ui->pushButtonProjPower->setEnabled(false);
        sText = "FAIL";
        break;
    case B9PrinterStatus::PS_UNKNOWN:
        ui->pushButtonProjPower->setEnabled(true);
    default:
        sText = "UNKNOWN";
        break;
    }
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
    int iTimeoutEstimate = 10000;
    //iTimeoutEstimate = pPrinterComm->m_Status.

    ui->groupBoxMain->setEnabled(false);
    // Remote activation of Reset (Find Home) Motion
    if(m_bResetInProgress) return;
    m_pResetTimer->start(iTimeoutEstimate);
    m_bResetInProgress = true;
    pPrinterComm->SendCmd("R");

}
void B9Terminal::onMotionResetComplete()
{
    ui->groupBoxMain->setEnabled(true);
    m_pResetTimer->stop();
    m_bResetInProgress = false;
    QMessageBox msg;
    msg.setText("Reset complete");
    msg.exec();
}

void B9Terminal::onMotionResetTimeout(){
    ui->groupBoxMain->setEnabled(true);
    m_pResetTimer->stop();
QMessageBox msg;
msg.setText("Timed out");
msg.exec();
m_bResetInProgress=false;
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

void B9Terminal::onBC_CurrentVatPercentOpen(int iPO){
    int iVPO = iPO;
    if (iVPO>-3 & iVPO<4)iVPO=0;
    if (iVPO>97 & iVPO<104)iVPO=100;
    ui->lineEditVatPercentOpen->setText(QString::number(iVPO));
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
    if(QString::number(iValue)!=ui->lineEditCurZPosInPU->text()|| iValue<0 || iValue >31497){
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
    if((dValue==0 && ui->lineEditCurZPosInMM->text().length()>1 )||dValue<0 || dValue >200.00595){
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
    if((dValue==0 && ui->lineEditCurZPosInMM->text().length()>1 )||dValue<0 || dValue >7.87425){
        // Bad Value, just return
        ui->lineEditCurZPosInInches->setText("Bad Value");
        return;
    }

    pPrinterComm->SendCmd("G"+QString::number((int)(dValue*25.4/dPU)));
    ui->lineEditCurZPosInPU->setText("In Motion...");
    ui->lineEditCurZPosInMM->setText("In Motion...");
    ui->lineEditCurZPosInInches->setText("In Motion...");

}
