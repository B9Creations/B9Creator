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

#ifndef DRAWINGCONTEXT_H
#define DRAWINGCONTEXT_H
#include <QtGui>
#include <QWidget>
#include <QImage>

class SliceEditView;
class DrawingContext : public QWidget
{
	Q_OBJECT
public:
	DrawingContext(QWidget *parent = 0);
	~DrawingContext();

	SliceEditView* pEditView;
	QImage* pActiveImage;
	QImage* pLowerImage;

	QString currDrawTool;//string indicating the current selected draw tool.
	QString currSupportTool;//string indicating the current selected Support tool.

	int PenWidth;
	int supportSize;
	bool deletemode;
	bool fastsupportmode;
	void SetPenColor(QColor color);
	void SetPenWidth(int w);

public slots:
	void SetUpperImg(QImage* img); //displays the passed image to the screen.
	void SetLowerImg(QImage* img); //sets the image to be displayed below the upper..
	void GenerateLogicImage();
	void GenerateGreenImage();

protected:
	 void mousePressEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);
     void mouseReleaseEvent(QMouseEvent *event);
     void paintEvent(QPaintEvent *event);

private:
	void drawLineTo(const QPoint &endPoint);

	
	bool scribbling; //true when the mouse is down and is scribbling
	
    QColor PenColor;
	QPoint lastPoint;
};
#endif
