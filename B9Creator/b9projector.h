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
#ifndef B9PROJECTOR_H
#define B9PROJECTOR_H

#include <QWidget>
#include <QHideEvent>
#include <QImage>
#include <QColor>
#include "crushbitmap.h"

class B9Projector : public QWidget
{
	Q_OBJECT

public:
    B9Projector(bool bPrintWindow, QWidget *parent = 0, Qt::WFlags flags = Qt::WindowMinMaxButtonsHint|Qt::Window|Qt::WindowCloseButtonHint);
    ~B9Projector();

public slots:
    void showProjector(int x, int y, int w, int h);	// Show ourself when we are signaled

    void hideCursor(){setCursor(Qt::BlankCursor);}	// Hide the mouse cursor when signaled

	void setShowGrid(bool bShow);					// Set bShow to true if Grid is to be drawn
	void setStatusMsg(QString status);				// Set status to the message to be displayed
	void setCPJ(CrushedPrintJob *pCPJ);				// Set the pointer to the CMB to be displayed, NULL if blank
    void setXoff(int xOff){m_xOffset = xOff;drawAll();} // x offset for layer image
    void setYoff(int yOff){m_yOffset = yOff;drawAll();} // y offset for layer image
    void createNormalizedMask(double XYPS=0.1, double dZ = 257.0, double dOhMM = 91.088); //call when we show or resize
    //void timingTest();

signals:
    void eventHiding();             // signal to the parent that we are being hidden
    void hideProjector();			// signal to the parent requesting we be hidden

    void keyReleased (int iKey);	// signal that a key has been pressed and released
    void newGeometry (int iScreenNumber, QRect geoRect);

private:
    void keyReleaseEvent(QKeyEvent * pEvent);		// Handle key Released events
    void mouseReleaseEvent(QMouseEvent * pEvent);	// Handle mouse button released events
    void mouseMoveEvent(QMouseEvent * pEvent);		// Handle mouse movement events
    void paintEvent (QPaintEvent * pEvent);			// Handle paint events
    void resizeEvent ( QResizeEvent * event );      // Handle resize events

    void hideEvent(QHideEvent *event);
	void drawAll();			// refresh the entire screen from scratch
	void blankProjector();	// fills the background in with black, overwrites previous image
	void drawGrid();		// draws a grid pattern using mGridColor
	void drawStatusMsg();	// draws the current status msg on the projector screen
	void drawCBM();			// draws the current CBM pointed to by mpCBM, returns if mpCBM is null

	
    bool m_bIsPrintWindow;  // set to true if we lock the window to full screen when shown
    bool m_bGrid;			// if true, grid is to be drawn
	CrushedPrintJob* mpCPJ;	// CPJ to inflate CBM from
	QImage mImage;
    QImage m_NormalizedMask;
	QString mStatusMsg;
	int m_xOffset, m_yOffset;
};

#endif // B9PROJECTOR_H
