#include "slicedebugger.h"
#include "slicecontext.h"
SliceDebugger::SliceDebugger(B9Layout* pmain, QWidget *parent, Qt::WFlags flags) : QWidget(parent, flags)
{
	ui.setupUi(this);
	pMain = pmain;

	ppaintwidget = new SliceContext(NULL, pMain);
	ppaintwidget->debugmode = true;
	ui.debuglayout->addWidget(ppaintwidget);
	
	currslice = 1;
	GoToSlice(QString().number(currslice));
}

SliceDebugger::~SliceDebugger()
{
	//Unbake selected instances
    for(unsigned int i = 0; i < pMain->GetSelectedInstances().size(); i++)
	{
		pMain->GetSelectedInstances()[i]->UnBakeGeometry();
	}
	ppaintwidget->debugmode = false;
	delete ppaintwidget;
}
////////////////////////////////////////////////
//Public Slots
////////////////////////////////////////////////
void SliceDebugger::GoToSlice(QString str)
{
	currslice = str.toInt();
	ShowSlice(currslice);
}
void SliceDebugger::NextSlice()
{
	currslice++;
	ShowSlice(currslice);

}
void SliceDebugger::PrevSlice()
{
	currslice--;
	ShowSlice(currslice);
}
void SliceDebugger::RefreshOptions()
{
	ppaintwidget->shownormals = ui.normalBox->isChecked();
	ppaintwidget->outlines = ui.OutlineBox->isChecked();
	ppaintwidget->connections = ui.ConnectionBox->isChecked();
	ppaintwidget->fills = ui.FillBox->isChecked();

	ppaintwidget->updateGL();
}
void SliceDebugger::BakeTests()
{
	//bake selected instances
    for(unsigned int i = 0; i < pMain->GetSelectedInstances().size(); i++)
	{
		pMain->GetSelectedInstances()[i]->BakeGeometry();
	}
}

//Private
void SliceDebugger::ShowSlice(int slice)
{
    unsigned int i;
	double thickness = pMain->project->GetPixelThickness()*0.001;
	ui.slicepick->setText(QString().number(currslice));
	for(i = 0; i < pMain->GetSelectedInstances().size(); i++)
	{
		ModelInstance* inst = pMain->GetSelectedInstances()[i];
			
		inst->pSliceSet->GenerateSlice(slice*thickness + thickness*0.5);
		ppaintwidget->SetSlice(inst->pSliceSet->pSliceData);
		
		ppaintwidget->updateGL();
	}
	

}




//events
void SliceDebugger::wheelEvent(QWheelEvent *event)
{
	ppaintwidget->zoom += event->delta()/100.0;
	if(ppaintwidget->zoom <= 0)
	{
		ppaintwidget->zoom = 0;
	}
	ppaintwidget->updateGL();
}
