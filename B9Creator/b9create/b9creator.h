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
#ifndef B9CREATOR_H
#define B9CREATOR_H

#include <QtGui/QDialog>
#include <QDesktopWidget>
#include <QSettings>
#include "ui_b9creator.h"
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include "crushbitmap.h"
#include "b9projector.h"
#include "b9terminal.h"


class B9Creator : public QDialog
{
	Q_OBJECT

public:
    B9Creator(B9Terminal *pTerm, QWidget *parent = 0, Qt::WFlags flags = Qt::WindowSystemMenuHint|Qt::WindowTitleHint|Qt::MSWindowsFixedSizeDialogHint|Qt::WindowMinimizeButtonHint);
	~B9Creator();
	void makeConnections();
	QDesktopWidget *pDesktop;
    B9Projector* pProjector;


private:
    B9Terminal *pTerminal;

    void hideEvent(QHideEvent *event);
    Ui::B9CreatorClass ui;
	void closeEvent(QCloseEvent *e);

	QSettings* mpSettings;					// pointer to the persistant QSettings where we store preferences

	CrushedPrintJob mCPJ;					// The job data

    QextSerialPort *commPort;				// commPort for serial comm with B9Creator, null if not connected
	QextSerialEnumerator *pEnumerator;		// enumerator for finding available comm ports
	QList<QextPortInfo> *pPorts;			// list of available comm ports
	void openCommPort(QString sPortName);	// Open the comm port if possible, save result to settings
	void setCommPortItems();				// Resets the Comm Ports combo box to reflect the available comm ports
	void enableB9Controls(bool bEnable);	// Controls are disabled if printer is not connected

	void analyzeEstimates();					// Computes and updates build volume and duration based on user inputs
	void updateSliceSlider();					// Reset the slider min, max & current position based on user inputs
	bool isInverted();							// Returns true if the first layer index > last layer index
	int homeIndex();							// Returns the first layer index
	int endIndex();								// Returns the last layer index
	int getNextIndex(int curIndex, int step=1);	// Returns the current index + step

	//TODO THIS IS HACK  REDO!
	QString curLayerIndex();
	void breatheLayer();
	int m_iCurLayerNumber;
	float m_fLayerThickness;
	int m_iPrintState;
	enum {PRINT_NO, PRINT_MOV2READY, PRINT_WAITFORP, PRINT_EXPOSING, PRINT_MOV2NEXT};
	QString m_sSerialString;
	int iPrinterUnits;
	bool m_bPaused, m_bAbort;
	bool m_bPrimaryScreen;


public slots:
	void commPortChanged(int iSelection);	// Signaled when the user selects a different comm port
    void onReadyRead();						// Signaled when data is available on the comm port
	void commPortInput();					// User has entered a command
	void sendCmd(QString sCMD);				// Writes sCMD + '\n' to the commPort
	void loadPrintJob();					// Attempts to open a .b9j job file

	void newFirstLayer();					// Signaled if user enters a new first layer number
	void newLastLayer();					// Signaled if user enters a new last layer number

	void newExposure();						// Signaled if user enters a new normal layer exposure number
	void newTotalAttachLayers();			// Signaled if user enters a new number of "attach layers"
	void newALExposure();					// Signaled if user enters a new Attach Layer Exposure time
	void newBreathe();						// Signaled if user enters a new PDMS Breathe time

	void previewPrintJob();					// Signaled when user presses the Preview button

	void setThisFocus();	

	void updateGrid(bool bshow);			// Signaled when the show Grid check box changes
	void getKey(int iKey);					// Signaled when we receive a (released) key from the projector
	void hideProjector();					// Signal from the projector window that the use wishes to close it
	void screenCountChanged(int i);			// Signal that the number of monitors has changed

	void setSlice(int iSlice);				// Signal that the slice slider bar has changed;
	void setXoff(int xOff);					// Signal that the x offset has been adjusted
	void setYoff(int yOff);					// Signal that the y offset has been adjusted
	void centerOffsets();					// Singal that the center Offsets button has been pressed

	//TODO THIS IS HACK  REDO!
	void startPrint();						// Signal that print is commanded
	void exposureFinished();
	void exposeLayer();
	void resetPrinter();
	void projPwrON();
	void projPwrOFF();
	void vatOpen();
	void vatClose();
	void goFillPos();
	void printAbort();


signals:

    void setProjectorPowerCmd(bool bPwrFlag);

    void eventHiding();
    void showProjector(int x, int y, int w, int h);		// signal to the Projector window to show itself
	//void showProjector(const QByteArray & geometry);	// signal to the Projector window to show itself
	void SetProjectorFullScreen(bool);
	void sendStatusMsg(QString text);					// signal to the Projector window to change the status msg
	void sendGrid(bool bshow);							// signal to the Projector window to update the grid display
	void sendCPJ(CrushedPrintJob * pCPJ);				// signal to the Projector window to show a crushed bit map image
	void sendXoff(int xOff);							// signal to the Projector window to update the X offset
	void sendYoff(int yOff);							// signal to the Projector window to update the Y offset

	void transmitCmd(QString sCMD);						// Signal to send serial command
};

#endif // B9CREATOR_H
