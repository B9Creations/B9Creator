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

#ifndef B9PRINT_H
#define B9PRINT_H

#include <QDialog>
#include <QDateTime>
#include <QHideEvent>
#include <QSettings>
#include "helpsystem.h"
#include "b9terminal.h"

namespace Ui {
class B9Print;
}

class B9Print : public QDialog
{
    Q_OBJECT
    
public:
    explicit B9Print(B9Terminal *pTerm, QWidget *parent = 0);
    ~B9Print();
    // If PrintPreview we do not power up the projector.  If UsePrimaryMonitor we force the output to the primary monitor
    void print3D(CrushedPrintJob *pCPJ, int iXOff, int iYOff, int iTbase, int iTover, int iTattach, int iNumAttach = 1, int iLastLayer = 0, bool bPrintPreview = false, bool bUsePrimaryMonitor = false);
    
signals:
    void eventHiding();

public slots:
    void setProjMessage(QString sText);
    QString updateTimes();

private slots:
    void showHelp();
    void on_updateConnectionStatus(QString sText);
    void on_updateProjectorOutput(QString sText);
    void on_updateProjectorStatus(QString sText);
    void on_updateProjector(B9PrinterStatus::ProjectorStatus eStatus);
    void on_signalAbortPrint();

    void exposeTBaseLayer();
    void startExposeTOverLayers();
    void exposureOfCurTintLayerFinished();
    void exposureOfTOverLayersFinished();

    void on_pushButtonPauseResume_clicked();

    void on_pushButtonAbort_clicked(QString sAbortText="User Directed Abort.");

private:
    enum {PRINT_NO, PRINT_SETUP1, PRINT_SETUP2, PRINT_RELEASING, PRINT_EXPOSING, PRINT_ABORT, PRINT_DONE};
    enum {PAUSE_NO, PAUSE_WAIT, PAUSE_YES};

    void keyPressEvent(QKeyEvent * pEvent);		// Handle key press events
    void hideEvent(QHideEvent *event);
    void closeEvent ( QCloseEvent * event );

    double curLayerIndexMM();
    void setSlice(int iSlice);

    HelpSystem m_HelpSystem;
    Ui::B9Print *ui;

    B9Terminal* m_pTerminal;
    CrushedPrintJob* m_pCPJ;
    int m_iTbase,  m_iTover, m_iTattach, m_iNumAttach;
    int m_iXOff, m_iYOff;
    int m_iPrintState;
    int m_iCurLayerNumber;
    int m_iLastLayer;
    double m_dLayerThickness;
    int m_iPaused;
    bool m_bAbort;
    QString m_sAbortMessage;
    double m_dTintMS;
    int m_iTintNum, m_iCurTintIndex, m_iMinimumTintMS,m_iMinimumTintMSWorstCase;
    QTime m_vClock;
    QSettings m_vSettings;

};

#endif // B9PRINT_H
