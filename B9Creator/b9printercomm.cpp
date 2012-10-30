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
#include <QtGui/QApplication>
#include <QDir>
#include <QProcess>
#include <QTimer>
#include <QMessageBox>
#include "b9printercomm.h"
#include "qextserialport.h"
#include "qextserialenumerator.h"

void B9PrinterStatus::reset(){
    // Reset all status variables to unknown
    iV1 = iV2 = iV3 = -1;
    iResetState = -1;
    iProjPwr = -1;
    lastMsgTime.start();
    bResetInProgress = false;

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
    return getVersion() == CURRENTFIRMWARE;
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
    pPorts = new QList<QextPortInfo>;
    pEnumerator = new QextSerialEnumerator();
    m_serialDevice = NULL;
    m_bCloneBlanks = false;
    m_Status.reset();
    qDebug() << "B9Creator COMM Start";
    QTimer::singleShot(2000, this, SLOT(RefreshCommPortItems())); // Check in 2 seconds
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
    if(sCmd == "r" || sCmd == "R")m_Status.setFindingHomeStatus(true);
    qDebug() << "SendCmd->" << sCmd;
}

void B9PrinterComm::watchDog()
{
    // We expect a valid handle to m_SerialDevice and
    // we expect the last readReady signal from the Printer to have happened within the last 10 seconds
    // Unless the printer is busy finding the home location (which
    int iTimeLimit = 10000;
    if(m_Status.isFindingHome()) iTimeLimit = 60000;

    if( m_serialDevice==NULL || m_Status.getLastMsgElapsedTime() > iTimeLimit){
        // Lost contact with B9Creator!
        if(m_serialDevice!=NULL){
            if (m_serialDevice->isOpen())
            m_serialDevice->close();
            delete m_serialDevice;
            m_serialDevice = NULL;
        }
        m_Status.reset();
        //TODO Broadcast Lost Contact Signal, return without activating watchDog timer
        emit updateConnectionStatus("Searching for B9Creator...");
        qDebug() << "WATCHDOG:  LOST CONTACT WITH B9CREATOR!";
    }
    else // Still in Contact with B9Creator
         startWatchDogTimer();
}

void B9PrinterComm::startWatchDogTimer()
{
    // We call the watchdog every 20 seconds
    QTimer::singleShot(20000, this, SLOT(watchDog())); // Check in 10 seconds
}

void B9PrinterComm::RefreshCommPortItems()
{
    QString sCommPortStatus = "Searching for B9Creator...";
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
//TODO emit a signal to announce that we lost contact with the printer!
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
                sCommPortStatus = "Connected to B9Creator On Port: "+sPortName;
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
            emit updateConnectionStatus("Updating Firmware...");
            B9FirmwareUpdate Firmware;
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            Firmware.UploadHex(sPortName);
            QApplication::restoreOverrideCursor();
            emit updateConnectionStatus("Searching for B9Creator...");
        }
        emit updateConnectionStatus(sCommPortStatus);
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
        m_serialDevice->setFlowControl(FLOW_HARDWARE);
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

    // Delay for up to 3 seconds while we wait for response from printer
    QTime delayTime = QTime::currentTime().addSecs(3);
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

void B9PrinterComm::ReadAvailable() {
    if(m_serialDevice==NULL) qFatal("Error:  slotRead, NULL Port Handle");

    m_Status.resetLastMsgTime();
    if(m_Status.isFindingHome()) {
        m_Status.setFindingHomeStatus(false); // if we are receiving data, we must not be finding home!
        // TODO emit reset complete signal
    }

    QByteArray ba = m_serialDevice->readAll();  // read block of available raw data

    // We process the raw data one line at a time, keeping in mind they may be spread across multiple blocks.
    int iCurPos = 0;
    char c;
    while(iCurPos<ba.size()){
        c = ba.at(iCurPos);
        iCurPos++;
        if(c!='\r') m_sSerialString+=QString(c);
        if(c=='\n'||c=='\r'){
            // Line Read Complete, process data
            emit broadcastPrinterComm(m_sSerialString);

            if(m_sSerialString.left(1) == "C"){
                // Comment
                // qDebug() << m_sSerialString.right(m_sSerialString.length()-1) << "\n";
            }
            if(m_sSerialString.left(1) == "V"){
                // Version
                m_Status.setVersion(m_sSerialString.right(m_sSerialString.length()-1));
                //qDebug() << "Version: " << m_Status.getVersion() << "\n";
            }

            if(m_sSerialString.left(1) == "F"){
                /*
                if(m_iPrintState == PRINT_MOV2READY){
                    m_iPrintState = PRINT_WAITFORP;
                }
                else if(m_iPrintState == PRINT_MOV2NEXT){
//					exposeLayer();
                    breatheLayer();
                }
                else if(m_iPrintState == PRINT_NO) {
                    sendCmd("p0");
                }
                */
            }

            if(m_sSerialString.left(2) == "R0"){
                //qDebug() << "R0:  Reset not Required.";
            }
            if(m_sSerialString.left(2) == "R1"){
                //qDebug() << "R1:  Reset Required.";
            }

            if(m_sSerialString.left(2) == "PO"||m_sSerialString.left(2) == "P0"){
                 //qDebug() << "P0:  Projector Power OFF.";
            }
            if(m_sSerialString.left(2) == "P1"){
                 //qDebug() << "P0:  Projector Power ON.";
            }

            if(m_sSerialString.left(1) =="S"){
                 //qDebug() << "Vat Position:" << m_sSerialString.right(m_sSerialString.length()-1);
            }

            if(m_sSerialString.left(1) =="I"){
                //iPrinterUnits = m_sSerialString.right(m_sSerialString.length()-1).toInt();
            }

            if(m_sSerialString.left(1) =="Z"){
                //ui.lineEditZPos->setText(m_sSerialString.right(m_sSerialString.length()-1));
                //double dPos = m_sSerialString.right(m_sSerialString.length()-1).toInt();
                //dPos *= iPrinterUnits;
                //dPos /= 100000;
                //ui.lineEditZPos->setText(QString::number(dPos,'g',8)+" mm");
            }

            m_sSerialString=""; // Line processed, clear it for next line
        }
    }
}
