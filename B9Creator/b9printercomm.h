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
#ifndef B9PRINTERCOMM_H
#define B9PRINTERCOMM_H
#include <QObject>
#include <QTime>
#include <QtDebug>

// The Firmware version is tied to a specific version
// These defines determine how we attempt to update Firmware
// The correctly named current hex file MUST be in the application directory
#define CURRENTFIRMWARE "v1.0.1"
#define FIRMWAREHEXFILE "B9Firmware_1_0_1.hex"
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

    HomeStatus getHomeStatus() {return m_eHomeStatus;}
    void setHomeStatus(HomeStatus eHS) {m_eHomeStatus = eHS;}

    ProjectorStatus getProjectorStatus() {return m_eProjStatus;}
    void setProjectorStatus(ProjectorStatus ePS) {m_eProjStatus = ePS;}

    int getLastHomeDiff() {return m_iLastHomeDiff;}
    void setLastHomeDiff(int iDiff) {m_iLastHomeDiff = iDiff;}

    bool resumeOnReconnect() {return bDoResume;}
    void setResumeOnReconnect(bool bResume) {bDoResume = bResume;}

    void resetLastMsgTime() {lastMsgTime.start();}
    int getLastMsgElapsedTime() {return lastMsgTime.elapsed();}

    void resetLastProjCmdTime() {lastProjCmdTime.start();}
    int getLastProjCmdElapsedTime() {return lastProjCmdTime.elapsed();}

private:
    QTime lastMsgTime; // Updated on every serial readReady signal
    bool bResetInProgress; // Set to True during home location reset
    bool bDoResume; // set to true if we wish to resume after disconnect

    int iV1,iV2,iV3; // version values
    QString m_sModel; // Product Model Description
    int m_iLastHomeDiff; // When we reset to home this is Z expected - found (in PU)

    HomeStatus m_eHomeStatus; // Has home been located?
    ProjectorStatus m_eProjStatus; // What's the projector doing?

    bool m_bProjCmdOn;  //Set to true if we want the projector on, false if off
    QTime lastProjCmdTime; //Updated on every serial cmd to projector;
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
    B9PrinterStatus::ProjectorStatus getProjectorStatus(){return m_Status.getProjectorStatus();}

signals:
    void updateConnectionStatus(QString sText); // Connected or Searching
    void BC_RawData(QString sText);    // Raw Data
    void BC_Comment(QString sComment); // Comment String
    void BC_HomeFound(); // Done with Reset, re-enable contols, etc.
    void BC_ProjectorStatusChanged(); // Projector status has changed
    void BC_ProjectorFAIL(); // Projector experienced and uncommanded power off

public slots:
    void SendCmd(QString sCmd);

private slots:
    void ReadAvailable();
    void RefreshCommPortItems();
    void watchDog();  // Checks to see we're receiving regular updates from the Ardunio

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
    void restoreState();
    void handleProjectorBC(int iBC);
    QTime startWarmingTime;
};
#endif // B9PRINTERCOMM_H
