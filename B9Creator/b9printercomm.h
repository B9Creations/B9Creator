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

#ifndef B9PRINTERCOMM_H
#define B9PRINTERCOMM_H
#include <QObject>
#include <QTime>
#include <QtDebug>

// The Firmware version is tied to a specific version
// These defines determine how we attempt to update Firmware
// The correctly named current hex file MUST be in the application directory
#define CURRENTFIRMWARE "v1.0.8"
#define FIRMWAREHEXFILE "B9Firmware_1_0_8.hex"
#define MSG_SEARCHING "Searching..."
#define MSG_CONNECTED "Connected"
#define MSG_FIRMUPDATE "Updating Firmware..."


class  QextSerialPort;
class  QextSerialEnumerator;
struct QextPortInfo;

/////////////////////////////////////////////////////////////////////////////
class B9PrinterStatus
{
public:
    enum ProjectorStatus{PS_OFF, PS_TURNINGON, PS_WARMING, PS_ON, PS_COOLING, PS_UNKNOWN, PS_TIMEOUT, PS_FAIL};
    enum HomeStatus{HS_SEEKING, HS_FOUND, HS_UNKNOWN};


    B9PrinterStatus(){reset();}
    void reset();

    QString getVersion();
    void setVersion(QString s);
    bool isCurrentVersion();
    bool isValidVersion();

    void cmdProjectorPowerOn(bool bOn){m_bProjCmdOn = bOn;}
    bool isProjectorPowerCmdOn(){return m_bProjCmdOn;}

    QString getModel(){return m_sModel;}
    void setModel(QString sModel){m_sModel = sModel;}

    bool isProjectorRemoteCapable(){ return m_bCanControlProjector;}
    void setProjectorRemoteCapable(bool bIsCapable){m_bCanControlProjector=bIsCapable;}

    bool hasShutter(){ return m_bHasShutter;}
    void setHasShutter(bool bHS){m_bHasShutter=bHS;}

    HomeStatus getHomeStatus() {return m_eHomeStatus;}
    void setHomeStatus(HomeStatus eHS) {m_eHomeStatus = eHS;}

    ProjectorStatus getProjectorStatus() {return m_eProjStatus;}
    void setProjectorStatus(ProjectorStatus ePS) {m_eProjStatus = ePS;}

    int getLampHrs(){return m_iLampHours;}
    void setLampHrs(int iLH){m_iLampHours = iLH;}

    void setPU(int iPU){m_iPU = iPU;}
    int getPU(){return m_iPU;}

    void setHalfLife(int iHL){m_iHalfLife = iHL;}
    int getHalfLife(){return m_iHalfLife;}

    void setNativeX(int iX){m_iNativeX = iX;}
    int getNativeX(){return m_iNativeX;}

    void setNativeY(int iY){m_iNativeY = iY;}
    int getNativeY(){return m_iNativeY;}

    void setXYPixelSize(int iPS){m_iXYPizelSize = iPS;}
    int getXYPixelSize(){return m_iXYPizelSize;}

    void setUpperZLimPU(int iZLimPU){m_iUpperZLimPU = iZLimPU;}
    int getUpperZLimPU(){return m_iUpperZLimPU;}

    void setCurZPosInPU(int iCZ){m_iCurZPosInPU = iCZ;}
    int getCurZPosInPU(){return m_iCurZPosInPU;}

    void setCurVatPercentOpen(int iPO){m_iCurVatPercentOpen = iPO;}
    int getCurVatPercentOpen(){return m_iCurVatPercentOpen;}

    int getLastHomeDiff() {return m_iLastHomeDiff;}
    void setLastHomeDiff(int iDiff) {m_iLastHomeDiff = iDiff;}

    void resetLastMsgTime() {lastMsgTime.start();}
    int getLastMsgElapsedTime() {return lastMsgTime.elapsed();}

    void resetLastProjCmdTime() {lastProjCmdTime.start();}
    int getLastProjCmdElapsedTime() {return lastProjCmdTime.elapsed();}

private:
    QTime lastMsgTime; // Updated on every serial readReady signal
    bool bResetInProgress; // Set to True during home location reset

    int iV1,iV2,iV3; // version values
    QString m_sModel; // Product Model Description
    bool m_bCanControlProjector; //set to true if printer reports capability to command projector
    bool m_bHasShutter; // set to true if printer reports capability to close a shutter
    int m_iLastHomeDiff; // When we reset to home this is Z expected - found (in PU)

    HomeStatus m_eHomeStatus; // Has home been located?
    ProjectorStatus m_eProjStatus; // What's the projector doing?

    bool m_bProjCmdOn;  //Set to true if we want the projector on, false if off
    QTime lastProjCmdTime; //Updated on every serial cmd to projector;
    int m_iLampHours;  // Total bulb time as reported by projector

    int m_iPU;
    int m_iUpperZLimPU;
    int m_iCurZPosInPU;
    int m_iCurVatPercentOpen;
    int m_iNativeX;
    int m_iNativeY;
    int m_iXYPizelSize;
    int m_iHalfLife;
};

/////////////////////////////////////////////////////////////////////////////
class B9FirmwareUpdate : public QObject
{
    Q_OBJECT

    // Uses avrdude to update B9Creator's Arduino Firmware
    // averdude and averdue.config must be present
    // B9FirmwareVxxx.hex file must be present
public:
    B9FirmwareUpdate()
    {
        sHexFilePath = FIRMWAREHEXFILE;
    }
    ~B9FirmwareUpdate(){}
    bool UploadHex(QString sCurPort);
private:
    QString sHexFilePath;
};

/////////////////////////////////////////////////////////////////////////////
//  The B9PrinterComm should be created once by the main window
//  and shared by and class that needs signal/slot access
class B9PrinterComm : public QObject
{
    Q_OBJECT

public:
    B9PrinterComm();
    ~B9PrinterComm();

    bool isConnected(){return m_Status.isValidVersion();}
    void enableBlankCloning(bool bEnable){m_bCloneBlanks = bEnable;}

    void cmdProjectorPowerOn(bool bOn){m_Status.cmdProjectorPowerOn(bOn);}
    bool isProjectorPowerCmdOn(){return m_Status.isProjectorPowerCmdOn();}
    B9PrinterStatus::ProjectorStatus getProjectorStatus(){return m_Status.getProjectorStatus();}
    int getLampHrs(){return m_Status.getLampHrs();}
    int getPU(){return m_Status.getPU();}
    int getUpperZLimPU(){return m_Status.getUpperZLimPU();}
    int getHalfLife(){return m_Status.getHalfLife();}
    int getNativeX(){return m_Status.getNativeX();}
    int getNativeY(){return m_Status.getNativeY();}
    int getXYPixelSize() {return m_Status.getXYPixelSize();}
    int getLastHomeDiff() {return m_Status.getLastHomeDiff();}
    B9PrinterStatus::HomeStatus getHomeStatus() {return m_Status.getHomeStatus();}
    void setHomeStatus(B9PrinterStatus::HomeStatus eHS) {m_Status.setHomeStatus(eHS);}
    QString errorString();

signals:
    void updateConnectionStatus(QString sText); // Connected or Searching
    void BC_ConnectionStatusDetailed(QString sText); // Connected or Searching
    void BC_LostCOMM(); // Lost previous connection!
    void BC_RawData(QString sText);    // Raw Data
    void BC_Comment(QString sComment); // Comment String
    void BC_HomeFound(); // Done with Reset, re-enable contols, etc.
    void BC_ProjectorStatusChanged(); // Projector status has changed
    void BC_ProjectorFAIL(); // Projector experienced and uncommanded power off

    void BC_ModelInfo(QString sModel); // Printer Model Description Info
    void BC_FirmVersion(QString sVersion); // Printer Firmware Version Number
    void BC_ProjectorRemoteCapable(bool isCapable); // Projector has ability to be remote controled?
    void BC_HasShutter(bool hasShutter); // Projector has a mechanical shutter
    void BC_PU(int); //Update to Printer Units (Microns * 100)
    void BC_UpperZLimPU(int); //Update to Upper Z Limit in PU
    void BC_HalfLife(int); //Update to Projector Lamp HalfLife
    void BC_NativeX(int); //Update to Native X resoltion
    void BC_NativeY(int); //Update to Native Y resoltion
    void BC_XYPixelSize(int); //Update to XY Pixel Size

    void BC_CurrentZPosInPU(int); //Broadcast whenever we get an update on the Z Position
    void BC_CurrentVatPercentOpen(int); //Broadcast whenever we get an update on the Vat Position

    void BC_PrintReleaseCycleFinished();  // Broadcast when we receive the "F" notice from the printer

public slots:
    void SendCmd(QString sCmd);
    void setProjectorPowerCmd(bool bPwrFlag); // call to send projector power on/off command
    void setWarmUpDelay(int iDelayMS){m_iWarmUpDelayMS = iDelayMS;}

private slots:
    void ReadAvailable();
    void RefreshCommPortItems();
    void watchDog();  // Checks to see we're receiving regular updates from the Ardunio

public:
    bool m_bIsPrinting;

private:
    QextSerialPort *m_serialDevice;
    QextSerialEnumerator *pEnumerator;		// enumerator for finding available comm ports
    QList<QextPortInfo> *pPorts;			// list of available comm ports
    QString sNoFirmwareAurdinoPort;         // if we locate an arduino without firmware, set this to the portname as a flag
    bool m_bCloneBlanks;                    // if false, we will not burn firmware into suspected blank Arduinos

    QString m_sSerialString; // Used by ReadAvailable to store current broadcast line
    B9PrinterStatus m_Status;

    bool OpenB9CreatorCommPort(QString sPortName);
    void startWatchDogTimer();
    void handleLostComm();
    bool handleProjectorBC(int iBC);
    QTime startWarmingTime;
    int m_iWarmUpDelayMS;
};
#endif // B9PRINTERCOMM_H
