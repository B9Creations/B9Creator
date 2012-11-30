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

#include "slicecontext.h"
#include <QtOpenGL>
#include "OS_GL_Wrapper.h"
#include "projectdata.h"
#include <QPoint>
#include <QVector2D>

SliceContext::SliceContext(QWidget *parent, B9Layout* pmain) : QGLWidget(parent)
{
	pMain = pmain;
	pSlice = NULL;
	debugmode = false;
	shownormals = true;
	connections = true;
	fills = true;
	outlines = true;
	zoom = 1.0;

	mousedown = false;
}
SliceContext::~SliceContext()
{
	pMain = NULL;
	pSlice = NULL;
}




void SliceContext::SetSlice(Slice* slice)
{
	pSlice = slice;
}



void SliceContext::initializeGL()
{
	qglClearColor(QColor(0,0,0));
	glEnable(GL_BLEND);

	glViewport(0, 0, pMain->project->GetResolution().x(), pMain->project->GetResolution().y());
    glMatrixMode(GL_PROJECTION);
	gluOrtho2D(-pMain->project->GetBuildSpace().x()/2.0,pMain->project->GetBuildSpace().x()/2.0,-pMain->project->GetBuildSpace().y()/2.0,pMain->project->GetBuildSpace().y()/2.0);
	glMatrixMode(GL_MODELVIEW);
}

void SliceContext::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();//reset matrix operations
	
	if(pSlice)
	{

		if(debugmode)
		{
			glScalef(zoom,zoom,zoom);
				glTranslatef(pan.x(),pan.y(),0);
				pSlice->DebugRender(shownormals,connections,fills,outlines);
				glTranslatef(-pan.x(),-pan.y(),0);
			glScalef(1.0,1.0,1.0);
		}
		else
		{
			pSlice->Render();
		}
	}
}



//events
void SliceContext::mousePressEvent(QMouseEvent *event)
{ 
	lastPos = event->pos();
	 if(event->button() == Qt::LeftButton)
	 {
		
		 mousedown = true;
	 }

}
void SliceContext::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
	 {
		 mousedown = false;
	 }

}
void SliceContext::mouseMoveEvent(QMouseEvent *event)
{ 
	int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

	if(mousedown)
	{
		pan += QVector2D(-double(dx)/100.0,double(dy)/100.0);
		updateGL();
	}
}
