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

#ifndef B9TERMINAL_H
#define B9TERMINAL_H

#include <QDesktopWidget>
#include <QtGui/QWidget>
#include <QHideEvent>
#include <QTimer>
#include <QTime>
#include "b9printercomm.h"
#include "logfilemanager.h"
#include "b9projector.h"
#include "b9matcat.h"


class PCycleSettings {
public:
    PCycleSettings(){loadSettings();}
    ~PCycleSettings(){}
    void updateValues(); // Opens a dialog and allows user to change settings

    void loadSettings();
    void saveSettings();
    void setFactorySettings();
    int m_iRSpd1, m_iLSpd1, m_iRSpd2, m_iLSpd2;
    int m_iOpenSpd1, m_iCloseSpd1, m_iOpenSpd2, m_iCloseSpd2;
    double m_dBreatheClosed1, m_dSettleOpen1, m_dBreatheClosed2, m_dSettleOpen2;
    double m_dOverLift1, m_dOverLift2;
    double m_dBTClearInMM;
    double m_dHardZDownMM;
    double m_dZFlushMM;
};

namespace Ui {
class B9Terminal;
}

class B9Terminal : public QWidget
{
    Q_OBJECT
    
public:
    explicit B9Terminal(QWidget *parent = 0, Qt::WFlags flags = Qt::Widget);
    ~B9Terminal();

    bool isConnected(){return pPrinterComm->isConnected();}

    double getHardZDownMM(){return pSettings->m_dHardZDownMM;}
    double getZFlushMM(){return pSettings->m_dZFlushMM;}

    int getEstBaseCycleTime(int iCur, int iTgt);
    int getEstNextCycleTime(int iCur, int iTgt);
    int getEstFinalCycleTime(int iCur, int iTgt);

    QTime getEstCompleteTime(int iCurLayer, int iTotLayers, double dLayerThicknessMM, int iExposeMS);
    int getEstCompleteTimeMS(int iCurLayer, int iTotLayers, double dLayerThicknessMM, int iExposeMS);
    int getLampAdjustedExposureTime(int iBaseTimeMS);

    B9MatCat* getMatCat() {return m_pCatalog;}

    void setUsePrimaryMonitor(bool bFlag){m_bUsePrimaryMonitor=bFlag; }
    bool getUsePrimaryMonitor(){return m_bUsePrimaryMonitor;}
    void setPrintPreview(bool bFlag){m_bPrintPreview=bFlag; }
    bool getPrintPreview(){return m_bPrintPreview;}
    void createNormalizedMask(double XYPS=0.1, double dZ = 257.0, double dOhMM = 91.088); //call when we show or resize

    int getXYPixelSize(){return pPrinterComm->getXYPixelSize();}

    void setIsPrinting(bool bFlag){
        pPrinterComm->m_bIsPrinting = bFlag;}

public slots:
    void dlgEditMatCat();

    void rcResetHomePos();
    void rcResetCurrentPositionPU(int iCurPos);
    void rcBasePrint(double dBaseMM); // Position for Base Layer Exposure.
    void rcNextPrint(double dNextMM); // Position for Next Layer Exposure.
    void rcFinishPrint(double dDeltaMM); // Calculates a final Z position at current + dDelta, closes vat, raises z, turns off projector.
    void rcSTOP();
    void rcCloseVat();
    void rcSetWarmUpDelay(int iDelayMS);

    void rcProjectorPwr(bool bPwrOn);
    void rcSetCPJ(CrushedPrintJob *pCPJ); // Set the pointer to the CMB to be displayed, NULL if blank
    void rcCreateToverMap(int iRadius){pProjector->createToverMap(iRadius);}
    bool rcClearTimedPixels(int iLevel){return pProjector->clearTimedPixels(iLevel);}
    void rcSetProjMessage(QString sMsg);
    void rcGotoFillAfterReset(int iFillLevel);

    void showIt(){show();setEnabledWarned();}
    void onScreenCountChanged(int iCount = 0);  // Signal that the number of monitors has changed

    void updateCycleValues(){pSettings->updateValues();}

signals:
    void signalAbortPrint(QString sMessage);
    void pausePrint();
    void updateConnectionStatus(QString sText); // Connected or Searching
    void updateProjectorOutput(QString sText);  // Data on video to projector connection
    void updateProjectorStatus(QString sText);  // Projector Power Status Changes
    void updateProjector(B9PrinterStatus::ProjectorStatus eStatus);
    void PrintReleaseCycleFinished();

    void sendStatusMsg(QString text);					// signal to the Projector window to change the status msg
    void sendGrid(bool bshow);							// signal to the Projector window to update the grid display
    void sendCPJ(CrushedPrintJob * pCPJ);				// signal to the Projector window to show a crushed bit map image
    void sendXoff(int xOff);							// signal to the Projector window to update the X offset
    void sendYoff(int yOff);							// signal to the Projector window to update the Y offset

    void eventHiding();

private slots:
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
    void onBC_HalfLife(int iHL);
    void onBC_NativeX(int iNX);
    void onBC_NativeY(int iNY);
    void onBC_XYPixelSize(int iPS);

    void setTgtAltitudePU(int iTgtPU);
    void setTgtAltitudeMM(double iTgtMM);
    void setTgtAltitudeIN(double iTgtIN);

    void onBC_PrintReleaseCycleFinished();
    void onReleaseCycleTimeout();

    void on_lineEditTgtZPU_editingFinished();

    void on_lineEditTgtZMM_editingFinished();

    void on_lineEditTgtZInches_editingFinished();

    void on_lineEditCurZPosInPU_returnPressed();

    void on_lineEditCurZPosInMM_returnPressed();

    void on_lineEditCurZPosInInches_returnPressed();

    void on_spinBoxVatPercentOpen_editingFinished();

    void on_pushButtonVOpen_clicked();

    void on_pushButtonVClose_clicked();

    void on_pushButtonPrintBase_clicked();

    void on_pushButtonPrintNext_clicked();

    void on_pushButtonPrintFinal_clicked();

    void SetCycleParameters();

    void on_pushButtonStop_clicked();

    void on_checkBoxVerbose_clicked(bool checked);

    void on_pushButtonCycleSettings_clicked();

    void on_comboBoxXPPixelSize_currentIndexChanged(int index);

    void on_pushButtonModMatCat_clicked();


private:
    Ui::B9Terminal *ui;
    void hideEvent(QHideEvent *event);

    int getZMoveTime(int iDelta, int iSpd);
    int getVatMoveTime(int iSpeed);

    B9MatCat* m_pCatalog;
    QString m_sModelName;
    PCycleSettings *pSettings;
    int m_iD, m_iE, m_iJ, m_iK, m_iL, m_iW, m_iX;
    void resetLastSentCycleSettings();

    B9PrinterComm *pPrinterComm;
    LogFileManager *pLogManager;
    B9Projector *pProjector;
    QDesktopWidget* m_pDesktop;
    bool m_bPrimaryScreen;
    bool m_bPrintPreview;
    bool m_bUsePrimaryMonitor;

    QTimer *m_pResetTimer;
    QTimer *m_pPReleaseCycleTimer;
    QTimer *m_pVatTimer;

    int m_iFillLevel;

    void setEnabledWarned(); // Set the enabled status based on connection and user response
    void warnSingleMonitor();
    bool m_bWaiverPresented;
    bool m_bWaiverAccepted;
    bool m_bWavierActive;
    bool m_bNeedsWarned;
};

#endif // B9TERMINAL_H
