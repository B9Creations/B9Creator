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
#define CURRENTFIRMWARE "v1.0.0"
#define FIRMWAREHEXFILE "B9Firmware_1_0_0.hex"

class  QextSerialPort;
class  QextSerialEnumerator;
struct QextPortInfo;

/////////////////////////////////////////////////////////////////////////////
class B9PrinterStatus
{
public:
    B9PrinterStatus(){reset();}
    void reset();
    QString getVersion();
    void setVersion(QString s);
    bool isCurrentVersion();
    bool isValidVersion();

    void resetLastMsgTime() {lastMsgTime.start();}
    int getLastMsgElapsedTime() {return lastMsgTime.elapsed();}
    bool isFindingHome() {return bResetInProgress;}
    void setFindingHomeStatus(bool b) {bResetInProgress = b;}

private:
    QTime lastMsgTime; // Updated on every serial readReady signal
    bool bResetInProgress; // Set to True during home location reset
    int iV1,iV2,iV3; // version values
    int iResetState; // Has home been located?  0 no, 1 yes, -1 ????
    int iProjPwr; // 0 = off, 1 = on, -1 = ????
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

signals:
    void updateConnectionStatus(QString sText);
    void broadcastPrinterComm(QString sText);

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
};
#endif // B9PRINTERCOMM_H
