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
