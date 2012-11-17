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
	bool shiftdown;
	bool pandown;
	bool dragdown;
	ModelInstance* selectedinst;

	//visual
	float buildsizex;
	float buildsizey;
	float buildsizez;

	QPoint lastPos;


    B9Layout* pMain;
 };

 #endif
