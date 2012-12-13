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

#include <QtGui/QApplication>
#include <QDir>
#include <QProcess>
#include <QTimer>
#include "b9printercomm.h"
#include "qextserialport.h"
#include "qextserialenumerator.h"

void B9PrinterStatus::reset(){
    // Reset all status variables to unknown
    iV1 = iV2 = iV3 = -1;
    m_eHomeStatus = HS_UNKNOWN;
    m_eProjStatus = PS_UNKNOWN;
    m_bProjCmdOn  = false;
    m_bCanControlProjector = false;
    m_bHasShutter = false;
    m_iPU = 635;
    m_iUpperZLimPU = -1;
    m_iCurZPosInPU = -1;
    m_iCurVatPercentOpen = -1;
    m_iLastHomeDiff = 0;

    m_iNativeX = 0;
    m_iNativeY = 0;
    m_iXYPizelSize = 0;
    m_iHalfLife = 2000;

    lastMsgTime.start();
    bResetInProgress = false;
    setLampHrs(-1); // unknown
}

void B9PrinterStatus::setVersion(QString s)
{
    // s must be formated as "n1 n2 n3" where nx are integers.
    int i=0;
    QChar c;
    int p=0;
    QString v;
    while (i<s.length()){
        c = s.at(i);
        v+=c;
        if(s.at(i)==' ' || i==s.length()-1){
            p++;
            if(p==1){
                iV1 = v.toInt();
            }
            else if(p==2){
                iV2 = v.toInt();
            }
            if(p==3){
                iV3 = v.toInt();
            }
            v="";
        }
        i++;
    }
}

bool B9PrinterStatus::isCurrentVersion(){
    return getVersion() >= CURRENTFIRMWARE;
}

bool B9PrinterStatus::isValidVersion(){
    if(iV1<0||iV2<0||iV3<0)
        return false;
    return true;
}

QString B9PrinterStatus::getVersion(){
    if(iV1<0||iV2<0||iV3<0)
        return "?";
    return "v" + QString::number(iV1)+ "."+ QString::number(iV2)+ "."+ QString::number(iV3);
}

bool B9FirmwareUpdate::UploadHex(QString sCurPort)
{
    QDir appdir = QDir(QCoreApplication::applicationDirPath());
    QString launchcommand = "avrdude ";
    QString args = "-Cavrdude.conf -v -v -v -v -patmega328p -carduino -P" + sCurPort + " -b115200 -D -Uflash:w:\"" + sHexFilePath + "\":i";

    #ifdef Q_WS_MAC//in mac packages if avrdude is in recources
        appdir.cdUp();
        appdir.cd("Resources");
    #endif
    #ifndef Q_WS_WIN
        launchcommand = "./avrdude ";
    #endif

    qDebug() << "Calling AVRDude...";

    QCoreApplication::processEvents(); //Process all pending events prior to proceeding.
    QProcess* myProcess = new QProcess(this);
    qDebug() << "Looking for avrdude, avrdude.conf, and .hex file in working dir:" << appdir.path();
    myProcess->setWorkingDirectory(appdir.path());
    myProcess->setProcessChannelMode(QProcess::MergedChannels);
    myProcess->start(launchcommand + args);
    if (!myProcess->waitForFinished(120000)) //Allow 120 seconds for avrdude to program firmware before timing out
    {
        qDebug() << "AVRDude Firmware Update FAILED.";
        return false;
    }
    else
    {
        qDebug() << "Begin Firmware Update on Port: " + sCurPort;
        qDebug() << myProcess->readAll();
    }
    if(myProcess->exitCode() != 0)
    {
        qDebug() << "Firmware Update FAILED, exit code:" << QString::number(myProcess->exitCode());
        return false;
    }
    qDebug() << "Firmware Update Complete";
    return true;
}

B9PrinterComm::B9PrinterComm()
{
    m_bIsPrinting = false;
    pPorts = new QList<QextPortInfo>;
    pEnumerator = new QextSerialEnumerator();
    m_serialDevice = NULL;
    m_bCloneBlanks = false;
    m_Status.reset();
    m_iWarmUpDelayMS = 15000;
    qDebug() << "B9Creator COMM Start";
    QTimer::singleShot(500, this, SLOT(RefreshCommPortItems())); // Check in .5 seconds
}

B9PrinterComm::~B9PrinterComm()
{
    if(pPorts)delete pPorts;
    if(pEnumerator) pEnumerator->deleteLater();
    if(m_serialDevice) delete m_serialDevice;
    qDebug() << "B9Creator COMM End";
}

void B9PrinterComm::SendCmd(QString sCmd)
{
    if(m_serialDevice)m_serialDevice->write(sCmd.toAscii()+'\n');
    if(sCmd == "r" || sCmd == "R") m_Status.setHomeStatus(B9PrinterStatus::HS_SEEKING);
    qDebug() << "SendCmd->" << sCmd;
}

void B9PrinterComm::watchDog()
{
    // We expect a valid handle to m_SerialDevice and
    // we expect the last readReady signal from the Printer to have happened within the last 10 seconds
    // Unless the printer is busy finding the home location
    int iTimeLimit = 10000;
    if(m_Status.getHomeStatus() == B9PrinterStatus::HS_SEEKING) iTimeLimit = 90000;

    if( m_serialDevice != NULL && m_Status.getLastMsgElapsedTime() <= iTimeLimit){
        // Still in Contact with the B9Creator
        startWatchDogTimer();
        emit updateConnectionStatus(MSG_CONNECTED);
        emit BC_ConnectionStatusDetailed("Connected to port: "+m_serialDevice->portName());
        return;
    }

    // Lost Comm...
    if(m_Status.getHomeStatus()==B9PrinterStatus::HS_SEEKING) m_Status.setHomeStatus(B9PrinterStatus::HS_UNKNOWN);
    if(m_serialDevice!=NULL){
        // port device still exists but getting no messages, time to reboot the port
        if (m_serialDevice->isOpen())
        m_serialDevice->close();
        delete m_serialDevice;
        m_serialDevice = NULL;
    }
    //Attempt to pick up were we lost contact if we find the port again within a short time
    qDebug() << "WATCHDOG:  LOST CONTACT WITH B9CREATOR!";

    emit updateConnectionStatus(MSG_SEARCHING);
    emit BC_ConnectionStatusDetailed("Lost Contact with B9Creator.  Searching...");
    m_Status.reset();
    handleLostComm();
    RefreshCommPortItems();
}

void B9PrinterComm::handleLostComm(){
    emit BC_LostCOMM();
}

void B9PrinterComm::startWatchDogTimer()
{
    // We call the watchdog every 10 seconds
    QTimer::singleShot(10000, this, SLOT(watchDog())); // Check in 10 seconds
}

void B9PrinterComm::RefreshCommPortItems()
{
    if(m_bIsPrinting)return; // We assume we stay connected during the print proccess.  If we are disconnected, the watchdog timer will fire
    QString sCommPortStatus = MSG_SEARCHING;
    QString sCommPortDetailedStatus = MSG_SEARCHING;
    QString sPortName;
    // Load the current enumerated available ports
    *pPorts = pEnumerator->getPorts();

    if(m_serialDevice){
        // We've previously located the printer, are we still connected?
        for (int i = 0; i < pPorts->size(); i++) {
        // Check each existing port to see if our's still exists
        #ifdef Q_WS_X11
            if(pPorts->at(i).physName == m_serialDevice->portName()){
        #else
            if(pPorts->at(i).portName == m_serialDevice->portName()){
        #endif
                // We're still connected, set a timer to check again in 5 seconds and then exit
                QTimer::singleShot(5000, this, SLOT(RefreshCommPortItems()));
                return;
            }
        }
        // We lost the previous connection and should delete this port connection
        if (m_serialDevice->isOpen())
            m_serialDevice->close();
        delete m_serialDevice;
        m_serialDevice = NULL;
        m_Status.reset();
        qDebug() << sCommPortStatus;
        emit updateConnectionStatus(sCommPortStatus);
        sCommPortDetailedStatus = "Lost Comm on previous port. Searching...";
        emit BC_ConnectionStatusDetailed("Lost Comm on previous port. Searching...");
        handleLostComm();
    }

    // Now we search for a B9Creator
    int eTime = 1000;  // 1 second search cylce time for while not connected

    sNoFirmwareAurdinoPort = "";  // Reset to null string before scanning ports
    if(pPorts->size()>0){
        // Some ports are available, are they the B9Creator?
        qDebug() << "Scanning For Serial Port Devices (" << pPorts->size() << "found )";
        for (int i = 0; i < pPorts->size(); i++) {
            qDebug() << "  port name   " << pPorts->at(i).portName;
            qDebug() << "  locationInfo" << pPorts->at(i).physName;
         #ifndef Q_WS_X11
            //Note: We only trust friendName, vendorID and productID with Windows and OS_X
            qDebug() << "  description " << pPorts->at(i).friendName;
            qDebug() << "  vendorID    " << pPorts->at(i).vendorID;
            qDebug() << "  productID   " << pPorts->at(i).productID;
         #endif
         #ifdef Q_WS_X11
            // linux ID's ports by physName
            sPortName = pPorts->at(i).physName;
            // We filter ports by requiring the portName to begin with "ttyA"
            if(pPorts->at(i).portName.left(4) == "ttyA" && OpenB9CreatorCommPort(sPortName)){
         #else
            // Windows and OSX use portName to ID ports
            sPortName = pPorts->at(i).portName;
            // We filter ports by requiring vendorID value of 9025 (Arduino)
            if(pPorts->at(i).vendorID==9025 && OpenB9CreatorCommPort(sPortName)){
         #endif
                // Connected!
                sCommPortStatus = MSG_CONNECTED;
                sCommPortDetailedStatus = "Connected on Port: "+m_serialDevice->portName();
                eTime = 5000;  // Re-check connection again in 5 seconds
                if(m_serialDevice && m_Status.isCurrentVersion())startWatchDogTimer();  // Start B9Creator "crash" watchDog
                break;
            }
        }
        bool bUpdateFirmware = false;
        if( m_serialDevice==NULL && sNoFirmwareAurdinoPort!=""){
            // We did not find a B9Creator with valid firmware, but we did find an Arduino
            // We assume this is a new B9Creator and needs firmware!
            // However, we will not upload firmware unless m_bCloneBlanks is true!
            if(m_bCloneBlanks){
                qDebug() << "\"Clone Firmware\" Option is enabled.  Attempting Firmware Upload to possible B9Creator found on port: "<< sNoFirmwareAurdinoPort;
                bUpdateFirmware = true;
                sPortName = sNoFirmwareAurdinoPort;
            }
            else {
                qDebug() << "\"Clone Firmware\" Option is disabled.  No Firmware upload attempted on possible B9Creator found on port: " << sNoFirmwareAurdinoPort;
                bUpdateFirmware = false;
            }
        }
        else if (m_serialDevice!=NULL && !m_Status.isCurrentVersion()){
            // We found a B9Creator with the wrong firmware version, update it!
            qDebug() << "Incorrect Firmware version found on connected B9Creator"<< sPortName << "  Attempting B9Creator Firmware Update to" << CURRENTFIRMWARE;
            bUpdateFirmware = true;
            if(m_serialDevice!=NULL) {
                m_serialDevice->flush();
                m_serialDevice->close();
                delete m_serialDevice;
            }
            m_serialDevice = NULL;
            m_Status.reset();
        }
        if(bUpdateFirmware){
            // Update the firmware on device on sPortName
            emit updateConnectionStatus(MSG_FIRMUPDATE);
            emit BC_ConnectionStatusDetailed("Updating Firmware on port: "+sPortName);
            B9FirmwareUpdate Firmware;
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            Firmware.UploadHex(sPortName);
            QApplication::restoreOverrideCursor();
            emit updateConnectionStatus(MSG_SEARCHING);
            emit BC_ConnectionStatusDetailed("Firmware Update Complete.  Searching...");
        }
        emit updateConnectionStatus(sCommPortStatus);
        emit BC_ConnectionStatusDetailed(sCommPortDetailedStatus);
    }
    QTimer::singleShot(eTime, this, SLOT(RefreshCommPortItems())); // Check again in 5 seconds if connected, 1 secs if not
}

bool B9PrinterComm::OpenB9CreatorCommPort(QString sPortName)
{
    if(m_serialDevice!=NULL) qFatal("Error:  We found an open port handle that should have been deleted!");

    // Attempt to establish a serial connection with the B9Creator
    m_serialDevice = new QextSerialPort(sPortName, QextSerialPort::EventDriven, this);
    if (m_serialDevice->open(QIODevice::ReadWrite) == true) {
        m_serialDevice->setBaudRate(BAUD115200);
        m_serialDevice->setDataBits(DATA_8);
        m_serialDevice->setParity(PAR_NONE);
        m_serialDevice->setStopBits(STOP_1);
        m_serialDevice->setFlowControl(FLOW_OFF);
        m_serialDevice->setDtr(true);   // Reset the Aurduino
        m_serialDevice->setDtr(false);

        connect(m_serialDevice, SIGNAL(readyRead()), this, SLOT(ReadAvailable()));
        qDebug() << "Opened Comm port:" << sPortName;
    }
    else {
        // device failed to open
        if(m_serialDevice!=NULL) delete m_serialDevice;
        m_serialDevice = NULL;
        m_Status.reset();
        qDebug() << "Failed to open Comm port:" << sPortName;
        return false;
    }

    // Delay for up to 5 seconds while we wait for response from printer
    QTime delayTime = QTime::currentTime().addSecs(5);
    while( QTime::currentTime() < delayTime && !m_Status.isValidVersion() )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    if(!m_Status.isValidVersion()){
        if(m_serialDevice!=NULL) {
            m_serialDevice->flush();
            m_serialDevice->close();
            delete m_serialDevice;
        }
        m_serialDevice = NULL;
        m_Status.reset();
        qDebug() << "Found a possible Arduino Device, perhaps a B9Creator without firmware loaded, on Port: " << sPortName;
        sNoFirmwareAurdinoPort = sPortName;
        return false;
    }
    return true;
}

QString B9PrinterComm::errorString(){
    return m_serialDevice->errorString();
}

void B9PrinterComm::ReadAvailable() {
    if(m_serialDevice==NULL) qFatal("Error:  slot 'ReadAvailable()' but NULL Port Handle");

    m_Status.resetLastMsgTime();  //update for watchdog

    if(m_Status.getHomeStatus() == B9PrinterStatus::HS_SEEKING) {
        m_Status.setHomeStatus(B9PrinterStatus::HS_UNKNOWN); // if we are receiving data, we must no longer be seeking Home
        // we'll set the status to HS_FOUND once we recieve a 'X' diff broadcast
    }

    QByteArray ba = m_serialDevice->readAll();  // read block of available raw data

    // We process the raw data one line at a time, keeping in mind they may be spread across multiple blocks.
    int iCurPos = 0;
    int iLampHrs = -1;
    int iInput = -1;
    char c;
    while(iCurPos<ba.size()){
        c = ba.at(iCurPos);
        iCurPos++;
        if(c!='\r') m_sSerialString+=QString(c);
        if(c=='\n'){
            // Line Read Complete, process data

           if(m_sSerialString.left(1) != "P" && m_sSerialString.left(1) != "L" && m_sSerialString.length()>0){
                // We only emit this data for display & log purposes
                if(m_sSerialString.left(1) == "C"){
                    qDebug() << m_sSerialString.right(m_sSerialString.length()-1) << "\n";
                }
                else{
                    emit BC_RawData(m_sSerialString);
                    qDebug() << m_sSerialString << "\n";
                }
            }

            int iCmdID = m_sSerialString.left(1).toUpper().toAscii().at(0);
            switch (iCmdID){
            case 'U':  // Mechanical failure of encoder?
                qDebug() << "WARNING:  Printer has sent 'U' report, runaway X Motor indication: " + m_sSerialString << "\n";
                break;
            case 'Q':  // Printer got tired of waiting for command and shut down projectors
                       // We will likely never see this as something bad has happened
                       // (like we have crashed or been shut off during a print process
                       // So if we get it, we'll just post it to the log and wait for
                       // timeouts to correct things.
                qDebug() << "WARNING:  Printer has sent 'Q' report, lost comm with host." << "\n";
                break;
            case 'P':  // take care of projector status
                iInput = m_sSerialString.right(m_sSerialString.length()-1).toInt();
                if(iInput!=1)iInput = 0;
                if(handleProjectorBC(iInput)){
                    //projector status changed
                    emit BC_RawData(m_sSerialString);
                    qDebug() << m_sSerialString << "\n";
                }
                break;
            case 'L':  // take care of projector Lamp hours update
                iLampHrs = m_sSerialString.right(m_sSerialString.length()-1).toInt();
                if(m_Status.getLampHrs()!= iLampHrs){
                    m_Status.setLampHrs(iLampHrs);
                    emit BC_ProjectorStatusChanged();
                    emit BC_RawData(m_sSerialString);
                    qDebug() << m_sSerialString << "\n";
                }
                break;

            case 'X':  // Found Home with this Difference Offset
                m_Status.setHomeStatus(B9PrinterStatus::HS_FOUND);
                m_Status.setLastHomeDiff(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_HomeFound();
                break;

            case 'R':  // Needs Reset?
                iInput = m_sSerialString.right(m_sSerialString.length()-1).toInt();
                if(iInput==0) m_Status.setHomeStatus(B9PrinterStatus::HS_FOUND);
                else m_Status.setHomeStatus(B9PrinterStatus::HS_UNKNOWN);
                break;

            case 'K':  // Current Lamp 1/2 life
                m_Status.setHalfLife(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_HalfLife(m_Status.getHalfLife());
                break;

            case 'D':  // Current Native X Projector resolution
                m_Status.setNativeX(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_NativeX(m_Status.getNativeX());
                break;

            case 'E':  // Current Native Y Projector resolution
                m_Status.setNativeY(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_NativeY(m_Status.getNativeY());
                break;

            case 'H':  // Current XY Pixel Size
                m_Status.setXYPixelSize(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_XYPixelSize(m_Status.getXYPixelSize());
                break;

            case 'I':  // Current PU
                m_Status.setPU(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_PU(m_Status.getPU());
                break;

            case 'A':  // Projector Control capability
                m_Status.setProjectorRemoteCapable(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_ProjectorRemoteCapable(m_Status.isProjectorRemoteCapable());
                break;

            case 'J':  // Projector Shutter capability
                m_Status.setHasShutter(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_HasShutter(m_Status.hasShutter());
                break;

            case 'M':  // Current Z Upper Limit in PUs
                m_Status.setUpperZLimPU(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_UpperZLimPU(m_Status.getUpperZLimPU());
                break;

            case 'Z':  // Current Z Position Update
                m_Status.setCurZPosInPU(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_CurrentZPosInPU(m_Status.getCurZPosInPU());
                break;

            case 'S':  // Current Vat(Shutter) Percent Open Position Update
                m_Status.setCurVatPercentOpen(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_CurrentVatPercentOpen(m_Status.getCurVatPercentOpen());
                break;

            case 'C':  // Comment
                emit BC_Comment(m_sSerialString.right(m_sSerialString.length()-1));
                break;

            case 'F':  // Print release cycle finished
                emit BC_PrintReleaseCycleFinished();
                break;

            case 'V':  // Version
                m_Status.setVersion(m_sSerialString.right(m_sSerialString.length()-1).trimmed());
                emit BC_FirmVersion(m_Status.getVersion());
                break;

            case 'W':  // Model
                m_Status.setModel(m_sSerialString.right(m_sSerialString.length()-1).trimmed());
                emit BC_ModelInfo(m_Status.getModel());
                break;

            case 'Y':  // Current Z Home position in PU's
                // ignored
                break;

            default:
                qDebug() <<"WARNING:  IGNORED UNKNOWN CMD:  " << m_sSerialString << "\n";
                break;
            }
            m_sSerialString=""; // Line processed, clear it for next line
        }
    }
}

void B9PrinterComm::setProjectorPowerCmd(bool bPwrFlag){
    if(bPwrFlag){
        SendCmd("P1"); // Turn On Command
        m_Status.setProjectorStatus(B9PrinterStatus::PS_TURNINGON);
    }
    else {
        SendCmd("P0"); // Turn On Command
        m_Status.setProjectorStatus(B9PrinterStatus::PS_COOLING);
    }
    m_Status.resetLastProjCmdTime();
    emit BC_ProjectorStatusChanged();
}

bool B9PrinterComm::handleProjectorBC(int iBC){
    bool bStatusChanged = false;
    if(m_Status.isProjectorPowerCmdOn() && iBC == 0){
        // Projector commanded ON but current report is OFF
        switch(m_Status.getProjectorStatus()){
        case B9PrinterStatus::PS_OFF:
        case B9PrinterStatus::PS_UNKNOWN:
        case B9PrinterStatus::PS_TIMEOUT:
        case B9PrinterStatus::PS_FAIL:
            setProjectorPowerCmd(true); // turn it on now
            bStatusChanged = true;
            break;
        case B9PrinterStatus::PS_TURNINGON:
            if(m_Status.getLastProjCmdElapsedTime()>45000){
                // Taking too long to turn on, something is wrong!
                m_Status.setProjectorStatus(B9PrinterStatus::PS_TIMEOUT);
                emit BC_ProjectorFAIL();
                m_Status.cmdProjectorPowerOn(false);
                bStatusChanged = true;
            }
            break;
        case B9PrinterStatus::PS_WARMING:
        case B9PrinterStatus::PS_ON:
            // Uncommanded Power off!  Lost power or bulb failure?
            m_Status.setProjectorStatus(B9PrinterStatus::PS_FAIL);
            emit BC_ProjectorFAIL();
            m_Status.cmdProjectorPowerOn(false);
            bStatusChanged = true;
            break;
        case B9PrinterStatus::PS_COOLING:
            // Done cooling off
            m_Status.setProjectorStatus(B9PrinterStatus::PS_OFF);
            emit BC_ProjectorStatusChanged();
            bStatusChanged = true;
            break;
        default:
            // Nothing we can do
            break;
        }
    }
    else if(m_Status.isProjectorPowerCmdOn() && iBC == 1){
        // Projector commanded ON and current report is ON
        switch(m_Status.getProjectorStatus()){
        case B9PrinterStatus::PS_COOLING:
        case B9PrinterStatus::PS_OFF:
        case B9PrinterStatus::PS_UNKNOWN:
        case B9PrinterStatus::PS_TIMEOUT:
        case B9PrinterStatus::PS_FAIL:
            // We were turning off, off, failed, timed out or unknown and suddenly we're cmd on and actually on?
            // Best we can do is set the status on and report
            m_Status.setProjectorStatus(B9PrinterStatus::PS_ON);
            emit BC_ProjectorStatusChanged();
            bStatusChanged = true;
            break;
        case B9PrinterStatus::PS_TURNINGON:
            // We were turning on and now we need to warm up a bit
            m_Status.setProjectorStatus(B9PrinterStatus::PS_WARMING);
            startWarmingTime.start();
            emit BC_ProjectorStatusChanged();
            bStatusChanged = true;
            break;
        case B9PrinterStatus::PS_WARMING:
            if(startWarmingTime.elapsed()>m_iWarmUpDelayMS){
                // All warmed up now, ready to use.
                m_Status.setProjectorStatus(B9PrinterStatus::PS_ON);
                emit BC_ProjectorStatusChanged();
                bStatusChanged = true;
            }
            break;
        case B9PrinterStatus::PS_ON:
        default:
            // No change, we're good.
            break;
        }
    }
    else if(!m_Status.isProjectorPowerCmdOn() && iBC == 0){
        // Projector commanded OFF and current report is OFF
        switch(m_Status.getProjectorStatus()){
        case B9PrinterStatus::PS_UNKNOWN:
        case B9PrinterStatus::PS_TURNINGON:
        case B9PrinterStatus::PS_WARMING:
        case B9PrinterStatus::PS_ON:
        case B9PrinterStatus::PS_COOLING:
        case B9PrinterStatus::PS_TIMEOUT:
        case B9PrinterStatus::PS_FAIL:
            // We were turning on, warming up, on, cooling off, unknown, timed out or failed and now we're off
            m_Status.setProjectorStatus(B9PrinterStatus::PS_OFF);
            emit BC_ProjectorStatusChanged();
            bStatusChanged = true;
            break;
        case B9PrinterStatus::PS_OFF:
        default:
            // No change, we're good
            break;
        }
    }
    else if(!m_Status.isProjectorPowerCmdOn() && iBC == 1){
        // Projector commanded OFF but current report is ON
        switch(m_Status.getProjectorStatus()){
        case B9PrinterStatus::PS_COOLING:
            if(m_Status.getLastProjCmdElapsedTime()>30000){
                // Taking too long to turn off, send the off command again
                m_Status.setProjectorStatus(B9PrinterStatus::PS_ON);
                emit BC_ProjectorStatusChanged();
                bStatusChanged = true;
            }
            break;
        case B9PrinterStatus::PS_OFF:
            //User must have switched it on manually, we'll accept that and set the status to commanded on and Turning on
            cmdProjectorPowerOn(true);
            setProjectorPowerCmd(true);
            bStatusChanged = true;
            break;

        case B9PrinterStatus::PS_TURNINGON:
        case B9PrinterStatus::PS_WARMING:
        case B9PrinterStatus::PS_ON:
        case B9PrinterStatus::PS_UNKNOWN:
        case B9PrinterStatus::PS_TIMEOUT:
        case B9PrinterStatus::PS_FAIL:
        default:
            setProjectorPowerCmd(false); // turn it off now
            bStatusChanged = true;
            break;
        }
    }
    return bStatusChanged;
}
