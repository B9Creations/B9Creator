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

#include "crushbitmap.h"

#include <QDataStream>
#include <QtDebug>
#include <QtCore/qmath.h>
#include <QPainter>


////////////////////////////////////////////////////////////
//
// SimpleSupport functions
//
///////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// Render the support to the image
void SimpleSupport::draw(QImage* pImage) {
	int ihalfSize = mSize/2;
	float ihalfbase = mSize/1.73205081;
	QPointF points[4];
	QPainter tPainter(pImage);
	tPainter.setPen(QColor(255,255,255));
	tPainter.setBrush(QBrush(QColor(255,255,255)));
	tPainter.setBackgroundMode(Qt::OpaqueMode);

	switch(mType) {
	case st_CIRCLE:	
		tPainter.drawEllipse(mPoint.x()-ihalfSize,mPoint.y()-ihalfSize,mSize,mSize);
		break;
	case st_SQUARE:
		tPainter.fillRect(mPoint.x()-ihalfSize,mPoint.y()-ihalfSize,mSize,mSize,Qt::white);
		break;
	case st_TRIANGLE:
		points[0].setX(             mPoint.x());	points[0].setY(-ihalfSize + mPoint.y());
		points[1].setX( ihalfbase + mPoint.x());	points[1].setY( ihalfSize + mPoint.y());
		points[2].setX(-ihalfbase + mPoint.x());	points[2].setY( ihalfSize + mPoint.y());
		tPainter.drawPolygon(points, 3);
		break;
	case st_DIAMOND:
		points[0].setX(             mPoint.x());	points[0].setY(-ihalfSize + mPoint.y());
		points[1].setX( ihalfbase + mPoint.x());	points[1].setY(             mPoint.y());
		points[2].setX(             mPoint.x());	points[2].setY( ihalfSize + mPoint.y());
		points[3].setX(-ihalfbase + mPoint.x());	points[3].setY(             mPoint.y());
		tPainter.drawPolygon(points, 4);
		break;
	default:
		break;
	}
}

QImage SimpleSupport::getCursorImage(){
	QImage cursor(32,32,QImage::Format_ARGB32);
	cursor.fill(QColor(0,0,0,0));
	draw(&cursor);
	return cursor;
}

void SimpleSupport::streamOutSupport(QDataStream* pOut){
	*pOut << mPoint << (quint32)mType << (quint32)mSize << (quint32)mStart << (quint32)mEnd;
}
void SimpleSupport::streamInSupport(QDataStream* pIn){
	quint32 temp;
	*pIn >> mPoint >> temp >> mSize >> mStart >> mEnd;
	mType = (SupportType)temp;
}

////////////////////////////////////////////////////////////
//
// CrushedBitMap functions
//
///////////////////////////////////////////////////////////

CrushedBitMap::CrushedBitMap(QImage* pImage)
{
	mIndex = 0;
	uiWhitePixels = 0;
	crushSlice(pImage);
	m_bIsBaseLayer=false;
}

CrushedBitMap::CrushedBitMap(QPixmap* pPixmap)
{
	mIndex = 0;
	uiWhitePixels = 0;
	crushSlice(pPixmap);
	m_bIsBaseLayer=false;
}

bool CrushedBitMap::saveCrushedBitMap(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly))
		return false;

	QDataStream out(&file);

	streamOutCMB(&out);
	return true;
}

void CrushedBitMap::streamOutCMB(QDataStream* pOut)
{
	*pOut << (quint32)uiWhitePixels << mExtents << mBitarray;
}

bool CrushedBitMap::loadCrushedBitMap(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly))
		return false;

	QDataStream in(&file);
	streamInCMB(&in);
	return true;
}

void CrushedBitMap::streamInCMB(QDataStream* pIn)
{
	*pIn >> uiWhitePixels >> mExtents >> mBitarray;
	mIndex = 0; // Reset to start
	m_iWidth  = popBits(16);
	m_iHeight = popBits(16);
	mIndex = 0;
}

void CrushedBitMap::inflateSlice(QImage* pImage, int xOffset, int yOffset, bool bUseNaturalSize)
{
    if(pImage == NULL) return; // No image to draw

    // Not using the slice's natural size, so pImage should have some size greater than zero!
    if(!bUseNaturalSize && (pImage->width()<=0 || pImage->height()<=0))
        return;


	mIndex = 0; // Reset to start
	bool bCurColorIsWhite = false;
	unsigned uCurrentPos = 0;
	unsigned uImageSize = 0;
	unsigned uData = 0;
    int iKey;

	if(!m_bIsBaseLayer){
        // not a fake base layer, so inflate the natural width and height info and the first color (white or not?)
		m_iWidth = popBits(16);
		m_iHeight = popBits(16);
		bCurColorIsWhite = (popBits(1)==1);
		uImageSize = m_iWidth * m_iHeight;
	}

    if(bUseNaturalSize)
    {
        if(m_iWidth<1 || m_iHeight<1)
        {
            m_iWidth = pImage->width();
            m_iHeight = pImage->height();
            *pImage = QImage(m_iWidth,m_iHeight,QImage::Format_ARGB32_Premultiplied);
            pImage->fill(qRgba(0,0,0,255));
            return;
        }
        // Using the slice's natural size, so we create a new QImage of this size and fill it with black pixels
        *pImage = QImage(m_iWidth,m_iHeight,QImage::Format_ARGB32_Premultiplied);
        pImage->fill(qRgba(0,0,0,255));
	}

    // note, WinXXX will equal SliceXXX if natural sized
	float WinWidth = pImage->width()/2.0;
	float WinHeight= pImage->height()/2.0;
	float SliceWidth = m_iWidth/2.0;
	float SliceHeight = m_iHeight/2.0;
    // Find centering offsets (zero if equal sizes
	int widthOff = (int)(WinWidth - SliceWidth);
	int heightOff = (int)(WinHeight - SliceHeight);
    // Find adjusted offsets (from centered) based on input parameters
	m_xOffset = widthOff  + xOffset;
	m_yOffset = heightOff + yOffset;

    if(m_bIsBaseLayer) return;  // If it's a base layer, we're done (no "non-support" image to inflate)

    // inflate the first pixel run info (uData = run length)
	iKey = popBits(5);
	uData = popBits(iKey+1);

    while((uData > 0) && (uCurrentPos < uImageSize)) {  //if uData <= 0, we're done.  if uCurrentPos >= uImage Size we've set the last pixel
		for(unsigned ui=0; ui<uData; ui++) {
            if(bCurColorIsWhite) setWhiteImagePixel(pImage, uCurrentPos);  // Set the corresponding pixel in pImage
            uCurrentPos++; //Note we could exceed uImageSize but setWhiteImagePixel above will safely ignore this error
		}
		// pop length of next run of pixels
		iKey = popBits(5);
		uData = popBits(iKey+1);
		// toggle current color
		if(bCurColorIsWhite) bCurColorIsWhite = false; else	bCurColorIsWhite = true;
	}

	return;
}

void CrushedBitMap::setWhiteImagePixel(QImage* pImage, unsigned uCurPos)
{
	// define a white pixel
    QRgb whitePixel = qRgb(255,255,255);
    // determine it's location x,y
	int x, y;
	y = uCurPos / m_iWidth;
	x = uCurPos - y*m_iWidth;
	x += m_xOffset;
	y += m_yOffset;
    // set it to white, if valid x,y
	if(x>=0 && y>=0 && x<pImage->width() && y<pImage->height()) {
		pImage->setPixel(x,y,whitePixel);
    }
}

bool CrushedBitMap::crushSlice(QPixmap* pPixmap)
{
    QImage img = pPixmap->toImage();
    bool bResult = crushSlice(&img);
	return bResult;
}

bool CrushedBitMap::crushSlice(QImage* pImage)
{
	if(pImage == NULL) return false;

	m_iWidth=pImage->width();
	m_iHeight=pImage->height();

	unsigned uCurrentPos = 0;
	unsigned uImageSize = m_iWidth * m_iHeight;
	unsigned uData = 0;
	int iKey;
	uiWhitePixels = 0;
    bool bCurColorIsWhite = pixelIsWhite(pImage, 0);

	// reset the bit array
	if (mBitarray.size()>0) mBitarray.resize(0);
	mIndex = 0;

	// reset extents
	mExtents.setBottomRight(QPoint(0,0));
	mExtents.setTopLeft(QPoint(pImage->width(),pImage->height()));

	// push the image width and height
	pushBits(pImage->width(),16);
	pushBits(pImage->height(),16);

	// push the first colorbit (0 or 1)
	pushBits(bCurColorIsWhite, 1);

	// loop through all the pixels in the image
	do {
		uData = 0;
        bCurColorIsWhite = pixelIsWhite(pImage, uCurrentPos); //returns true if pixel is white, updates extents
		// count the pixels until the color changes
		while ((uCurrentPos < uImageSize) && (bCurColorIsWhite == pixelIsWhite(pImage, uCurrentPos))) {
			uCurrentPos++;
            uData++;
			if(bCurColorIsWhite)uiWhitePixels++;
		}
		// store the count
        iKey = computeKeySize(uData);
		if(iKey<0) return false;
        pushBits(iKey, 5);
        pushBits(uData, iKey+1);
	} while (uCurrentPos < uImageSize);
	return true;
}

bool CrushedBitMap::pixelIsWhite(QImage* pImage, unsigned uCurPos)
{
	// define a black pixel
    //QRgb blackPixel = QColor(0,0,0).rgb();
	int x, y;
	y = uCurPos / pImage->width();
	x = uCurPos - y*pImage->width();
	// Import "mostly black" pixels as black, else white.  This helps clean up .jpg compression artifacts.
	QColor c = pImage->pixel(x,y);
	if(c.red()<32 && c.blue()<32 && c.green()<32) return false;
	
	if(x < mExtents.left()  ) mExtents.setLeft(x); 
	if(x > mExtents.right() ) mExtents.setRight(x);
	if(y > mExtents.bottom()) mExtents.setBottom(y); 
	if(y < mExtents.top()   ) mExtents.setTop(y);

	return true;
}

int CrushedBitMap::computeKeySize(unsigned uData)
{
	for (int iKey=0; iKey<32; iKey++)
		if(uData <= qPow(2.0, iKey))
			return iKey;
	return -1;
}


void CrushedBitMap::pushBits(int iValue, int iBits)
{
	//  Pushes the iBits least significant bits of iValue into the bit array
	//  iBits should always be smaller than 33
	if (mIndex + iBits > mBitarray.size()) mBitarray.resize(mIndex + iBits);
    for (int i=iBits-1; i>=0; i--) {
		mBitarray.setBit(mIndex, iValue & (int)qPow(2.0,i));
		mIndex++;
	}
}

int CrushedBitMap::popBits(int iBits)
{
	//  Pops iBits from the array (up to 32)  returns -1 if iBits>32 or end of array reached
	int iReturn = -1;
	if (mIndex + iBits <= mBitarray.size() && iBits<33){
		iReturn = 0;
		for (int i=iBits-1; i>=0; i--) {
            iReturn = (iReturn<<1) + mBitarray.testBit(mIndex);
            mIndex++;
		}
	}
	return iReturn;
}

bool CrushedBitMap::isWhitePixel(QPoint qPoint){
	// inflate to qPoint, report if white pixel
	if(m_bIsBaseLayer) return false;
	
	bool bCurColorIsWhite = false;
	unsigned uCurrentPos = 0;
	unsigned uImageSize = 0;
	unsigned uData = 0;
	int iKey;

	mIndex = 0; // Reset to start
	m_iWidth = popBits(16);
	m_iHeight = popBits(16);
	bCurColorIsWhite = (popBits(1)==1);
	uImageSize = m_iWidth * m_iHeight;

	iKey = popBits(5);
	uData = popBits(iKey+1);
	int x, y;

	while((uData > 0) && (uCurrentPos < uImageSize)) {
		for(unsigned ui=0; ui<uData; ui++) {

			y = (uCurrentPos / m_iWidth) + m_yOffset;
			x = (uCurrentPos - y*m_iWidth) + m_xOffset;
			if(qPoint == QPoint(x,y)) return bCurColorIsWhite;
			uCurrentPos++;
		}
		// pop length of next run of pixels
		iKey = popBits(5);
		uData = popBits(iKey+1);
		// toggle current color
		if(bCurColorIsWhite) bCurColorIsWhite = false; else	bCurColorIsWhite = true;
	}
	return false;
}

////////////////////////////////////////////////////////////
//
// CrushPrintJob functions
//
///////////////////////////////////////////////////////////

CrushedPrintJob::CrushedPrintJob() {
	clearAll();
}

CrushedBitMap* CrushedPrintJob::getCBMSlice(int i) {
    // i is zero based index, values of 0 to mSlices.size()-1 inclusive
    if(i >= mBase + mSlices.size()) return NULL;
    if(i>=mBase) return &mSlices[i-mBase];  //mSlices[] does not store blank base offset layers, we fake those by always returning the same mBaseLayer CBM
	mBaseLayer.setWidth(m_Width);
	mBaseLayer.setHeight(m_Height);
	mBaseLayer.setIsBaseLayer(true);
	return &mBaseLayer;
}



void CrushedPrintJob::streamOutCPJ(QDataStream* pOut)
{
	int i;
	*pOut << mSlices.size();
	// Loop throuh all slices and save them
	for(i=0; i<mSlices.size();i++)
		mSlices[i].streamOutCMB(pOut);

	*pOut << mSupports.size();
	// Loop throuh all supports and save them
	for(i=0; i<mSupports.size();i++)
		mSupports[i].streamOutSupport(pOut);
}

void CrushedPrintJob::streamInCPJ(QDataStream* pIn)
{
	CrushedBitMap* pCBM;
	mTotalWhitePixels = 0;
	mSlices.clear();
	mSupports.clear();

	int i, iTotal;
	*pIn >> iTotal;

	// reset extents
	mJobExtents.setBottomRight(QPoint(0,0));
	mJobExtents.setTopLeft(QPoint(65535,65535));
	m_Width=0;m_Height=0;

	// Read in all slices.
	for(i=0; i<iTotal;i++){
		pCBM = new CrushedBitMap();
		addCBM(*pCBM);
		mSlices[i].streamInCMB(pIn);
		mTotalWhitePixels += mSlices[i].getWhitePixels();
		QRect tExtent =  mSlices[i].getExtents();
        if(mSlices[i].getHeight()<0 || mSlices[i].getWidth()<0)continue;
		if(mSlices[i].getExtents().left()   < mJobExtents.left()  ) mJobExtents.setLeft(  mSlices[i].getExtents().left()); 
		if(mSlices[i].getExtents().right()  > mJobExtents.right() ) mJobExtents.setRight( mSlices[i].getExtents().right());
		if(mSlices[i].getExtents().bottom() > mJobExtents.bottom()) mJobExtents.setBottom(mSlices[i].getExtents().bottom()); 
		if(mSlices[i].getExtents().top()    < mJobExtents.top()   ) mJobExtents.setTop(   mSlices[i].getExtents().top());
		if(m_Width<mSlices[i].getWidth())m_Width=mSlices[i].getWidth();
		if(m_Height<mSlices[i].getHeight())m_Height=mSlices[i].getHeight();
	}
    // Initialize the generic base layer CBM for use with all "base" layers
	mBaseLayer.setWidth(m_Width);
	mBaseLayer.setHeight(m_Height);
	mBaseLayer.setIsBaseLayer(true);

	// Read in all supports
	*pIn >> iTotal;
	for(i=0; i<iTotal;i++) {
		mSupports.append(SimpleSupport());
		mSupports[i].streamInSupport(pIn);
	}
}

bool CrushedPrintJob::loadCPJ(QFile* pFile)
{
    if(!pFile->open(QIODevice::ReadOnly))
		return false;
	QDataStream in(pFile);
    QString version;
	double xy, z;
    in >> version;
    int iVersion = version.toInt();

    if(iVersion == 1)
    {
	    clearAll();
        mVersion = version;
        in >> mName;
        in >> mDescription;
        in >> xy;
        in >> z;
        in >> mBase;
        in >> mFilled;
        in >> mReserved3;
        in >> mReserved2;
        in >> mReserved1;
	}
	else return false; // unknown version

   // qDebug() << "xy pix" << xy;

	mXYPixel=QString::number(xy); mZLayer=QString::number(z);
	streamInCPJ(&in);
	pFile->close();
	return true;
}

bool CrushedPrintJob::saveCPJ(QFile* pFile)
{
	mVersion = "1";  // Update this and loadCPJ if changed!

	if (!pFile->open(QIODevice::WriteOnly))
		return false;
	QDataStream out(pFile);

	// Current Version "1"
	out << mVersion << mName << mDescription << (qreal)mXYPixel.toDouble() << (qreal)mZLayer.toDouble() << (qint32)mBase << (qint32)mFilled << (QString)"Reserved3"<< (QString)"Reserved2"<< (QString)"Reserved1";

	streamOutCPJ(&out);
	pFile->close();
	return true;
}

uint CrushedPrintJob::getTotalWhitePixels(int iFirst, int iLast)
{
	uint iTotal = 0;
	for(int i=iFirst; i<=iLast; i++)
		if(i>0 && i<mSlices.size())
			iTotal += mSlices[i].getWhitePixels();
	return iTotal;
}

void CrushedPrintJob::clearAll(int iLayers) {
    mTotalWhitePixels=0;
	mBase=0; 
	mFilled=0; 
	mShowSupports=false; 
	m_Width=0; m_Height=0;	
	mJobExtents.setBottomRight(QPoint(0,0));
	mJobExtents.setTopLeft(QPoint(65535,65535));
    DeleteAllSupports();
	mSlices.clear();

    if(iLayers == 0) return;

    CrushedBitMap *pCBM;
    for(int i=0; i<iLayers; i++){
        pCBM = new CrushedBitMap();
        mSlices.append(*pCBM);
    }
}
void CrushedPrintJob::inflateCurrentSlice(QImage* pImage, int xOffset, int yOffset, bool bUseNaturalSize)
{
    inflateSlice(m_CurrentSlice, pImage, xOffset, yOffset, bUseNaturalSize);
}

void CrushedPrintJob::inflateSlice(int sliceIndx, QImage* pImage, int xOffset, int yOffset, bool bUseNaturalSize) {
	float WinWidth;
	float WinHeight;
	float SliceWidth;
	float SliceHeight;
	int widthOff;
	int heightOff;
	SimpleSupport sSimple;

    if(sliceIndx < 0 || sliceIndx >= getTotalLayers()) return;

    getCBMSlice(sliceIndx)->inflateSlice(pImage, xOffset, yOffset, bUseNaturalSize);

	//Todo Render filled Extent && Supports
	if(mShowSupports){
		WinWidth = pImage->width()/2.0;
		WinHeight= pImage->height()/2.0;
		SliceWidth = m_Width/2.0;
		SliceHeight = m_Height/2.0;
		widthOff = (int)(WinWidth - SliceWidth);
		heightOff = (int)(WinHeight - SliceHeight);
		xOffset = widthOff  + xOffset;
		yOffset = heightOff + yOffset;

		QRect rOffset = mJobExtents;
		rOffset.moveCenter(rOffset.center() + QPoint(xOffset, yOffset));

        if(sliceIndx < mFilled){
			// Render extents if filled layer
			QPainter tPainter(pImage);
			tPainter.setPen(QColor(255,255,255));
			tPainter.setBrush(QBrush(QColor(255,255,255)));
			tPainter.setBackgroundMode(Qt::OpaqueMode);
			tPainter.fillRect(rOffset,Qt::white);
		}
		else {
			// Render Supports
			//Loop through supports list, if current slice has support, draw it
			for(int i=0; i<mSupports.size(); i++){
                if((sliceIndx<mBase && mSupports[i].getStart()==0)||(sliceIndx >= mSupports[i].getStart() + mBase && sliceIndx <= mSupports[i].getEnd() + mBase)){
					sSimple = mSupports[i];
					sSimple.setPoint(sSimple.getPoint()+QPoint(xOffset, yOffset));
					sSimple.draw(pImage);
				}
			}
		}
	}	
}

bool CrushedPrintJob::addImage(QImage* pImage){
	CrushedBitMap CBM;
	if(!CBM.crushSlice(pImage))return false;
	
	// Update Extents
	if(CBM.getExtents().left()   < mJobExtents.left()  ) mJobExtents.setLeft(  CBM.getExtents().left()); 		
	if(CBM.getExtents().right()  > mJobExtents.right() ) mJobExtents.setRight( CBM.getExtents().right());
	if(CBM.getExtents().bottom() > mJobExtents.bottom()) mJobExtents.setBottom(CBM.getExtents().bottom()); 
	if(CBM.getExtents().top()    < mJobExtents.top()   ) mJobExtents.setTop(   CBM.getExtents().top());
	
	// Update width and height
	if(m_Width<CBM.getWidth())m_Width=CBM.getWidth();
	if(m_Height<CBM.getHeight())m_Height=CBM.getHeight();
	addCBM(CBM);
	return true;
}
bool CrushedPrintJob::crushCurrentSlice(QImage* pImage)
{
    return crushSlice(m_CurrentSlice, pImage);
}

bool CrushedPrintJob::crushSlice(int sliceIndx, QImage* pImage){
    // Crushes the pImage and stores at m_CurrentSlice.  Adjusts the job's width and height if needed
    if(getCBMSlice(sliceIndx)==NULL)
        return false;

    bool bResult=getCBMSlice(sliceIndx)->crushSlice(pImage);

    if(m_Width<getCBMSlice(sliceIndx)->getWidth())
        m_Width=getCBMSlice(sliceIndx)->getWidth();

    if(m_Height<getCBMSlice(sliceIndx)->getHeight())
        m_Height=getCBMSlice(sliceIndx)->getHeight();

    return bResult;
}

bool CrushedPrintJob::isWhitePixel(QPoint qPoint, int iSlice){
	if(iSlice<0) iSlice = m_CurrentSlice;
	if(iSlice<0 || iSlice> getTotalLayers()) return false;
	CrushedBitMap* pCBM = getCBMSlice(iSlice);
	return pCBM->isWhitePixel(qPoint);
}

void CrushedPrintJob::AddSupport(int iEndSlice, QPoint qCenter, int iSize, SupportType eType, int fastmode){
	SimpleSupport support(qCenter, eType, iSize, 0-mBase, iEndSlice-mBase);
	if(fastmode)
	{
		//Scan from iEndSlice down to 0, if we hit a white pixel set start to that layer
		for(int i = iEndSlice; i>=0; i--)
		{
			if(isWhitePixel(qCenter,i)){
				support.setStart(i-mBase);
				break;
			}
		}
	}
	mSupports.append(support);
}

bool CrushedPrintJob::DeleteSupport(int iSlice, QPoint qCenter, int iRadius){
	float dist;
	float mindist = 1000.0;
	int delindx;
	iSlice = iSlice - mBase;
	//Loop through all supports, if valid for this slice and share same qCenter, remove it
	for(int i = 0; i<mSupports.size(); i++){
		if(iSlice >= mSupports[i].getStart() && iSlice <= mSupports[i].getEnd() || iSlice -1 >= mSupports[i].getStart() && iSlice -1 <= mSupports[i].getEnd())
		{
			dist = qSqrt(qPow((qCenter.y() - mSupports[i].getPoint().y()),2)+qPow((qCenter.x() - mSupports[i].getPoint().x()),2));
			if(dist < mindist)
			{
				mindist = dist;
				delindx = i;
			}
		}
	}
	if(mindist<=iRadius)
	{
		mSupports.removeAt(delindx);
		return true;
	}
	return false;
}

