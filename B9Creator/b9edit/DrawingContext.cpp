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

#include "DrawingContext.h"
#include "SliceEditView.h"
#include "floodfill.h"

//////////////////////////////////////////
//Public
/////////////////////////////////////////
DrawingContext::DrawingContext(QWidget *parent) : QWidget(parent)
{
	pActiveImage = NULL;
	scribbling = false;
	PenWidth = 6;
	supportSize = 32;
	fastsupportmode = true;
	deletemode = false;
	PenColor = Qt::white;


}
DrawingContext::~DrawingContext()
{
}

void DrawingContext::SetPenColor(QColor color)
{
	PenColor = color;
}
void DrawingContext::SetPenWidth(int w)
{
	PenWidth = w;
}


void DrawingContext::SetUpperImg(QImage* img)
{
	setMinimumSize(img->size());
	pActiveImage = img;
	update();
}
void DrawingContext::SetLowerImg(QImage* img)
{
	pLowerImage = img;
	update();
}

void DrawingContext::GenerateLogicImage()
{
	if(pActiveImage == NULL || pLowerImage == NULL)
		return;

	int x;
	int y;
    int white = QColor(255,255,255).rgb();
    int grey = QColor(190,190,190).rgb();
    int black = QColor(0,0,0).rgb();
    int red = QColor(255,0,0).rgb();
	int width = pActiveImage->width();
	int height = pActiveImage->height();
	
	
	
	for(x=0;x<width;x++)
	{
		for(y=0;y<height;y++)
		{
			if(pActiveImage->pixel(x,y) == white || pActiveImage->pixel(x,y) == red || pActiveImage->pixel(x,y) == grey)
			{		
				if(pLowerImage->pixel(x,y) == black)
				{
					pActiveImage->setPixel(x,y,red);	
				}
				else
				{
					pActiveImage->setPixel(x,y,grey);
				}
			}
		}
	}
}
void DrawingContext::GenerateGreenImage()
{
	if(pActiveImage == NULL || pLowerImage == NULL)
		return;
	int x;
	int y;
    int white = QColor(255,255,255).rgb();
    int black = QColor(0,0,0).rgb();
    int green = QColor(0,100,0).rgb();
	int width = pActiveImage->width();
	int height = pActiveImage->height();

	for(x=0;x<width;x++)
	{
		for(y=0;y<height;y++)
		{
			if(pActiveImage->pixel(x,y) == black)
			{		
				if(pLowerImage->pixel(x,y) == white)
				{
					pActiveImage->setPixel(x,y,green);	
				}
			}
		}
	}
}
///////////////////////////////////////
//Private
///////////////////////////////////
void DrawingContext::drawLineTo(const QPoint &endPoint)
 {
     QPainter painter(pActiveImage);
     painter.setPen(QPen(PenColor, PenWidth, Qt::SolidLine, Qt::RoundCap,
                         Qt::RoundJoin));
     painter.drawLine(lastPoint, endPoint);
     pEditView->modified = true;

     int rad = (PenWidth / 2) + 2;
     update(QRect(lastPoint, endPoint).normalized()
                                      .adjusted(-rad, -rad, +rad, +rad));
     lastPoint = endPoint;
 }

///////////////////////////////////////
//events
///////////////////////////////////////
void DrawingContext::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect dirtyRect = event->rect();

	if(pActiveImage)
	{
		painter.drawImage(dirtyRect, *pActiveImage, dirtyRect);
	}
}

void DrawingContext::mousePressEvent(QMouseEvent *event)
 {
     if (event->button() == Qt::LeftButton) {
		 if(pActiveImage)
		 {
			 if(pEditView->currSlice < pEditView->pCPJ->getBase())
			 {
				 QMessageBox::StandardButton ret;
					ret = QMessageBox::warning(this, tr("Slice Manager"),
					tr("Can't Edit Base Slices"), QMessageBox::Ok);
					return;
			 }
			 if(pEditView->supportMode)
			 {
				if(deletemode)
				{
					//delete supports
					if(pEditView->pCPJ->DeleteSupport(pEditView->currSlice, event->pos(), 16))
					{
						pEditView->DeCompressIntoContext();
						GenerateLogicImage();
						pEditView->RefreshWithGreen();
					}

				}
				else
				{
					if(currSupportTool == "circle")
					{
						pEditView->AddSupport(event->pos(),supportSize,st_CIRCLE,fastsupportmode);
					}
					else if(currSupportTool == "square")
					{
						pEditView->AddSupport(event->pos(),supportSize,st_SQUARE,fastsupportmode);
					}
					else if(currSupportTool == "triangle")
					{
						pEditView->AddSupport(event->pos(),supportSize,st_TRIANGLE,fastsupportmode);
					}
					else if(currSupportTool == "diamond")
					{
						pEditView->AddSupport(event->pos(),supportSize,st_DIAMOND,fastsupportmode);
					}
				}
			 }
			 else
			 {
				 scribbling = true;

				 if(currDrawTool == "penfill" || currDrawTool == "penvoid")
				 {
					 lastPoint = event->pos();
				 }
				 else if(currDrawTool == "floodfill" || currDrawTool == "floodvoid")
				 {
					floodFill(pActiveImage, event->pos().x(), event->pos().y(), PenColor);
					pEditView->modified = true;
				 }
			 }
			 
		 }
     }
 }

 void DrawingContext::mouseMoveEvent(QMouseEvent *event)
 {
     if(event->buttons() & Qt::LeftButton) 
	 {
		if(!pEditView->supportMode)
		{
			if(scribbling && (currDrawTool == "penfill" || currDrawTool == "penvoid"))
			{
				drawLineTo(event->pos());
			}	 
		}
	 }
	
 }

 void DrawingContext::mouseReleaseEvent(QMouseEvent *event)
 {
     if (event->button() == Qt::LeftButton) {
		 if(pActiveImage && !pEditView->supportMode)
		 {
			 if(scribbling && (currDrawTool == "penfill" ||currDrawTool == "penvoid"))
			 { 
			 	drawLineTo(event->pos());
			 }	 
			 scribbling = false;
			 pEditView->RefreshContext(1);
			 pEditView->SaveToUndoBuffer();//since the image is altered, save to the buffer.
			 pEditView->ReCompress();
		 }
     }
 }
