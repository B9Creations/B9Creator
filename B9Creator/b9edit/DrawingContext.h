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
