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
//  This file is part of B9Creator
//
//    B9Creator is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    B9Creator is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with B9Creator .  If not, see <http://www.gnu.org/licenses/>.
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
#include <QSettings>
#include "b9terminal.h"
#include "ui_b9terminal.h"
#include "dlgcyclesettings.h"
#include "dlgmaterialsmanager.h"

void PCycleSettings::updateValues()
{
    DlgCycleSettings dlg(this);
    dlg.exec();
}

void PCycleSettings::loadSettings()
{
    QSettings settings;
    m_iRSpd1 = settings.value("RSpd1",85).toInt();
    m_iLSpd1 = settings.value("LSpd1",85).toInt();
    m_iCloseSpd1 = settings.value("CloseSpd1",100).toInt();
    m_iOpenSpd1 = settings.value("OpenSpd1",25).toInt();
    m_dBreatheClosed1 = settings.value("BreatheClosed1",1).toDouble();
    m_dSettleOpen1 = settings.value("SettleOpen1",1).toDouble();
    m_dOverLift1 = settings.value("OverLift1",3).toDouble();

    m_iRSpd2 = settings.value("RSpd2",85).toInt();
    m_iLSpd2 = settings.value("LSpd2",85).toInt();
    m_iCloseSpd2 = settings.value("CloseSpd2",100).toInt();
    m_iOpenSpd2 = settings.value("OpenSpd2",100).toInt();
    m_dBreatheClosed2 = settings.value("BreatheClosed2",0).toDouble();
    m_dSettleOpen2 = settings.value("SettleOpen2",0).toDouble();
    m_dOverLift2 = settings.value("OverLift2",0).toDouble();

    m_dBTClearInMM = settings.value("BTClearInMM",5.0).toDouble();

    m_dHardZDownMM = settings.value("HardDownZMM",0.9525).toDouble();
    m_dZFlushMM    = settings.value("ZFlushMM",   0.7620).toDouble();
}

void PCycleSettings::saveSettings()
{
    QSettings settings;
    settings.setValue("RSpd1",m_iRSpd1);
    settings.setValue("LSpd1",m_iLSpd1);
    settings.setValue("CloseSpd1",m_iCloseSpd1);
    settings.setValue("OpenSpd1",m_iOpenSpd1);
    settings.setValue("BreatheClosed1",m_dBreatheClosed1);
    settings.setValue("SettleOpen1",m_dSettleOpen1);
    settings.setValue("OverLift1",m_dOverLift1);

    settings.setValue("RSpd2",m_iRSpd2);
    settings.setValue("LSpd2",m_iLSpd2);
    settings.setValue("CloseSpd2",m_iCloseSpd2);
    settings.setValue("OpenSpd2",m_iOpenSpd2);
    settings.setValue("BreatheClosed2",m_dBreatheClosed2);
    settings.setValue("SettleOpen2",m_dSettleOpen2);
    settings.setValue("OverLift2",m_dOverLift2);

    settings.setValue("BTClearInMM",m_dBTClearInMM);

    settings.setValue("HardDownZMM",m_dHardZDownMM);
    settings.setValue("ZFlushMM",   m_dZFlushMM   );
}

void PCycleSettings::setFactorySettings()
{
    m_iRSpd1 = m_iLSpd1 = 85;
    m_iRSpd2 = m_iLSpd2 = 85;
    m_iOpenSpd1 = 25;
    m_iCloseSpd1 = 100;
    m_iOpenSpd2 = m_iCloseSpd2 = 100;
    m_dBreatheClosed1 = 1.0;
    m_dSettleOpen1 = 3.0;
    m_dBreatheClosed2 = 0.0;
    m_dSettleOpen2 = 0.5;
    m_dOverLift1 = 3.0;
    m_dOverLift2 = 0.0;
    m_dBTClearInMM = 5.0;
    m_dHardZDownMM = 0.9525;
    m_dZFlushMM = 0.7620;
}

B9Terminal::B9Terminal(QWidget *parent, Qt::WFlags flags) :
    QWidget(parent, flags),
    ui(new Ui::B9Terminal)
{
    setWindowFlags(Qt::WindowContextHelpButtonHint);
    m_bWaiverPresented = false;
    m_bWaiverAccepted = false;
    m_bWavierActive = false;
    m_bNeedsWarned = true;
    m_iFillLevel = -1;

    ui->setupUi(this);
    ui->commStatus->setText("Searching for B9Creator...");

    qDebug() << "Terminal Start";

    m_pCatalog = new B9MatCat;
    onBC_ModelInfo("B9C1");

    pSettings = new PCycleSettings;
    resetLastSentCycleSettings();

    // Always set up the B9PrinterComm in the Terminal constructor
    pPrinterComm = new B9PrinterComm;
    pPrinterComm->enableBlankCloning(true); // Allow for firmware update of suspected "blank" B9Creator Arduino's

    // Always set up the B9Projector in the Terminal constructor
    m_pDesktop = QApplication::desktop();
    pProjector = NULL;
    m_bPrimaryScreen = false;
    m_bPrintPreview = false;
    m_bUsePrimaryMonitor = false;

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
    connect(pPrinterComm, SIGNAL(BC_HalfLife(int)), this, SLOT(onBC_HalfLife(int)));
    connect(pPrinterComm, SIGNAL(BC_NativeX(int)), this, SLOT(onBC_NativeX(int)));
    connect(pPrinterComm, SIGNAL(BC_NativeY(int)), this, SLOT(onBC_NativeY(int)));
    connect(pPrinterComm, SIGNAL(BC_XYPixelSize(int)), this, SLOT(onBC_XYPixelSize(int)));


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
    delete pProjector;
    delete pPrinterComm;
    qDebug() << "Terminal End";
}

void B9Terminal::resetLastSentCycleSettings(){
    m_iD=m_iE=m_iJ=m_iK=m_iL=m_iW=m_iX = -1;
}

void B9Terminal::dlgEditMatCat()
{
    DlgMaterialsManager dlgMatMan(m_pCatalog,0);

    QSettings settings;
    int indexMat=0;
    for(int i=0; i<m_pCatalog->getMaterialCount(); i++) {
        if(settings.value("CurrentMaterialLabel","B9R-1-Red").toString()==m_pCatalog->getMaterialLabel(i)) {
            indexMat = i;
            break;
        }
    }
    m_pCatalog->setCurMatIndex(indexMat);

    int indexXY = 0;
    if(settings.value("CurrentXYLabel","100").toString()=="75 (µm)")indexXY=1;
    else if(settings.value("CurrentXYLabel","100").toString()=="100 (µm)")indexXY = 2;
    dlgMatMan.setXY(indexXY);
    m_pCatalog->setCurXYIndex(indexXY);
    dlgMatMan.exec();
}

void B9Terminal::on_pushButtonModMatCat_clicked()
{
    dlgEditMatCat();
}


void B9Terminal::makeProjectorConnections()
{
    // should be called any time we create a new projector object
    if(pProjector==NULL)return;
    connect(pProjector, SIGNAL(keyReleased(int)),this, SLOT(getKey(int)));
    connect(this, SIGNAL(sendStatusMsg(QString)),pProjector, SLOT(setStatusMsg(QString)));
    connect(this, SIGNAL(sendGrid(bool)),pProjector, SLOT(setShowGrid(bool)));
    connect(this, SIGNAL(sendCPJ(CrushedPrintJob*)),pProjector, SLOT(setCPJ(CrushedPrintJob*)));
    connect(this, SIGNAL(sendXoff(int)),pProjector, SLOT(setXoff(int)));
    connect(this, SIGNAL(sendYoff(int)),pProjector, SLOT(setYoff(int)));
}


void B9Terminal::warnSingleMonitor(){
    if(m_bPrimaryScreen && m_bNeedsWarned){
        m_bNeedsWarned = false;
        QMessageBox msg;
        msg.setWindowTitle("Projector Connection?");
        msg.setText("WARNING:  The printer's projector is not connected to a secondary video output.  Please check that all connections (VGA or HDMI) and system display settings are correct.  Disregard this message if your system has only one video output and will utilize a splitter to provide video output to both monitor and Projector.");
        if(isEnabled())msg.exec();
    }
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

            if(ret==QMessageBox::Cancel){m_bWavierActive = false;m_bWaiverPresented=false;hide();return;}
            else if(ret==QMessageBox::Yes)m_bWaiverAccepted=true;
            warnSingleMonitor();
            m_bWavierActive = false;
        }
    }
    ui->groupBoxMain->setEnabled(m_bWaiverAccepted&&pPrinterComm->isConnected()&&ui->lineEditNeedsInit->text()!="Seeking");
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
    if(!isEnabled())emit signalAbortPrint("ERROR: Lost Printer Connection.  Possible reasons: Power Loss, USB cord unplugged.");
    qDebug() << "BC_LostCOMM signal received.";
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
    html = ui->textEditCommOut->toHtml();
    ui->textEditCommOut->clear();
    ui->textEditCommOut->insertHtml(html.right(2000));
    ui->textEditCommOut->setAlignment(Qt::AlignBottom);
}
void B9Terminal::onUpdateRAWPrinterComm(QString sText)
{
    QString html = "<font color=\"Blue\">" + sText + "</font><br>";
    ui->textEditCommOut->insertHtml(html);
    html = ui->textEditCommOut->toHtml();
    ui->textEditCommOut->clear();
    ui->textEditCommOut->insertHtml(html.right(2000));
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
    emit sendStatusMsg("B9Creator - Projector status: TURN ON");

    // We always close the vat when powering up
    if(checked){
        emit onBC_CurrentVatPercentOpen(0);
        emit on_spinBoxVatPercentOpen_editingFinished();
    }
}

void B9Terminal::setProjectorPowerCmd(bool bPwrFlag){
    pPrinterComm->setProjectorPowerCmd(bPwrFlag);
}


void B9Terminal::onBC_ProjStatusFAIL()
{
    onBC_ProjStatusChanged();
    on_pushButtonProjPower_toggled(pPrinterComm->isProjectorPowerCmdOn());
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
        sText = "TURN ON";
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
        if(!isEnabled())emit signalAbortPrint("Timed out while attempting to turn on projector.  Check Projector's Power Cord and RS-232 cable.");
        break;
    case B9PrinterStatus::PS_FAIL:
        ui->pushButtonProjPower->setEnabled(false);
        sText = "FAIL";
        if(!isEnabled())emit signalAbortPrint("Lost Communications with Projector.  Possible Causes:  Manually powered off, Power Failure, Cord Disconnected or Projector Lamp Failure");
        break;
    case B9PrinterStatus::PS_UNKNOWN:
        ui->pushButtonProjPower->setEnabled(true);
    default:
        sText = "UNKNOWN";
        break;
    }

    // Update the power button state
    if(pPrinterComm->isProjectorPowerCmdOn()){
        pProjector->show();
        activateWindow();
        ui->pushButtonProjPower->setChecked(true);
        ui->pushButtonProjPower->setText("ON");
    }
    else{
        pProjector->hide();
        activateWindow();
        ui->pushButtonProjPower->setChecked(false);
        ui->pushButtonProjPower->setText("OFF");
    }

    if(!isEnabled())emit sendStatusMsg("B9Creator - Projector status: "+sText);
    if(!isEnabled())emit updateProjectorStatus(sText);
    if(!isEnabled())emit updateProjector(pPrinterComm->getProjectorStatus());

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
    int iTimeoutEstimate = 80000; // 80 seconds (should never take longer than 75 secs from upper limit)

    ui->groupBoxMain->setEnabled(false);
    ui->lineEditNeedsInit->setText("Seeking");
    // Remote activation of Reset (Find Home) Motion
    m_pResetTimer->start(iTimeoutEstimate);
    pPrinterComm->SendCmd("R");
    resetLastSentCycleSettings();
}

void B9Terminal::onMotionResetComplete()
{    
    ui->groupBoxMain->setEnabled(true);
    if(pPrinterComm->getHomeStatus()==B9PrinterStatus::HS_FOUND) ui->lineEditNeedsInit->setText("No");
    else if(pPrinterComm->getHomeStatus()==B9PrinterStatus::HS_UNKNOWN) ui->lineEditNeedsInit->setText("Yes");
    else ui->lineEditNeedsInit->setText("Seeking");
    ui->lineEditZDiff->setText(QString::number(pPrinterComm->getLastHomeDiff()).toAscii());
    m_pResetTimer->stop();

    // Check for post reset go to fill command
    if(m_iFillLevel>=0){
        pPrinterComm->SendCmd("G"+QString::number(m_iFillLevel));
        m_iFillLevel=-1;
    }
}

void B9Terminal::onMotionResetTimeout(){
    ui->groupBoxMain->setEnabled(true);
    m_pResetTimer->stop();
    QMessageBox msg;
    msg.setText("ERROR: TIMEOUT attempting to locate home position.  Check connections.");
    if(isEnabled())msg.exec();
}

void B9Terminal::onBC_ModelInfo(QString sModel){
    m_sModelName = sModel;
    m_pCatalog->load(m_sModelName);
    ui->lineEditModelInfo->setText(m_sModelName);
    resetLastSentCycleSettings();
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

void B9Terminal::onBC_HalfLife(int iHL){
    ui->lineEditHalfLife->setText(QString::number(iHL));
}

void B9Terminal::onBC_NativeX(int iNX){
    ui->lineEditNativeX->setText(QString::number(iNX));
}

void B9Terminal::onBC_NativeY(int iNY){
    ui->lineEditNativeY->setText(QString::number(iNY));
    if(pProjector == NULL)emit onScreenCountChanged();
}

void B9Terminal::onBC_XYPixelSize(int iPS){
    int i=2;
    if(iPS==50)i=0;
    else if(iPS==75)i=1;
    ui->comboBoxXPPixelSize->setCurrentIndex(i);
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

void B9Terminal::on_lineEditTgtZPU_editingFinished()
{
    int iValue=ui->lineEditTgtZPU->text().toInt();
    if(QString::number(iValue)!=ui->lineEditTgtZPU->text()||
            iValue<0 || iValue >31497){
        QMessageBox::information(this, tr("Target Level (Z steps) Out of Range"),
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
        QMessageBox::information(this, tr("Target Level (Inches) Out of Range"),
                     tr("Please enter an integer value between 0-7.87425.\n"
                     "This will be the altitude for the next layer.\n"),QMessageBox::Ok);
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
        QMessageBox::information(this, tr("Target Level (Inches) Out of Range"),
                     tr("Please enter an integer value between 0-7.87425.\n"
                        "This will be the altitude for the next layer.\n"),QMessageBox::Ok);
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
    m_pPReleaseCycleTimer->stop();
    m_pVatTimer->stop();
    pPrinterComm->SendCmd("S");
    ui->lineEditCycleStatus->setText("Cycle Stopped.");
    ui->pushButtonPrintBase->setEnabled(true);
    ui->pushButtonPrintNext->setEnabled(true);
    ui->pushButtonPrintFinal->setEnabled(true);
    ui->groupBoxVAT->setEnabled(true);
    resetLastSentCycleSettings();
}

void B9Terminal::on_checkBoxVerbose_clicked(bool checked)
{
    if(checked)pPrinterComm->SendCmd("T1"); else pPrinterComm->SendCmd("T0");
}

void B9Terminal::on_spinBoxVatPercentOpen_editingFinished()
{
    if(m_pVatTimer->isActive()) return;
    m_pVatTimer->start(3000); //should never take that long, even at slow speed
    int iValue = ui->spinBoxVatPercentOpen->value();
    ui->groupBoxVAT->setEnabled(false);
    pPrinterComm->SendCmd("V"+QString::number(iValue));
}

void B9Terminal::on_pushButtonVOpen_clicked()
{
    ui->groupBoxVAT->setEnabled(false);
    m_pVatTimer->start(3000); //should never take that long, even at slow speed
    pPrinterComm->SendCmd("V100");
}

void B9Terminal::on_pushButtonVClose_clicked()
{
    ui->groupBoxVAT->setEnabled(false);
    m_pVatTimer->start(3000); //should never take that long, even at slow speed
    pPrinterComm->SendCmd("V0");
}

void B9Terminal::onMotionVatTimeout(){
    m_pVatTimer->stop();
    on_pushButtonStop_clicked(); // STOP!
    QMessageBox msg;
    msg.setText("Vat Timed out");
    if(isEnabled())msg.exec();
    ui->groupBoxVAT->setEnabled(true);
}
void B9Terminal::onBC_CurrentVatPercentOpen(int iPO){
    m_pVatTimer->stop();
    int iVPO = iPO;
    if (iVPO>-3 && iVPO<4)iVPO=0;
    if (iVPO>97 && iVPO<104)iVPO=100;
    ui->spinBoxVatPercentOpen->setValue(iVPO);
    ui->groupBoxVAT->setEnabled(true);
}

void B9Terminal::onBC_PrintReleaseCycleFinished()
{
    m_pPReleaseCycleTimer->stop();
    ui->lineEditCycleStatus->setText("Cycle Complete.");
    ui->pushButtonPrintBase->setEnabled(true);
    ui->pushButtonPrintNext->setEnabled(true);
    ui->pushButtonPrintFinal->setEnabled(true);
    emit PrintReleaseCycleFinished();
}

void B9Terminal::onReleaseCycleTimeout()
{
    m_pPReleaseCycleTimer->stop();
    if(true){  // Set to true if we wish to abort due to the timeout.
        qDebug()<<"Release Cycle Timeout.  Possible reasons: Power Loss, Jammed Mechanism.";
        on_pushButtonStop_clicked(); // STOP!
        ui->lineEditCycleStatus->setText("ERROR: TimeOut");
        ui->pushButtonPrintBase->setEnabled(true);
        ui->pushButtonPrintNext->setEnabled(true);
        ui->pushButtonPrintFinal->setEnabled(true);
        if(!isEnabled())emit signalAbortPrint("ERROR: Cycle Timed Out.  Possible reasons: Power Loss, Jammed Mechanism.");
        return;
    }
    else {
        qDebug()<<"Release Cycle Timeout.  Possible reasons: Power Loss, Jammed Mechanism. IGNORED";
        qDebug()<<"Serial Port Last Error:  "<<pPrinterComm->errorString();
    }
}

void B9Terminal::on_pushButtonPrintBase_clicked()
{
    ui->lineEditCycleStatus->setText("Moving to Base...");
    ui->pushButtonPrintBase->setEnabled(false);
    ui->pushButtonPrintNext->setEnabled(false);
    ui->pushButtonPrintFinal->setEnabled(false);
    resetLastSentCycleSettings();
    SetCycleParameters();
    int iTimeout = getEstBaseCycleTime(ui->lineEditCurZPosInPU->text().toInt(), ui->lineEditTgtZPU->text().toInt());
    pPrinterComm->SendCmd("B"+ui->lineEditTgtZPU->text());
    m_pPReleaseCycleTimer->start(iTimeout * 2.0); // Timeout after 200% of estimated time required
}

void B9Terminal::on_pushButtonPrintNext_clicked()
{
    ui->lineEditCycleStatus->setText("Cycling to Next...");
    ui->pushButtonPrintBase->setEnabled(false);
    ui->pushButtonPrintNext->setEnabled(false);
    ui->pushButtonPrintFinal->setEnabled(false);

    SetCycleParameters();
    int iTimeout = getEstNextCycleTime(ui->lineEditCurZPosInPU->text().toInt(), ui->lineEditTgtZPU->text().toInt());
    pPrinterComm->SendCmd("N"+ui->lineEditTgtZPU->text());
    m_pPReleaseCycleTimer->start(iTimeout * 2.0); // Timeout after 200% of estimated time required
}

void B9Terminal::on_pushButtonPrintFinal_clicked()
{
    rcProjectorPwr(false);  // command projector OFF
    ui->lineEditCycleStatus->setText("Final Release...");
    ui->pushButtonPrintBase->setEnabled(false);
    ui->pushButtonPrintNext->setEnabled(false);
    ui->pushButtonPrintFinal->setEnabled(false);
    SetCycleParameters();
    int iTimeout = getEstFinalCycleTime(ui->lineEditCurZPosInPU->text().toInt(), ui->lineEditTgtZPU->text().toInt());
    pPrinterComm->SendCmd("F"+ui->lineEditTgtZPU->text());
    m_pPReleaseCycleTimer->start(iTimeout * 2.0); // Timeout after 200% of estimated time required
}

void B9Terminal::SetCycleParameters(){
    int iD, iE, iJ, iK, iL, iW, iX;
    if(pSettings->m_dBTClearInMM*100000/pPrinterComm->getPU()>ui->lineEditTgtZPU->text().toInt()){
        iD = (int)(pSettings->m_dBreatheClosed1*1000.0); // Breathe delay time
        iE = (int)(pSettings->m_dSettleOpen1*1000.0); // Settle delay time
        iJ = (int)(pSettings->m_dOverLift1*100000.0/(double)pPrinterComm->getPU()); // Overlift Raise Gap coverted to PU
        iK = pSettings->m_iRSpd1;  // Raise Speed
        iL = pSettings->m_iLSpd1;  // Lower Speed
        iW = pSettings->m_iOpenSpd1;  // Vat open speed
        iX = pSettings->m_iCloseSpd1; // Vat close speed
    }
    else{
        iD = (int)(pSettings->m_dBreatheClosed2*1000.0); // Breathe delay time
        iE = (int)(pSettings->m_dSettleOpen2*1000.0); // Settle delay time
        iJ = (int)(pSettings->m_dOverLift2*100000.0/(double)pPrinterComm->getPU()); // Overlift Raise Gap coverted to PU
        iK = pSettings->m_iRSpd2;  // Raise Speed
        iL = pSettings->m_iLSpd2;  // Lower Speed
        iW = pSettings->m_iOpenSpd2;  // Vat open speed
        iX = pSettings->m_iCloseSpd2; // Vat close speed
    }
    if(iD!=m_iD){pPrinterComm->SendCmd("D"+QString::number(iD)); m_iD = iD;}
    if(iE!=m_iE){pPrinterComm->SendCmd("E"+QString::number(iE)); m_iE = iE;}
    if(iJ!=m_iJ){pPrinterComm->SendCmd("J"+QString::number(iJ)); m_iJ = iJ;}
    if(iK!=m_iK){pPrinterComm->SendCmd("K"+QString::number(iK)); m_iK = iK;}
    if(iL!=m_iL){pPrinterComm->SendCmd("L"+QString::number(iL)); m_iL = iL;}
    if(iW!=m_iW){pPrinterComm->SendCmd("W"+QString::number(iW)); m_iW = iW;}
    if(iX!=m_iX){pPrinterComm->SendCmd("X"+QString::number(iX)); m_iX = iX;}
}

void B9Terminal::rcProjectorPwr(bool bPwrOn){
    on_pushButtonProjPower_toggled(bPwrOn);
}

void B9Terminal::rcResetHomePos(){
    on_pushButtonCmdReset_clicked();
}

void B9Terminal::rcGotoFillAfterReset(int iFillLevel){
    m_iFillLevel = iFillLevel;
}

void B9Terminal::rcResetCurrentPositionPU(int iCurPos){
    pPrinterComm->SendCmd("O"+QString::number(iCurPos));
}

void B9Terminal::rcBasePrint(double dBaseMM)
{
    setTgtAltitudeMM(dBaseMM);
    on_pushButtonPrintBase_clicked();
}

void B9Terminal::rcNextPrint(double dNextMM)
{
    setTgtAltitudeMM(dNextMM);
    on_pushButtonPrintNext_clicked();
}

void B9Terminal::rcSTOP()
{
    on_pushButtonStop_clicked();
}

void B9Terminal::rcCloseVat()
{
    on_pushButtonVClose_clicked();
}

void B9Terminal::rcSetWarmUpDelay(int iDelayMS)
{
    pPrinterComm->setWarmUpDelay(iDelayMS);
}


void B9Terminal::rcFinishPrint(double dDeltaMM)
{
    // Calculates final position based on current + dDeltaMM
    int newPos = dDeltaMM*100000.0/(double)pPrinterComm->getPU();
    newPos += ui->lineEditCurZPosInPU->text().toInt();
    if(newPos>ui->lineEditUpperZLimPU->text().toInt())newPos = ui->lineEditUpperZLimPU->text().toInt();
    setTgtAltitudePU(newPos);
    on_pushButtonPrintFinal_clicked();
}

void B9Terminal::rcSetCPJ(CrushedPrintJob *pCPJ)
{
    // Set the pointer to the CMB to be displayed, NULL if blank
    emit sendCPJ(pCPJ);
}

void B9Terminal::rcSetProjMessage(QString sMsg)
{
    if(pProjector==NULL)return;
    // Pass along the message for the projector screen
    pProjector->setStatusMsg("B9Creator  -  "+sMsg);
}

int B9Terminal::getLampAdjustedExposureTime(int iBaseTimeMS)
{
    if(pPrinterComm==NULL||pPrinterComm->getLampHrs()<0||pPrinterComm->getHalfLife()<0)return iBaseTimeMS;

    //  dLife = 0.0 at zero lamp hours and 1.0 at or above halflife hours.
    //  We multiply the base time by dLife and return the original amount + the product.
    //  So at Halflife, we've doubled the standard exposure time.
    double dLife = (double)pPrinterComm->getLampHrs()/(double)pPrinterComm->getHalfLife();
    if(dLife > 1.0)dLife = 1.0; // Limit to 100% the amount of applied bulb degradation (reached at HalfLife)
    return iBaseTimeMS + (double)iBaseTimeMS*dLife;
}

QTime B9Terminal::getEstCompleteTime(int iCurLayer, int iTotLayers, double dLayerThicknessMM, int iExposeMS)
{
    return QTime::currentTime().addMSecs(getEstCompleteTimeMS(iCurLayer, iTotLayers, dLayerThicknessMM, iExposeMS));
}

int B9Terminal::getEstCompleteTimeMS(int iCurLayer, int iTotLayers, double dLayerThicknessMM, int iExposeMS)
{
    //return estimated completion time
    int iTransitionPointLayer = (int)(pSettings->m_dBTClearInMM/dLayerThicknessMM);

    int iLowerCount = (int)(pSettings->m_dBTClearInMM/dLayerThicknessMM);
    int iUpperCount = iTotLayers - iLowerCount;

    if(iLowerCount>iTotLayers)iLowerCount=iTotLayers;
    if(iUpperCount<0)iUpperCount=0;

    if(iCurLayer<iTransitionPointLayer)iLowerCount = iLowerCount-iCurLayer; else iLowerCount = 0;
    if(iCurLayer>=iTransitionPointLayer) iUpperCount = iTotLayers - iCurLayer;

    int iTotalTimeMS = iExposeMS*iLowerCount + iExposeMS*iUpperCount;

    iTotalTimeMS = getLampAdjustedExposureTime(iTotalTimeMS);

    // Add Breathe and Settle
    iTotalTimeMS += iLowerCount*(pSettings->m_dBreatheClosed1 + pSettings->m_dSettleOpen1);
    iTotalTimeMS += iUpperCount*(pSettings->m_dBreatheClosed2 + pSettings->m_dSettleOpen2);

    // Z Travel Time
    int iGap1 = iLowerCount*(int)(pSettings->m_dOverLift1*100000.0/(double)pPrinterComm->getPU());
    int iGap2 = iUpperCount*(int)(pSettings->m_dOverLift2*100000.0/(double)pPrinterComm->getPU());

    int iZRaiseDistance1 = iGap1 + iLowerCount*(int)(dLayerThicknessMM*100000.0/(double)pPrinterComm->getPU());
    int iZLowerDistance1 = iGap1;

    int iZRaiseDistance2 = iGap2 + iUpperCount*(int)(dLayerThicknessMM*100000.0/(double)pPrinterComm->getPU());
    int iZLowerDistance2 = iGap2;

    iTotalTimeMS += getZMoveTime(iZRaiseDistance1,pSettings->m_iRSpd1);
    iTotalTimeMS += getZMoveTime(iZRaiseDistance2,pSettings->m_iRSpd2);
    iTotalTimeMS += getZMoveTime(iZLowerDistance1,pSettings->m_iLSpd1);
    iTotalTimeMS += getZMoveTime(iZLowerDistance2,pSettings->m_iLSpd2);

    // Vat movement Time
    iTotalTimeMS += iLowerCount*getVatMoveTime(pSettings->m_iOpenSpd1) + iLowerCount*getVatMoveTime(pSettings->m_iCloseSpd1);
    iTotalTimeMS += iUpperCount*getVatMoveTime(pSettings->m_iOpenSpd2) + iUpperCount*getVatMoveTime(pSettings->m_iCloseSpd2);
    return iTotalTimeMS;
}

int B9Terminal::getZMoveTime(int iDelta, int iSpd){
    // returns time to travel iDelta PUs distance in milliseconds
    // Accurate but assumes that 100% is 140rpm and 0% is 10rpm
    // Also assumes 200 PU (Steps) per revolution
    // returns milliseconds required to move iDelta PU's
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
//    return 999 - dPercent*229.0; // based on speed tests of B9C1 on 11/14/2012
    return 1500 - dPercent*229.0; // updated based on timeouts on pre-production model tests 11/18/2012
}

int B9Terminal::getEstBaseCycleTime(int iCur, int iTgt){
    int iDelta = abs(iTgt - iCur);
    int iLowerSpd,iOpnSpd,iSettle;
    int cutOffPU = (int)(pSettings->m_dBTClearInMM*100000.0/(double)pPrinterComm->getPU());
    if(iTgt<=cutOffPU){
        iLowerSpd = pSettings->m_iLSpd1;
        iOpnSpd = pSettings->m_iOpenSpd1;
        iSettle = pSettings->m_dSettleOpen1*1000.0;
    }
    else{
        iLowerSpd = pSettings->m_iLSpd2;
        iOpnSpd = pSettings->m_iOpenSpd2;
        iSettle = pSettings->m_dSettleOpen2*1000.0;
    }
    // Time to move iDelta
    int iTimeReq = getZMoveTime(iDelta, iLowerSpd);
    // Plus time to open vat
    iTimeReq += getVatMoveTime(iOpnSpd);
    // Plus settle time;
    iTimeReq += iSettle;
    return iTimeReq;
}

int B9Terminal::getEstNextCycleTime(int iCur, int iTgt){
    int iDelta = abs(iTgt - iCur);
    int iRaiseSpd,iLowerSpd,iOpnSpd,iClsSpd,iGap,iBreathe,iSettle;
    int cutOffPU = (int)(pSettings->m_dBTClearInMM*100000.0/(double)pPrinterComm->getPU());
    if(iTgt<=cutOffPU){
        iRaiseSpd = pSettings->m_iRSpd1;
        iLowerSpd = pSettings->m_iLSpd1;
        iOpnSpd = pSettings->m_iOpenSpd1;
        iClsSpd = pSettings->m_iCloseSpd1;
        iGap = (int)(pSettings->m_dOverLift1*100000.0/(double)pPrinterComm->getPU());
        iBreathe = pSettings->m_dBreatheClosed1*1000.0;
        iSettle = pSettings->m_dSettleOpen1*1000.0;
    }
    else{
        iRaiseSpd = pSettings->m_iRSpd2;
        iLowerSpd = pSettings->m_iLSpd2;
        iOpnSpd = pSettings->m_iOpenSpd2;
        iClsSpd = pSettings->m_iCloseSpd2;
        iGap = (int)(pSettings->m_dOverLift2*100000.0/(double)pPrinterComm->getPU());
        iBreathe = pSettings->m_dBreatheClosed2*1000.0;
        iSettle = pSettings->m_dSettleOpen2*1000.0;
    }
    // Time to move +iDelta + iGap, up and down
    int iTimeReq = getZMoveTime(iDelta+iGap, iRaiseSpd);
    iTimeReq += getZMoveTime(iDelta+iGap, iLowerSpd);
    // Plus time to close + open the vat
    iTimeReq += getVatMoveTime(iClsSpd)+getVatMoveTime(iOpnSpd);
    // Plus breathe & settle time;
    iTimeReq += iBreathe + iSettle;
    return iTimeReq;
}

int B9Terminal::getEstFinalCycleTime(int iCur, int iTgt){
    int iDelta = abs(iTgt - iCur);
    int iRaiseSpd,iClsSpd;
    int cutOffPU = (int)(pSettings->m_dBTClearInMM*100000.0/(double)pPrinterComm->getPU());
    if(iTgt<=cutOffPU){
        iRaiseSpd = pSettings->m_iRSpd1;
        iClsSpd = pSettings->m_iCloseSpd1;
    }
    else{
        iRaiseSpd = pSettings->m_iRSpd2;
        iClsSpd = pSettings->m_iCloseSpd2;
    }
    // Time to move +iDelta up
    int iTimeReq = getZMoveTime(iDelta, iRaiseSpd);
    // time to close the vat
    iTimeReq += getVatMoveTime(iClsSpd);
    return iTimeReq;
}

void B9Terminal::onScreenCountChanged(int iCount){
    QString sVideo = "Disconnected or Primary Monitor";
    if(pProjector) {
        delete pProjector;
        pProjector = NULL;
        if(pPrinterComm->getProjectorStatus()==B9PrinterStatus::PS_ON)
            if(!isEnabled())emit signalAbortPrint("Print Aborted:  Connection to Projector Lost or Changed During Print Cycle");
    }
    pProjector = new B9Projector(true, 0,Qt::WindowStaysOnTopHint);
    makeProjectorConnections();
    int i=iCount;
    int screenCount = m_pDesktop->screenCount();
    QRect screenGeometry;

    if(m_bUsePrimaryMonitor)
    {
        screenGeometry = m_pDesktop->screenGeometry(0);
    }
    else{
        for(i=screenCount-1;i>= 0;i--) {
            screenGeometry = m_pDesktop->screenGeometry(i);
            if(screenGeometry.width() == pPrinterComm->getNativeX() && screenGeometry.height() == pPrinterComm->getNativeY()) {
                //Found the projector!
                sVideo = "Connected to Monitor: " + QString::number(i+1);
                m_bNeedsWarned = true;
                break;
            }
        }
    }
    if(i<=0||m_bUsePrimaryMonitor)m_bPrimaryScreen = true; else m_bPrimaryScreen = false;

    emit updateProjectorOutput(sVideo);

    pProjector->setShowGrid(true);
    pProjector->setCPJ(NULL);

    emit sendStatusMsg("B9Creator - Idle");
    pProjector->setGeometry(screenGeometry);
    if(!m_bPrimaryScreen){
        pProjector->showFullScreen(); // Only show it if it is a secondary monitor
        pProjector->hide();
        activateWindow(); // if not using primary monitor, take focus back to here.
    }
    else if(m_bPrintPreview||(pPrinterComm->getProjectorStatus() != B9PrinterStatus::PS_OFF &&
            pPrinterComm->getProjectorStatus() != B9PrinterStatus::PS_COOLING &&
            pPrinterComm->getProjectorStatus() != B9PrinterStatus::PS_UNKNOWN)) {
        // if the projector is not turned off, we better put up the blank screen now!
        pProjector->showFullScreen();
    }
    else warnSingleMonitor();
}

void B9Terminal::createNormalizedMask(double XYPS, double dZ, double dOhMM)
{
    //call when we show or resize
    pProjector->createNormalizedMask(XYPS, dZ, dOhMM);
}


void B9Terminal::on_comboBoxXPPixelSize_currentIndexChanged(int index)
{
    QString sCmd;
    bool bUnChanged = false;
    switch (index){
        case 0: // 50 microns
            sCmd = "U50";
            if(getXYPixelSize()==50)bUnChanged=true;
            break;
        case 1: // 75 microns
            sCmd = "U75";
            if(getXYPixelSize()==75)bUnChanged=true;
            break;
        case 2: // 100 mircons
            sCmd = "U100";
            if(getXYPixelSize()==100)bUnChanged=true;
        default:
            break;
    }
    if(!bUnChanged){
        pPrinterComm->SendCmd(sCmd);
        pPrinterComm->SendCmd("A"); // Force refresh of printer stats
    }
}

void B9Terminal::on_pushButtonCycleSettings_clicked()
{
    pSettings->updateValues();
}

void B9Terminal::getKey(int iKey)
{
    if(!m_bPrimaryScreen)return; // Ignore keystrokes from the print window unless we're using the primary monitor
    if(isVisible()&&isEnabled())
    {
        // We must be "calibrating"  If we get any keypress we should close the projector window
        if(pProjector!=NULL) pProjector->hide();
    }
    switch(iKey){
    case 112:		// 'p' Pause/Resume
        emit pausePrint();
        break;
    case 65:        // Capital 'A' to abort
        if(!isEnabled()){
            m_pPReleaseCycleTimer->stop();
            emit signalAbortPrint("User Directed Abort.");
        }
        break;
    default:
        break;
    }
}

