#ifndef SLICEDEBUGGER_H
#define SLICEDEBUGGER_H
#include "ui_slicedebugwindow.h"
class SliceContext;
class B9Layout;
class SliceDebugger : public QWidget
{
	Q_OBJECT
public:
    SliceDebugger(B9Layout* pmain, QWidget *parent = 0, Qt::WFlags flags = 0);
	~SliceDebugger();
	
	Ui::Form ui;
public slots:
	void RefreshOptions();
	void GoToSlice(QString str);
	void NextSlice();
	void PrevSlice();
	void BakeTests();
private:
	void ShowSlice(int slice);
	SliceContext* ppaintwidget;
    B9Layout* pMain;
	
	
	void wheelEvent(QWheelEvent *event);
	
	
	int currslice;




};
#endif
