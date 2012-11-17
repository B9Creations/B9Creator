#include <QtGui>
#include <QtOpenGL>
#include "OS_GL_Wrapper.h"
#include "math.h"
#include "worldview.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif


WorldView::WorldView(QWidget *parent, B9Layout* main) : QGLWidget(parent)
{
	pMain = main;

	xRot = 0.0;
	yRot = 0.0;
    zRot = 0.0;
	pan = QVector3D(0,0,0);//set pan to center of build area.
	
	camdist = 350;

	//tools/keys
	currtool = "pointer";
	shiftdown = false;
	dragdown = false;
	pandown = 0;
	selectedinst = NULL;

	buildsizex = pMain->project->GetBuildSpace().x();
	buildsizey = pMain->project->GetBuildSpace().y();
	buildsizez = pMain->project->GetBuildSpace().z();

	pDrawTimer = new QTimer();
	connect(pDrawTimer, SIGNAL(timeout()), this, SLOT(UpdateTick()));
    pDrawTimer->start(0);

	setFocusPolicy(Qt::ClickFocus);
}

WorldView::~WorldView()
{
	delete pDrawTimer;
}

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360;
    while (angle > 360 )
        angle -= 360;
}


///////////////////////////////////////
//Public Slots
///////////////////////////////////////
void WorldView::setXRotation(int angle)
{
    qNormalizeAngle(angle);

	if(angle < 180)
	{
		return;
	}

    if (angle != xRot)
	{
		
        xRot = angle;
        emit xRotationChanged(angle);
    }
}
void WorldView::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        
    }
}
void WorldView::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
    }
}
void WorldView::CenterView()
{
	pan = QVector3D(0,0,0);//set pan to center of build area.
	camdist = 350;
}
void WorldView::UpdateTick()
{
	buildsizex += (pMain->project->GetBuildSpace().x() - buildsizex)/2;
	buildsizey += (pMain->project->GetBuildSpace().y() - buildsizey)/2;
	buildsizez += (pMain->project->GetBuildSpace().z() - buildsizez)/2;

	glDraw();
}
void WorldView::SetTool(QString tool)
{
	currtool = tool;
}

//Private
void WorldView::initializeGL()
{

    qglClearColor(QColor(0,75,75));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    glLineWidth(1.2);
    glEnable(GL_LINE_SMOOTH);
    glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable ( GL_COLOR_MATERIAL );
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    static GLfloat lightPosition[4] = { 0.0, 0.0, 100, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

}

void WorldView::paintGL()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();//restores default matrix.
	
	glTranslatef(0.0, 0.0, -camdist);//step back amount
	glTranslatef(pan.x(),-pan.y(),0);
	
	glRotatef(xRot, 1.0, 0.0, 0.0);
	glRotatef(yRot, 0.0, 1.0, 0.0);
	glRotatef(zRot, 0.0, 0.0, 1.0);
	
	
    DrawInstances();
	
	//draw the BuildArea
    DrawBuildArea();

}

void WorldView::resizeGL(int width, int height)
 {

	 glViewport(0,0,width,height);
     
	 glMatrixMode(GL_PROJECTION);
     glLoadIdentity();

     gluPerspective(30,double(width)/height,1,5500);
     glMatrixMode(GL_MODELVIEW);

 }
void WorldView::DrawInstances()
{
    unsigned int m;
    unsigned int i;

	glEnable(GL_DEPTH_TEST);

	for(m=0;m<pMain->ModelDataList.size();m++)
	{
		for(i=0;i<pMain->ModelDataList[m]->instList.size();i++)
		{
			pMain->ModelDataList[m]->instList[i]->RenderGL();
		}
	}
}
void WorldView::DrawBuildArea()
 {
	glPushMatrix();
	glColor3f(1,1,1);
	glTranslatef(-buildsizex/2, -buildsizey/2, 0);

    glDisable(GL_LIGHTING);
		//4 vertical lines
		glBegin(GL_LINES); 
			glVertex3d( 0, 0, 0);
			glVertex3d( 0, 0, buildsizez); 
		glEnd();
		glBegin(GL_LINES); 
			glVertex3d( buildsizex, 0,0);
			glVertex3d( buildsizex, 0,buildsizez); 
		glEnd();
		glBegin(GL_LINES); 
			glVertex3d( 0, buildsizey,0);
			glVertex3d( 0, buildsizey,buildsizez); 
		glEnd();
		glBegin(GL_LINES); 
			glVertex3d( buildsizex, buildsizey,0);
			glVertex3d( buildsizex, buildsizey,buildsizez); 
		glEnd();

		//4 Top lines
		glBegin(GL_LINES); 
			glVertex3d( 0, 0, buildsizez);
			glVertex3d( buildsizex, 0, buildsizez); 
		glEnd();
		glBegin(GL_LINES); 
			glVertex3d( 0, 0, buildsizez);
			glVertex3d( 0, buildsizey, buildsizez); 
		glEnd();
		glBegin(GL_LINES); 
			glVertex3d( buildsizex, buildsizey, buildsizez);
			glVertex3d( 0, buildsizey, buildsizez); 
		glEnd();
		glBegin(GL_LINES); 
			glVertex3d( buildsizex, buildsizey, buildsizez);
			glVertex3d( buildsizex, 0, buildsizez); 
		glEnd();
		//4 Bottom lines
		glBegin(GL_LINES); 
			glVertex3d( 0, 0, 0);
			glVertex3d( buildsizex, 0,0); 
		glEnd();
		glBegin(GL_LINES); 
			glVertex3d( 0, 0, 0);
			glVertex3d( 0, buildsizey, 0); 
		glEnd();
		glBegin(GL_LINES); 
			glVertex3d( buildsizex, buildsizey, 0);
			glVertex3d( 0, buildsizey, 0); 
		glEnd();
		glBegin(GL_LINES); 
			glVertex3d( buildsizex, buildsizey, 0);
			glVertex3d( buildsizex, 0, 0); 
		glEnd();
		
		
		//draw cordinate hints
		
        glColor3f(1.0f,0.0f,0.0f);
		glBegin(GL_LINES); 
				glVertex3d( 0, 0, 0);
				glVertex3d( 0, 0, buildsizez + 10); 
		glEnd();
		glBegin(GL_LINES); 
				glVertex3d( 0, 0, 0);
				glVertex3d( buildsizex + 10, 0, 0); 
		glEnd();
		glBegin(GL_LINES); 
				glVertex3d( 0, 0, 0);
				glVertex3d( 0, buildsizey + 10, 0); 
		glEnd();
        glColor3f(1.0f,1.0f,1.0f);

        //TODO _RESULTS IN CRASH ON SOME INTEL DRIVERS....
        //renderText( buildsizex + 10,0,0, "+X");
        //renderText( 0,buildsizey + 10,0, "+Y");
        //renderText( 0,0,buildsizez + 10, "+Z");

        glEnable(GL_LIGHTING);
		//top rectangle
        glColor4f(0.0f,0.0f,1.0f,1.0f);
		glNormal3f(0,0,1);
			glBegin(GL_TRIANGLES);                    
				glVertex3f( buildsizex, 0, 0);     
				glVertex3f( buildsizex, buildsizey, 0);     
				glVertex3f( 0, buildsizey, 0);     
			glEnd();
		
			glBegin(GL_TRIANGLES);                    
				glVertex3f( buildsizex, 0, 0);     
				glVertex3f( 0, buildsizey, 0);     
				glVertex3f( 0, 0, 0);     
			glEnd();

		
		//bottom rectangle
		glNormal3f(0,0,-1);
        glColor3f(1.0f,0.7f,0.3f);
			glBegin(GL_TRIANGLES);
				glVertex3f( 0, buildsizey, 0);      
				glVertex3f( buildsizex, buildsizey, 0);                     
				glVertex3f( buildsizex, 0, 0);        
			glEnd();
		
			glBegin(GL_TRIANGLES);                    
				glVertex3f( 0, 0, 0);
				glVertex3f( 0, buildsizey, 0); 
				glVertex3f( buildsizex, 0, 0);          
			glEnd();
        glColor4f(1.0f,1.0f,1.0f,1.0f);


	glPopMatrix();
 }

 ModelInstance* WorldView::SelectByScreen(QPoint pos, bool singleselect)
 {
    unsigned int m;
    unsigned int i;
	unsigned char pixel[3];
	//clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	for(m=0;m<pMain->ModelDataList.size();m++)
	{
		for(i=0;i<pMain->ModelDataList[m]->instList.size();i++)
		{
			pMain->ModelDataList[m]->instList[i]->RenderPickGL();
		}
	}
    glReadPixels(pos.x(), this->height() - pos.y(), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glEnable(GL_LIGHTING);
	paintGL();
	

	qDebug() << pixel[0] << pixel[1] << pixel[2];
	//now that we have the color, compare it against every instance
	
	if(singleselect)
		pMain->DeSelectAll();//if shift is off deselect all.
	
	
	for(m=0;m<pMain->ModelDataList.size();m++)
	{
		for(i=0;i<pMain->ModelDataList[m]->instList.size();i++)
		{
			ModelInstance* inst = pMain->ModelDataList[m]->instList[i];
			qDebug() << inst->pickcolor[0] << inst->pickcolor[1] << inst->pickcolor[2];


			if((pixel[0] == inst->pickcolor[0]) && (pixel[1] == inst->pickcolor[1]) && (pixel[2] == inst->pickcolor[2]))
			{
				
				pMain->Select(inst);
				return inst;
			}
		}
	}
	
	return NULL;
 }
 void WorldView::UpdateSelectedBounds()
 {
     unsigned int i;
     pMain->setCursor(Qt::WaitCursor);

	 for(i=0;i<pMain->GetSelectedInstances().size();i++)
	 {
		 pMain->GetSelectedInstances()[i]->UpdateBounds();
	 }
     pMain->setCursor(Qt::ArrowCursor);
 }

 //Mouse Interaction
void WorldView::mousePressEvent(QMouseEvent *event)
{
     lastPos = event->pos();
	 if(event->button() == Qt::MiddleButton)
	 {
		pandown = true;
	 }
	 if(event->button() == Qt::LeftButton)
	 {
		 dragdown = true;
		 if(currtool == "pointer")
		 {
			SelectByScreen(event->pos(),!shiftdown); // start the proccess of picking objects in the screen;
		 }
		 else//if we are on some other tool, only be able to select it if nothing else is selected
		 {
			 if(!pMain->GetSelectedInstances().size())
			 {
				 SelectByScreen(event->pos(),!shiftdown);
			 }
		 }
	 }
}

void WorldView::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::MiddleButton)
	{
		pandown = false;
	}
	if(event->button() == Qt::LeftButton)
	{
		dragdown = false;
		if(currtool == "rotate")
		{
			UpdateSelectedBounds();
		}
	}
}

void WorldView::mouseMoveEvent(QMouseEvent *event)
 {
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();
	double x;
	double y;


	if (event->buttons() & Qt::RightButton) {
		setXRotation(xRot + 0.5 * dy);
		setZRotation(zRot + 0.5 * dx);
	}
	lastPos = event->pos();

	
	if(pandown)
	{
		pan += QVector3D(dx/5.0,dy/5.0,0);
	}
	 
	if(currtool == "move")
	{
		if(dragdown)
		{
            for(unsigned int i = 0; i<pMain->GetSelectedInstances().size(); i++)
			{
				x = qSin(zRot*0.017453)*-dy + qSin((zRot+90)*0.017453)*dx;
				y = qCos(zRot*0.017453)*-dy + qCos((zRot+90)*0.017453)*dx;

				if(shiftdown)
				{
					
					pMain->GetSelectedInstances()[i]->Move(QVector3D(0,0,-dy*0.1));
				}
				else
				{
					pMain->GetSelectedInstances()[i]->Move(QVector3D(x*0.2,y*0.1,0));
				}
			}
		}
		//show the results of the movement in real time
		pMain->UpdateTranslationInterface();
	}
	if(currtool == "rotate")
	{
		if(dragdown)
		{
			if(pMain->GetSelectedInstances().size() == 1)
			{
                for(unsigned int i = 0; i<pMain->GetSelectedInstances().size(); i++)
				{
					if(shiftdown)
					{
						pMain->GetSelectedInstances()[i]->Rotate(QVector3D(0,0,-dx*0.2));
					}
					else
					{
						pMain->GetSelectedInstances()[i]->Rotate(QVector3D(dy*0.2,dx*0.2,0));
					}	
				}
			}
		}
		//show the results of the movement in real time
		pMain->UpdateTranslationInterface();
	}
 }

void WorldView::wheelEvent(QWheelEvent *event)
{
	camdist -= event->delta()/4.0;
	if(camdist <= 50.0)
	{
		camdist = 50.0;
	}
}

 //Keyboard interaction
void WorldView::keyPressEvent( QKeyEvent * event )
{
	QWidget::keyPressEvent(event);
	if(event->key() == Qt::Key_Shift)
	{
		shiftdown = true;
	}
	if(event->key() == Qt::Key_J)
	{
        for(unsigned int i = 0; i<pMain->GetSelectedInstances().size(); i++)
		{
			pMain->GetSelectedInstances()[i]->UpdateBounds();


		}
	}

}
void WorldView::keyReleaseEvent( QKeyEvent * event )
{
	if(event->key() == Qt::Key_Shift)
	{
		shiftdown = false;
	}
}
