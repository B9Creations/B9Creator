#ifndef SLICECONTEXT_H
#define SLICECONTEXT_H

#include <QGLWidget>
#include "b9layout.h"
#include "slice.h"
class SliceData;
class SliceContext : public QGLWidget
{
	Q_OBJECT

public:
    SliceContext(QWidget *parent, B9Layout* pmain);
     ~SliceContext();

	void SetSlice(Slice* slice);
	bool debugmode;
	bool shownormals;
	bool connections;
	bool fills;
	bool outlines;

	double zoom;
private:
	void initializeGL();
    void paintGL();
	
	
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
	
	
	
    B9Layout* pMain;
	Slice* pSlice;

	bool mousedown;
	QPoint lastPos;
	QVector2D pan;
	
};

#endif
