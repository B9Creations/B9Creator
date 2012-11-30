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

#ifndef B9PROJECTOR_H
#define B9PROJECTOR_H

#include <QWidget>
#include <QHideEvent>
#include <QImage>
#include <QColor>
#include <QByteArray>
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
    bool clearTimedPixels(int iLevel);              // based on the level (0-255) we clear all pixels with Tover array values < iLevel
    void createToverMap(int iRadius);
    void setXoff(int xOff){m_xOffset = xOff;drawAll();} // x offset for layer image
    void setYoff(int yOff){m_yOffset = yOff;drawAll();} // y offset for layer image
    void createNormalizedMask(double XYPS=0.1, double dZ = 257.0, double dOhMM = 91.088); //call when we show or resize

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

    void createToverMap0();
    void createToverMap1();
    void createToverMap2();
    void createToverMap3();
	
    bool m_bIsPrintWindow;  // set to true if we lock the window to full screen when shown
    bool m_bGrid;			// if true, grid is to be drawn
	CrushedPrintJob* mpCPJ;	// CPJ to inflate CBM from
    QImage mImage;
    QImage mCurSliceImage;  // Current Normalized slice, possible that has some or all pixels cleared
    int m_iLevel;
    QImage m_NormalizedMask;
	QString mStatusMsg;
	int m_xOffset, m_yOffset;
    QByteArray m_vToverMap;

};

#endif // B9PROJECTOR_H
