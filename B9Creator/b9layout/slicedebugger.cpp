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
