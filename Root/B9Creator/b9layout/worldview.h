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
#include "b9supportstructure.h"
#include <QPoint>
class Slice;
class B9ModelInstance;
class B9SupportStructure;
class MainWindow;
class B9Layout;
class Triangle3D;
class WorldView : public QGLWidget
{
    Q_OBJECT
 public:
     WorldView(QWidget *parent, B9Layout* main);
     ~WorldView();
     QTimer* pDrawTimer; //refreshes the 3d scene

     bool shiftdown; //public so that the mainwindow can alter these values easy.
     bool controldown;
 public slots:


	void CenterView();
    void TopView();
    void RightView();
    void FrontView();
    void BackView();
    void LeftView();
    void BottomView();
    void SetPerpective(bool persp){perspective = persp;}
    void SetRevolvePoint(QVector3D point);
    void SetPan(QVector3D pan);
    void SetZoom(double zoom);
    void setXRotation(float angle);
    void setYRotation(float angle);
    void setZRotation(float angle);


    void UpdatePlasmaFence();//check if any instances are out of the build area and show fence.

    void UpdateVisSlice();

    void UpdateTick();//called every 1/60th of a second by update timer. also refreshes the openGL Screen
	void SetTool(QString tool);
    QString GetTool();
    void ExitToolAction();//stops the using of tool while the mouse is moving.

 public://GETS

    QVector3D GetPan();
    float GetZoom();
    QVector3D GetRotation();

public: //Events
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent * event );
    void keyReleaseEvent(QKeyEvent * event );

    //tools code
    void OnToolInitialAction(QString tool, QMouseEvent* event);
    void OnToolDragAction(QString tool, QMouseEvent* event);
    void OnToolReleaseAction(QString tool, QMouseEvent* event);
    void OnToolHoverMove(QString tool, QMouseEvent* event);
    QString toolStringMemory;
    QString toolStringMemory2;
    QVector3D toolVectorMemory1;
    QVector3D toolVectorMemory2;
    QVector3D toolVectorOffsetMemory;
    double toolDoubleMemory1;
    double toolDoubleMemory2;
    double toolDoubleMemory3;
    B9SupportStructure toolSupportMemory;
    bool toolBoolMemory1;


 signals:

    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

 private://functions
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);


    void ResetCamera(bool xrayon = false);
    void DrawMisc();//draw stuff...
	void DrawInstances(); //draws all instances!
    void DrawVisualSlice();//draws the visual slice indicator.
	void DrawBuildArea();//draws the bounds of the build area.
	

    B9ModelInstance* SelectInstanceByScreen(QPoint pos);// quarrys the screen for an object at the pos,
    B9SupportStructure* GetSupportByScreen(QPoint pos);// returns pointer to object.
    QString GetSupportSectionByScreen(QPoint pos, B9SupportStructure* sup);

    void Update3DCursor(QPoint pos);
    void Update3DCursor(QPoint pos, B9ModelInstance *trackInst);
    void Update3DCursor(QPoint pos, B9ModelInstance *trackInst, bool &isAgainstInst);
    QVector3D Get3DCursorOnScreen(QPoint pos, QVector3D hintPos, double standoff);

    unsigned int GetPickTriangleIndx(B9ModelInstance *inst, QPoint pos, bool &success);//returns the triangle of any model that the user clicks on. success is returned.
    QVector3D GetPickOnTriangle(QPoint pos, B9ModelInstance *inst, unsigned int triIndx);
    QVector3D GetDrillingHit(QVector3D localBeginPosition, B9ModelInstance *inst, bool &hitground, Triangle3D *&pTri);




    void UpdateSelectedBounds();


 private: //members
    //viewing vars
    float xRot;
    float yRot;
    float zRot;
    float xRotTarget;//target values for smooth transitions.
    float yRotTarget;
    float zRotTarget;

    float camdist;
    float camdistTarget;

    QString currViewAngle;//"FREE", "TOP", "FRONT", etc

    bool perspective;
    bool hideNonActiveSupports;

    QVector3D pan;
    QVector3D panTarget;
    QVector3D revolvePoint;
    QVector3D revolvePointTarget;

    float deltaTime;//frame rate and delta time vars
    float lastFrameTime;


    QVector3D cursorPos3D;//true global position of cursor
    QVector3D cursorNormal3D;//
    QVector2D cursorPosOnTrackCanvas;//similar to pixel cords, but in real units in world.
    QVector3D cursorPreDragPos3D;
    QVector3D cursorPostDragPos3D;//TODO not yet implemented
    QVector3D PreDragInstanceOffset;
    QVector3D PreDragRotationOffsetData;//not really a cordinate just info.

	//tools/keys
	QString currtool;
	bool pandown;
	bool dragdown;

    //visual build size only - use project settings as actual size
	float buildsizex;
	float buildsizey;
	float buildsizez;

    //visual fence stuff
    bool fencesOn[5];
    float fenceAlpha;
    float supportAlpha;

    //mouse cords
    QVector2D mouseDeltaPos;
    QPoint mouseDownInitialPos;
    QPoint mouseLastPos;

    //visual slice following
    Slice* pVisSlice;

    //layout window pointer.
    B9Layout* pMain;
 };

 #endif
