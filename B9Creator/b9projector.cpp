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
#include <QByteArray>
#include "b9projector.h"

B9Projector::B9Projector(bool bPrintWindow, QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	hideCursor();
	setMouseTracking(true);
    m_bGrid = true;
	mStatusMsg = "B9Creator - www.b9creator.com";
	mpCPJ = NULL;
	m_xOffset = m_yOffset = 0;
    m_bIsPrintWindow = bPrintWindow;
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
    setGeometry(x,y,w,h);
    if(m_bIsPrintWindow) showFullScreen(); else show();
    mImage = QImage(width(),height(),QImage::Format_ARGB32_Premultiplied);
    createNormalizedMask();
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
    if(m_bGrid == bShow) return;
    m_bGrid = bShow;
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
    if(!m_bGrid) return;
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

/*
void B9Projector::timingTest()
{
    QByteArray timingMask;
    timingMask.resize(1024*768);
    QMessageBox msg;

    m_NormalizedMask = QImage(1024,768,QImage::Format_ARGB32_Premultiplied);
    m_NormalizedMask.fill(qRgba(0,0,0,0));

    QImage tImage = QImage(1024,768,QImage::Format_ARGB32_Premultiplied);
    tImage.fill(qRgba(0,0,0,0));

    QTime t;
    t.start();

    int igs=222;
    for (quint32 y = 0; y < (quint32)m_NormalizedMask.height(); ++y) {
        QRgb *scanLine = (QRgb *)m_NormalizedMask.scanLine(y);
        for (quint32 x = 0; x < (quint32)m_NormalizedMask.width(); ++x){
            if((int)timingMask[x+y*1024]>233)
                scanLine[x] = qRgba(igs,igs,igs,255);
            else
                scanLine[x] = qRgba(igs,igs,igs,255);
        }
    }

    // Here we copy the resulting normalized slice to the mImage
    QPainter mPainter2(&mImage);
    mPainter2.setCompositionMode(QPainter::CompositionMode_SourceOver);
    mPainter2.drawImage(0,0,tImage);

    msg.setText(QString::number(t.elapsed()));
    msg.exec();

}
*/

void B9Projector::createNormalizedMask( double XYPS, double dZ, double dOhMM)
{
//    dZ=20;
    if(mpCPJ!=NULL)
        XYPS = mpCPJ->getXYPixelmm();
    double xsqrmm, ysqrmm;
    double zsqrmm = dZ*dZ;
    double Oh = dOhMM;
    double Ol = width()*XYPS*0.5;
    double dDimRg = 255.0/sqrt(Ol*Ol + Oh*Oh + zsqrmm);

    //Here we create a gray scale mask to normalize the projector's output
    m_NormalizedMask = QImage(width(),height(),QImage::Format_ARGB32_Premultiplied);
    m_NormalizedMask.fill(qRgba(0,0,0,0));
    int igs;
    for (quint32 y = 0; y < (quint32)m_NormalizedMask.height(); ++y) {
        QRgb *scanLine = (QRgb *)m_NormalizedMask.scanLine(y);
        for (quint32 x = 0; x < (quint32)m_NormalizedMask.width(); ++x){
            xsqrmm = pow((-Ol + (double)x*0.1),2.0);
            ysqrmm = pow(( Oh - (double)y*0.1),2.0);
            igs = sqrt(xsqrmm+ysqrmm+zsqrmm)*dDimRg;
            scanLine[x] = qRgba(igs,igs,igs,255);
        }
    }
}

void B9Projector::drawCBM()
{
	if(mpCPJ==NULL) return;
    // Here we inflate the slice
    QImage tImage = QImage(width(),height(),QImage::Format_ARGB32_Premultiplied);
    if(true) // Set to false to test full normalized screen.
    {
        tImage.fill(qRgba(0,0,0,0));
        mpCPJ->inflateCurrentSlice(&tImage, m_xOffset, m_yOffset);
    }
    else tImage.fill(qRgba(255,255,255,255));

    // Here we copy the gray scale over using the slice as a mask
    QPainter mPainter(&tImage);
    mPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    mPainter.drawImage(0,0,m_NormalizedMask);

    // Here we copy the resulting normalized slice to the mImage
    QPainter mPainter2(&mImage);
    mPainter2.setCompositionMode(QPainter::CompositionMode_SourceOver);
    mPainter2.drawImage(0,0,tImage);
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
    if(pEvent->modifiers()!=Qt::ShiftModifier)
        emit keyReleased(pEvent->key()+32);
    else
        emit keyReleased(pEvent->key());
}

void B9Projector::mouseReleaseEvent(QMouseEvent * pEvent)
{
	QWidget::mouseReleaseEvent(pEvent);
    if(pEvent->button()==Qt::LeftButton && !m_bIsPrintWindow)
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
    QImage newImage(width(),height(),QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&newImage);
	painter.drawImage(QPoint(0,0), mImage);
	mImage = newImage;
    createNormalizedMask();
	drawAll();
	QDesktopWidget dt;
	emit newGeometry (dt.screenNumber(), geometry());
}
