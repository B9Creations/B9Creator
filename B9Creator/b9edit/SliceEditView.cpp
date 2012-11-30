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

#include <QtGui>
#include "SliceEditView.h"
#include <QClipboard>

/////////////////////////////////
//Public
/////////////////////////////////
SliceEditView::SliceEditView(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags)
{
	ui.setupUi(this);

	bGrid = false;
	supportMode = false;
	pCPJ = NULL;
	backupIndx = 0;
	m_xOffset = 0;
	m_yOffset = 0;
	currSlice = 0;

    setWindowIcon(QIcon(":/B9JobBuilder/icons/edit.png"));
	setStatusBar(0);
	//green timer connections
	greenTimer.setSingleShot(true);
	greenTimer.setInterval(0);
	greenTimer.stop();
	QObject::connect(&greenTimer,SIGNAL(timeout()),this,SLOT(RefreshWithGreen()));

	pDrawingContext = new DrawingContext(this);
	pDrawingContext->pEditView = this;
	
	SetSupportTool("circle");
	SetDrawTool("penfill");
	ui.actionPrepare_Base_Gap->setEnabled(false);
	ui.menuSupports->setEnabled(false);
	

	ui.scrollArea->setWidget(pDrawingContext);
	ui.scrollArea->setFocusPolicy(Qt::NoFocus);


	QObject::connect(this,SIGNAL(sliceDirtied(QImage*, int)),parent,SLOT(PatchJobData(QImage*, int)));

	//toolbars
	ui.toolBar->addAction(ui.actionSupport_Mode);
	ui.toolBar->addSeparator();
	
	ui.toolBar->addAction(ui.actionWhite_Pen);
	ui.toolBar->addAction(ui.actionBlack_Pen);
	ui.toolBar->addAction(ui.actionFlood_Fill);
	ui.toolBar->addAction(ui.actionFlood_Void);
	
	ui.toolBar->addSeparator();
	
	ui.toolBar->addAction(ui.actionCircle);
	ui.toolBar->addAction(ui.actionSquare);
	ui.toolBar->addAction(ui.actionTriangle_2);
	ui.toolBar->addAction(ui.actionDiamond);
	ShowSupportButtons(false);
}
SliceEditView::~SliceEditView()
{
}

QString SliceEditView::GetEditMode()
{
	if(supportMode)
	{
		return "Support Mode";
	}
	else
	{
		return "Image Mode";
	}

}

//public slots
void SliceEditView::TogSupportMode()
{
	
	supportMode = !supportMode;
	if(supportMode)//going INTO support mode
	{
		ui.actionSupport_Mode->setText("Enter Image Mode");
		ui.menuDrawing->setEnabled(false);
		ui.menuSupports->setEnabled(true);
		ui.actionCopy->setEnabled(false);
		ui.actionPaste->setEnabled(false);
		ShowDrawButtons(false);
		ShowSupportButtons(true);
		ui.actionPrepare_Base_Gap->setEnabled(true);
		SetSupportTool("currTool");
		pCPJ->showSupports(true);
		DeCompressIntoContext();
		pDrawingContext->GenerateLogicImage();
		RefreshWithGreen();
	}
	else//comming OUT of support mode
	{
		
		ui.actionSupport_Mode->setText("Enter Support Mode");
		ui.menuDrawing->setEnabled(true);
		ui.menuSupports->setEnabled(false);
		ui.actionCopy->setEnabled(true);
		ui.actionPaste->setEnabled(true);
		ShowDrawButtons(true);
		ShowSupportButtons(false);
		ui.actionPrepare_Base_Gap->setEnabled(false);
		SetDrawTool("currTool");
		pCPJ->showSupports(false);
		DeCompressIntoContext();
	}
	UpdateWidgets();
}
void SliceEditView::ReCompress()
{
	if(!supportMode)
	{
		emit sliceDirtied(&topImg,currSlice);
		modified = false;
	}
}
void SliceEditView::GoToSlice(int slicenumber)
{
	//make sure the suggested slice is in range...
	if(slicenumber > pCPJ->getTotalLayers() - 1 || slicenumber < 0)
	{
		return;
	}
	if(slicenumber != currSlice)
	{
		ClearUndoBuffer();
	}	
	
	currSlice = slicenumber;
	setWindowTitle("Slice Manager - " + GetEditMode() + ": " + QString().number(currSlice+1) + " / " + QString().number(pCPJ->getTotalLayers()));
	DeCompressIntoContext();
	
	if(supportMode)
	{
		RefreshContext(0);
	}
	else
	{
		RefreshContext(1);
	}
	
	if(!supportMode)
	{
		SaveToUndoBuffer(); //when moving on to a new slice, always save the state it's in to a buffer.
	}

	greenTimer.start();
}
void SliceEditView::DeCompressIntoContext()
{
	pCPJ->setCurrentSlice(currSlice);
	if(supportMode)
	{pCPJ->showSupports(1);}
	else
	{pCPJ->showSupports(0);}


	pCPJ->inflateCurrentSlice(&topImg, m_xOffset, m_yOffset, true);
	pDrawingContext->SetUpperImg(&topImg);
	
	if(currSlice <= 0)//make "base" image
	{
		botImg = QImage(topImg.width(),topImg.height(),QImage::Format_RGB16);
		botImg.fill(QColor(255,255,255));
	}
	else
	{
		pCPJ->setCurrentSlice(currSlice - 1);
		pCPJ->inflateCurrentSlice(&botImg, m_xOffset, m_yOffset, true);
	}
		
	pDrawingContext->SetLowerImg(&botImg);
}
void SliceEditView::RefreshContext(bool alreadywhite)
{
	if(!alreadywhite)
		pDrawingContext->GenerateLogicImage();
	pDrawingContext->repaint();
	
}
void SliceEditView::UpdateWidgets()
{
	setWindowTitle("Slice Manager - " + GetEditMode() + ": " + QString().number(currSlice+1) + " / " + QString().number(pCPJ->getTotalLayers()));
	ui.horizontalSlider->setMinimum(0);
	ui.horizontalSlider->setMaximum(pCPJ->getTotalLayers() - 1);
	ui.horizontalSlider->setValue(currSlice);
}
void SliceEditView::RefreshWithGreen()
{
	if(supportMode)
	{
		pDrawingContext->GenerateGreenImage();
		pDrawingContext->repaint();
	}
}

//base
void SliceEditView::PromptBaseOptions()
{
	int baselayers;
	int fills;
	bool cont;
	
    baselayers = QInputDialog::getInteger(this, tr("Attachment Base"), tr("Object Standoff, # of Layers:"), pCPJ->getBase(), 4, 1000, 1, &cont, windowFlags() & ~Qt::WindowContextHelpButtonHint);
    if (!cont)
		return;
    fills = QInputDialog::getInteger(this, tr("Attachment Base"), tr("Filled Layers:"), pCPJ->getFilled(), 1, baselayers, 1, &cont, windowFlags() & ~Qt::WindowContextHelpButtonHint);
	if (!cont)
		return;

	PrepareBase(baselayers, fills);
}
void SliceEditView::PrepareBase(int baselayers, int filledlayers)
{
	currSlice += baselayers - pCPJ->getBase();
	
	pCPJ->setBase(baselayers);
	pCPJ->setFilled(filledlayers);
	
	GoToSlice(currSlice);
	pBuilder->SetDirty();
	UpdateWidgets();
}

//suppports
void SliceEditView::AddSupport(QPoint pos, int size, SupportType type, int fastmode)
{
	QCursor prevC = pDrawingContext->cursor();
	pDrawingContext->setCursor(QCursor(Qt::WaitCursor));
	
	pCPJ->AddSupport(currSlice - 1,pos,size,type,fastmode);
	
	DeCompressIntoContext();
	pDrawingContext->GenerateLogicImage();
	RefreshWithGreen();
	pDrawingContext->setCursor(prevC);

	pBuilder->SetDirty();
}
void SliceEditView::DeleteAllSupports()
{
	QMessageBox::StandardButton ret;
	ret = QMessageBox::warning(this, tr("Slice Manager"),
				tr("Are you sure you want to remove all supports?"),
					QMessageBox::Yes | QMessageBox::No);
	
	if(ret == QMessageBox::No)
	{
		return;
	}

	pCPJ->DeleteAllSupports();
	DeCompressIntoContext();
	pDrawingContext->GenerateLogicImage();
	RefreshWithGreen();
	pBuilder->SetDirty();
}

//undo
void SliceEditView::ClearUndoBuffer()
{
	imgBackup.clear();
	backupIndx = -1;
}
void SliceEditView::SaveToUndoBuffer()
{
	int i;
	for(i = imgBackup.size() - 1; i > backupIndx; i--)
	{
		imgBackup.removeAt(i);
	}
	//add to the list of undos.
	imgBackup.append(topImg);
	if(imgBackup.size() > 25)
		{imgBackup.removeFirst();}
	backupIndx = imgBackup.size() - 1;
}
void SliceEditView::Undo()
{
	if(!supportMode)
	{
		if(backupIndx >= 1)
		{	
			backupIndx--;
			topImg = imgBackup[backupIndx];
			pDrawingContext->SetUpperImg(&topImg);
			RefreshContext(1);
			modified = true;
			ReCompress();
			pDrawingContext->update();
		}
	}
}
void SliceEditView::Redo()
{
	if(!supportMode)
	{
		backupIndx++;
		if(backupIndx >= imgBackup.size())
		{
			backupIndx = imgBackup.size() - 1;
			return;
		}

		topImg = imgBackup[backupIndx];
		pDrawingContext->SetUpperImg(&topImg);
		RefreshContext(1);
		modified = true;
		ReCompress();
		pDrawingContext->update();
	}
}

//clipboard
void SliceEditView::CopyToClipboard()
{
	if(supportMode)
		return;
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setImage(topImg);
}
void SliceEditView::PasteFromClipboard()
{
	if(supportMode)
		return;

	QClipboard *clipboard = QApplication::clipboard();
	if(clipboard->image().isNull())
		return;

	modified = true;
	topImg = clipboard->image();
	pDrawingContext->SetUpperImg(&topImg);
	RefreshContext(1);//so we can undo the paste...
	SaveToUndoBuffer();//since the image is altered, save to the buffer.
	ReCompress();
}

//tools
void SliceEditView::SelectPenColor()
{
	QColor newColor = QColorDialog::getColor(Qt::white);
     if (newColor.isValid())
         pDrawingContext->SetPenColor(newColor);
}
void SliceEditView::SelectPenWidth()
{
	bool ok;
    int newWidth = QInputDialog::getInteger(this, tr("Edit View"), tr("Select pen width:"), pDrawingContext->PenWidth, 1, 50, 1, &ok);
    if (ok) pDrawingContext->SetPenWidth(newWidth);

}
void SliceEditView::SetDrawTool(QString tool)
{
	PopDrawButtons();
	if(tool == "currTool")
	{
		tool = pDrawingContext->currDrawTool;
	}
	if(tool == "penfill")
	{
		pDrawingContext->SetPenColor(Qt::white);
        QPixmap bucket(":/Cursors/icons/crosshair.png","PNG");
		pDrawingContext->setCursor(QCursor(bucket));
		ui.actionWhite_Pen->setChecked(true);

	}
	else if(tool == "penvoid")
	{
		pDrawingContext->SetPenColor(Qt::black);
        QPixmap bucket(":/Cursors/icons/crosshair.png","PNG");
		pDrawingContext->setCursor(QCursor(bucket));
		ui.actionBlack_Pen->setChecked(true);

	}
	else if(tool == "floodfill")
	{
		pDrawingContext->SetPenColor(Qt::white);
        QPixmap bucket(":/Cursors/icons/bucketwhite.png","PNG");
		pDrawingContext->setCursor(QCursor(bucket,10,25));
		ui.actionFlood_Fill->setChecked(true);

	}
	else if(tool == "floodvoid")
	{
		pDrawingContext->SetPenColor(Qt::black);
        QPixmap bucket(":/Cursors/icons/bucket.png","PNG");
		pDrawingContext->setCursor(QCursor(bucket,10,25));
		ui.actionFlood_Void->setChecked(true);
	}
	
	pDrawingContext->currDrawTool = tool;
}
void SliceEditView::SetSupportTool(QString tool)
{
	PopSupportButtons();
	if(tool == "nextTool")
	{
		if(pDrawingContext->currSupportTool == "circle")
		{
			SetSupportTool("square");	
		}
		else if(pDrawingContext->currSupportTool == "square")
		{
			SetSupportTool("triangle");
		}
		else if(pDrawingContext->currSupportTool == "triangle")
		{
			SetSupportTool("diamond");	
		}
		else if(pDrawingContext->currSupportTool == "diamond")
		{
			SetSupportTool("circle");	
		}
		return;
	}
	if(tool == "currTool")
	{
		tool = pDrawingContext->currSupportTool;
	}
	if(tool == "circle")
	{
		QImage img = SimpleSupport(st_CIRCLE,pDrawingContext->supportSize).getCursorImage();
		pDrawingContext->setCursor(QCursor(QPixmap().fromImage(img)));
		ui.actionCircle->setChecked(true);
	}
	else if(tool == "square")
	{
		QImage img = SimpleSupport(st_SQUARE,pDrawingContext->supportSize).getCursorImage();
		pDrawingContext->setCursor(QCursor(QPixmap().fromImage(img)));
		ui.actionSquare->setChecked(true);
	}
	else if(tool == "triangle")
	{
		QImage img = SimpleSupport(st_TRIANGLE,pDrawingContext->supportSize).getCursorImage();
		pDrawingContext->setCursor(QCursor(QPixmap().fromImage(img)));
		ui.actionTriangle_2->setChecked(true);
	}
	else if(tool == "diamond")
	{
		QImage img = SimpleSupport(st_DIAMOND,pDrawingContext->supportSize).getCursorImage();
		pDrawingContext->setCursor(QCursor(QPixmap().fromImage(img)));
		ui.actionDiamond->setChecked(true);
	}

	pDrawingContext->currSupportTool = tool;
	
}
//navigation
void SliceEditView::NextSlice()
{
	GoToSlice(currSlice+1);
	ui.horizontalSlider->setValue(currSlice);
}
void SliceEditView::PrevSlice()
{
	GoToSlice(currSlice-1);
	ui.horizontalSlider->setValue(currSlice);
}
void SliceEditView::PgUpSlice(int increment)
{
	GoToSlice(currSlice+increment);
	ui.horizontalSlider->setValue(currSlice);
}
void SliceEditView::PgDownSlice(int increment)
{
	GoToSlice(currSlice-increment);
	ui.horizontalSlider->setValue(currSlice);
} 
void SliceEditView::BaseSlice()
{
	GoToSlice(0);
	ui.horizontalSlider->setValue(0);
}
void SliceEditView::TopSlice()
{
	GoToSlice(pCPJ->getTotalLayers() -1);
	ui.horizontalSlider->setValue(pCPJ->getTotalLayers() -1);
}

//Toolbar
void SliceEditView::PopDrawButtons()//uncheckes all drawing buttons
{
	ui.actionWhite_Pen->setChecked(false);
	ui.actionBlack_Pen->setChecked(false);
	ui.actionFlood_Fill->setChecked(false);
	ui.actionFlood_Void->setChecked(false);
}
void SliceEditView::SetDrawButtonsEnabled(bool enabled)//uncheckes all drawing buttons
{
	ui.actionWhite_Pen->setEnabled(enabled);
	ui.actionBlack_Pen->setEnabled(enabled);
	ui.actionFlood_Fill->setEnabled(enabled);
	ui.actionFlood_Void->setEnabled(enabled);
}
void SliceEditView::ShowDrawButtons(bool show)
{

	ui.actionBlack_Pen->setVisible(show);
	ui.actionWhite_Pen->setVisible(show);
	ui.actionFlood_Fill->setVisible(show);
	ui.actionFlood_Void->setVisible(show);
	

}
void SliceEditView::PopSupportButtons()//uncheckes all support buttons
{
	ui.actionCircle->setChecked(false);
	ui.actionSquare->setChecked(false);
	ui.actionTriangle_2->setChecked(false);
	ui.actionDiamond->setChecked(false);
}
void SliceEditView::SetSupportButtonsEnabled(bool enabled)//set enabled for all support buttons.
{
	ui.actionCircle->setEnabled(enabled);
	ui.actionSquare->setEnabled(enabled);
	ui.actionTriangle_2->setEnabled(enabled);
	ui.actionDiamond->setEnabled(enabled);
}
void SliceEditView::ShowSupportButtons(bool show)//hide/shows all support buttons
{
	ui.actionCircle->setVisible(show);
	ui.actionSquare->setVisible(show);
	ui.actionTriangle_2->setVisible(show);
	ui.actionDiamond->setVisible(show);
}

//widget callbacks
void SliceEditView::on_actionFlood_Fill_activated()
{
	SetDrawTool("floodfill");
}
void SliceEditView::on_actionFlood_Void_activated()
{
	SetDrawTool("floodvoid");
}
void SliceEditView::on_actionWhite_Pen_activated()
{
	SetDrawTool("penfill");
}
void SliceEditView::on_actionBlack_Pen_activated()
{
	SetDrawTool("penvoid");
}

void SliceEditView::on_actionCircle_activated()
{
	SetSupportTool("circle");
}
void SliceEditView::on_actionSquare_activated()
{
	SetSupportTool("square");
}
void SliceEditView::on_actionTriangle_2_activated()
{
	SetSupportTool("triangle");
}
void SliceEditView::on_actionDiamond_activated()
{
	SetSupportTool("diamond");
}

////////////////////////////////
//Protected
////////////////////////////////
void SliceEditView::keyPressEvent(QKeyEvent * pEvent)
{
	
	if(pEvent->key() == Qt::Key_PageUp)
	{
		PgUpSlice(10);
	}
	else if(pEvent->key() == Qt::Key_PageDown)
	{
		PgDownSlice(10);
	}
	else if(pEvent->key() == Qt::Key_Control)
	{
		pDrawingContext->fastsupportmode = false;
	}
	else if(pEvent->key() == Qt::Key_C)
	{
		SetSupportTool("nextTool");
	}
	else if(pEvent->key() == Qt::Key_Delete)
	{
		if(supportMode)
		{
            QPixmap bucket(":/Cursors/icons/delete.png","PNG");
			pDrawingContext->setCursor(QCursor(bucket));
			pDrawingContext->deletemode = true;
		}
	}
	else if(pEvent->key() == Qt::Key_Shift)
	{
		pDrawingContext->supportSize = pDrawingContext->supportSize * 0.5;
		SetSupportTool("currTool");
	}
	else
	{
		QWidget::keyPressEvent(pEvent);
	}
}
void SliceEditView::keyReleaseEvent(QKeyEvent * pEvent)
{
	if(pEvent->key() == Qt::Key_Control)
	{
		pDrawingContext->fastsupportmode = true;
	}
	else if(pEvent->key() == Qt::Key_Shift)
	{
		pDrawingContext->supportSize = pDrawingContext->supportSize*2;
		SetSupportTool("currTool");
	}
	else if(pEvent->key() == Qt::Key_Delete)
	{
		if(supportMode)
		{
			SetSupportTool("currTool");
			pDrawingContext->deletemode = false;
		}
	}
	else
	{
		QWidget::keyPressEvent(pEvent);
	}
}

void SliceEditView::mouseReleaseEvent(QMouseEvent * pEvent)
{
	QWidget::mouseReleaseEvent(pEvent);
}




