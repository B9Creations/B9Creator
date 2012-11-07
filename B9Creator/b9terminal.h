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
#ifndef B9TERMINAL_H
#define B9TERMINAL_H

#include <QDesktopWidget>
#include <QtGui/QWidget>
#include <QHideEvent>
#include <QTimer>
#include "b9printercomm.h"
#include "logfilemanager.h"
#include "b9projector.h"

namespace Ui {
class B9Terminal;
}

class B9Terminal : public QWidget
{
    Q_OBJECT
    
public:
    explicit B9Terminal(QWidget *parent = 0, Qt::WFlags flags = Qt::Widget);
    ~B9Terminal();

    int getEstBaseCycleTime(int iDelta, int iDwnSpd=85, int iClsSpd=100, int iSettle=0);
    int getEstNextCycleTime(int iDelta, int iUpSpd=85, int iDwnSpd=85, int iOpnSpd=100, int iClsSpd=100, int iBreathe=0, int iSettle=0, int iGap=0);
    int getEstFinalCycleTime(int iDelta, int iUpSpd=85, int iClsSpd=100);

public slots:
    void rcProjectorPwr(bool bPwrOn);
    void rcResetHomePos();

    void showIt(){show();setEnabledWarned();}

signals:
    void signalAbortPrint(QString sMessage);

    void sendStatusMsg(QString text);					// signal to the Projector window to change the status msg
    void sendGrid(bool bshow);							// signal to the Projector window to update the grid display
    void sendCPJ(CrushedPrintJob * pCPJ);				// signal to the Projector window to show a crushed bit map image
    void sendXoff(int xOff);							// signal to the Projector window to update the X offset
    void sendYoff(int yOff);							// signal to the Projector window to update the Y offset

    void updateConnectionStatus(QString sText); // Connected or Searching
    void eventHiding();

private slots:
    void onScreenCountChanged(int iCount = 0);  // Signal that the number of monitors has changed
    void makeProjectorConnections();
    void getKey(int iKey);					    // Signal that we received a (released) key from the projector


    void on_pushButtonProjPower_toggled(bool checked);  //Remote slot for turning projector on/off
    void on_pushButtonCmdReset_clicked(); // Remote slot for commanding Reset (find home) motion
    void sendCommand();
    void setProjectorPowerCmd(bool bPwrFlag); // call to send projector power on/off command
    void onUpdateConnectionStatus(QString sText);
    void onBC_ConnectionStatusDetailed(QString sText);
    void onUpdatePrinterComm(QString sText);
    void onUpdateRAWPrinterComm(QString sText);
    void onBC_LostCOMM();
    void onBC_ProjStatusChanged();
    void onBC_ProjStatusFAIL();

    void onMotionResetTimeout();
    void onMotionResetComplete();

    void onMotionVatTimeout();

    void onBC_ModelInfo(QString sModel);
    void onBC_FirmVersion(QString sVersion);
    void onBC_ProjectorRemoteCapable(bool bCapable);
    void onBC_HasShutter(bool bHS);
    void onBC_PU(int iPU);
    void onBC_UpperZLimPU(int iUpZLimPU);
    void onBC_CurrentZPosInPU(int iCZ);
    void onBC_CurrentVatPercentOpen(int iPO);

    void setTgtAltitudePU(int iTgtPU);
    void setTgtAltitudeMM(double iTgtMM);
    void setTgtAltitudeIN(double iTgtIN);

    void onBC_PrintReleaseCycleFinished();
    void onReleaseCycleTimeout();

    void on_lineEditZRaiseSpd_editingFinished();

    void on_lineEditZLowerSpd_editingFinished();

    void on_lineEditVatOpenSpeed_editingFinished();

    void on_lineEditVatCloseSpeed_editingFinished();

    void on_lineEditDelayClosedPos_editingFinished();

    void on_lineEditDelayOpenPos_editingFinished();

    void on_lineEditOverLift_editingFinished();

    void on_lineEditTgtZPU_editingFinished();

    void on_lineEditTgtZMM_editingFinished();

    void on_lineEditTgtZInches_editingFinished();

    void on_lineEditCurZPosInPU_returnPressed();

    void on_lineEditCurZPosInMM_returnPressed();

    void on_lineEditCurZPosInInches_returnPressed();

    void on_pushButtonStop_clicked();

    void on_checkBoxVerbose_clicked(bool checked);

    void on_pushButtonVOpen_clicked();

    void on_pushButtonVClose_clicked();

    void on_lineEditVatPercentOpen_returnPressed();

    void on_pushButtonPrintBase_clicked();

    void on_pushButtonPrintNext_clicked();

    void on_pushButtonPrintFinal_clicked();

    void SetCycleParameters();

private:
    Ui::B9Terminal *ui;
    void hideEvent(QHideEvent *event);

    int getZMoveTime(int iDelta, int iSpd);
    int getVatMoveTime(int iSpeed);

    B9PrinterComm *pPrinterComm;
    LogFileManager *pLogManager;
    B9Projector *pProjector;
    QDesktopWidget* m_pDesktop;
    bool m_bPrimaryScreen;

    QTimer *m_pResetTimer;
    QTimer *m_pPReleaseCycleTimer;
    QTimer *m_pVatTimer;

    void setEnabledWarned(); // Set the enabled status based on connection and user response
    bool m_bWaiverPresented;
    bool m_bWaiverAccepted;
    bool m_bWavierActive;
};

#endif // B9TERMINAL_H
