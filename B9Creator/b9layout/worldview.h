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

#ifndef WORLDVIEW_H
#define WORLDVIEW_H

#include <QGLWidget>
#include "b9layout.h"
#include <QPoint>
class ModelInstance;
class MainWindow;
class B9Layout;
class WorldView : public QGLWidget
{
    Q_OBJECT
 public:
     WorldView(QWidget *parent, B9Layout* main);
     ~WorldView();
     QTimer* pDrawTimer; //refreshed the 3d scene

     bool shiftdown; //public so that the mainwindow can alter these values easy.
     bool controldown;
 public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
	void CenterView();
	void UpdateTick();//called every 1/60th of a second by update timer. also refreshes the openGL Screen
	void SetTool(QString tool);
 signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

 private:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);


	void DrawInstances(); //draws all instances!
	void DrawBuildArea();//draws the bounds of the build area.
	

	ModelInstance* SelectByScreen(QPoint pos,bool singleselect = true);// quarrys the screen for an object at the pos, then gives pMain the go ahead to select it etc..
	void UpdateSelectedBounds();

    void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void keyPressEvent(QKeyEvent * event );
	void keyReleaseEvent(QKeyEvent * event );


 private:
    float xRot;
    float yRot;
    float zRot;
	QVector3D pan;
	

	float camdist;
	//tools/keys
	QString currtool;

	bool pandown;
	bool dragdown;
	ModelInstance* selectedinst;

	//visual
	float buildsizex;
	float buildsizey;
	float buildsizez;

    QPoint mousedownPos;
    bool initialsnap;
	QPoint lastPos;

    B9Layout* pMain;
 };

 #endif
