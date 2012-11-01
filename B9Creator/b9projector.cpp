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
#include <QtGui>
#include "b9projector.h"

B9Projector::B9Projector(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	hideCursor();
	setMouseTracking(true);
    bGrid = true;
	mStatusMsg = "B9Creator - www.b9creator.com";
	mpCPJ = NULL;
	m_xOffset = m_yOffset = 0;
}

B9Projector::~B9Projector()
{

}

void B9Projector::hideEvent(QHideEvent *event)
{
    emit eventHiding();
    event->accept();
}

void B9Projector::showProjector(int x, int y, int w, int h)
{
	setWindowTitle("Preview");
//	setGeometry(x,y,w,h);
	////setFixedWidth(w);
	//setFixedHeight(h);
	show();
	setFocus();

	mImage = QImage(width(),height(),QImage::Format_RGB32);

	drawAll();
}

void B9Projector::showProjector(const QByteArray & geometry)
{
	setWindowTitle("Preview");
	show();
	bool b = restoreGeometry(geometry);
	setFocus();

	QImage newImage(width(),height(),QImage::Format_RGB32);
	QPainter painter(&newImage);
	painter.drawImage(QPoint(0,0), mImage);
	mImage = newImage;

	drawAll();
}

void B9Projector::setStatusMsg(QString status)
{
	if(mStatusMsg == status) return;
	mStatusMsg = status;
	drawAll();
}

void B9Projector::setShowGrid(bool bShow)
{
	if(bGrid == bShow) return;
	bGrid = bShow;
	drawAll();
}

void B9Projector::setCPJ(CrushedPrintJob * pCPJ)
{
	mpCPJ = pCPJ;
	drawAll();
}

void B9Projector::drawAll()
{
	blankProjector();
	drawGrid();
	drawStatusMsg();
	drawCBM();
	update();
}

void B9Projector::blankProjector()
{
	mImage.fill(qRgb(0,0,0));
}

void B9Projector::drawGrid()
{
	if(!bGrid) return;
	QPainter painter(&mImage);
	QColor color;
	color.setRgb(127,0,0);

	painter.setPen(QPen(color,1,Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin));
	
	for(int x=0; x < width(); x+=100)
		painter.drawLine(x,0,x,height());
	painter.drawLine(width()-1,0,width()-1,height());
	for(int y=0; y < height(); y+=100)
		painter.drawLine(0,y,width(),y);
	painter.drawLine(0,height()-1,width(),height()-1);
}

void B9Projector::drawStatusMsg()
{
    if(mStatusMsg.size()==0 || this->isHidden())return;
	QPainter painter(&mImage);
	QColor color;
	color.setRgb(127,0,0);
	painter.setPen(color);
	int leftOffset = 10;
	int fontHeight = 10;
	int bottomOffset = height() - 10;
	int buffer = fontHeight/2.2;
	painter.setFont(QFont("arial", fontHeight));
	QRect bounds;
	bounds = painter.boundingRect(leftOffset,bottomOffset-fontHeight,width(),fontHeight,Qt::TextDontClip,mStatusMsg);
	color.setRgb(0,0,0);
	painter.fillRect(leftOffset,bottomOffset-fontHeight-buffer,bounds.width(),fontHeight+2*buffer,color);
 	painter.drawText(QPoint(leftOffset,bottomOffset),mStatusMsg);
}

void B9Projector::drawCBM()
{
	if(mpCPJ==NULL) return;
	mpCPJ->inflateCurrentSlice(&mImage, m_xOffset, m_yOffset);
}


void B9Projector::paintEvent (QPaintEvent * pEvent)
{
	QPainter painter(this);
	QRect dirtyRect = pEvent->rect();
	painter.drawImage(dirtyRect, mImage, dirtyRect);
}


void B9Projector::keyReleaseEvent(QKeyEvent * pEvent)
{
	QWidget::keyReleaseEvent(pEvent);
	emit keyReleased(pEvent->key());
}

void B9Projector::mouseReleaseEvent(QMouseEvent * pEvent)
{
	QWidget::mouseReleaseEvent(pEvent);
	if(pEvent->button()==Qt::LeftButton)
		emit hideProjector();
}

void B9Projector::mouseMoveEvent ( QMouseEvent * pEvent )
{
	setCursor(Qt::CrossCursor);
	QTimer::singleShot(500, this, SLOT(hideCursor()));
	QWidget::mouseMoveEvent(pEvent);
}

void B9Projector::resizeEvent ( QResizeEvent * pEvent )
{	
	QImage newImage(width(),height(),QImage::Format_RGB32);
	QPainter painter(&newImage);
	painter.drawImage(QPoint(0,0), mImage);
	mImage = newImage;
	drawAll();
	QDesktopWidget dt;
	emit newGeometry (dt.screenNumber(), geometry());
}
