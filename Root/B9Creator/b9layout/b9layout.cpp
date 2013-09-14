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

#include "b9layout.h"
#include "b9layoutprojectdata.h"
#include "crushbitmap.h"
#include "slicecontext.h"
#include "sliceset.h"
#include "slice.h"
#include "loadingbar.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVector3D>
#include <QGLWidget>
#include <QDebug>
#include "SlcExporter.h"
#include "modeldata.h"
#include "b9modelinstance.h"
#include "b9supportstructure.h"
#include "OS_Wrapper_Functions.h"
#include "b9modelwriter.h"

//////////////////////////////////////////////////////
//Public
//////////////////////////////////////////////////////
B9Layout::B9Layout(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags)
{
	ui.setupUi(this);


	//Initialize project data
    project = new B9LayoutProjectData();
    project->pMain = this;
	

	//Create the worldview and attach it to the window
    pWorldView = new WorldView(NULL,this);

    ui.WorldViewContext->addWidget(pWorldView);
    pWorldView->show();
    SetToolModelSelect();//start off with pointer tool

	
    //support editing
    currInstanceInSupportMode = NULL;
    useContourAid = true;
    useXRayVision = false;
    xRayPercentage = 0.5;
    hideSupports = true;

	//slicing
	cancelslicing = false;

    //build the interface initially - different than update interface..
    BuildInterface();

    New();

    UpdateInterface();
}

B9Layout::~B9Layout()
{
	
    unsigned int m;
	for(m=0;m<ModelDataList.size();m++)
	{
		delete ModelDataList[m];
	}
	delete project;
	delete pWorldView;

}

//returns a list of the currently selected instances
std::vector<B9ModelInstance*> B9Layout::GetSelectedInstances()
{
    std::vector<B9ModelInstance*> insts;
    int i;
    for(i=0;i<ui.ModelList->selectedItems().size();i++)
    {
        insts.push_back(FindInstance(ui.ModelList->selectedItems()[i]));
    }
    return insts;
}
std::vector<B9ModelInstance*> B9Layout::GetAllInstances()
{
    unsigned int d, i;
    std::vector<B9ModelInstance*> allInsts;

    for(d = 0; d < ModelDataList.size(); d++)
    {
        for(i = 0; i < ModelDataList[d]->instList.size(); i++)
        {
            allInsts.push_back(ModelDataList[d]->instList[i]);
        }
    }

    return allInsts;


}











//////////////////////////////////////////////////////
//Public Slots
//////////////////////////////////////////////////////

//debug interface
void B9Layout::OpenDebugWindow()
{


}

//file
void B9Layout::New()
{
    SetSupportMode(false);
    RemoveAllInstances(); //remove instances.
	project->New();
    UpdateBuildSpaceUI();
    project->SetDirtied(false);//because UpdatingBuildSpaceUI dirties things in a round about way.
    pWorldView->CenterView();
    UpdateInterface();
}
QString B9Layout::Open(bool withoutVisuals)
{
	bool success;

    QSettings settings;


    //first check if the user needs to save what hes working on first.
    if(project->IsDirtied())
    {
        QMessageBox msgBox;
        msgBox.setText("The current layout has been modified.");
         msgBox.setInformativeText("Do you want to save your changes before opening?");
         msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
         msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret)
        {
          case QMessageBox::Save:
                SaveAs();
              break;
          case QMessageBox::Discard:
                //do nothing
              break;
          case QMessageBox::Cancel:
            return "";
              break;
          default:
              break;
        }
    }


	QString filename = QFileDialog::getOpenFileName(this,
             tr("Open Layout"), settings.value("WorkingDir").toString(), tr("B9Layout (*.b9l)"));

	if(filename.isEmpty())
    {return "";}


    QApplication::setOverrideCursor(Qt::WaitCursor);

    SetSupportMode(false);
    RemoveAllInstances(); //remove instances.
    success = project->Open(filename,withoutVisuals);

    //lets update some of the UI stuff to match what we just loaded.
    UpdateBuildSpaceUI();
    pWorldView->UpdatePlasmaFence();

    QApplication::restoreOverrideCursor();
	if(!success)
	{
        QMessageBox::warning(this, tr("B9Layout"), tr("Unable To Open Layout"),QMessageBox::Ok);

        return "";
	}

	CleanModelData();// now delete unneeded model data

    //set recent directory.
    settings.setValue("WorkingDir", QFileInfo(filename).absolutePath());

    return filename;

}
void B9Layout::Save()
{
    if(project->GetFileName() == "untitled")
	{
		SaveAs();
	}
	else
	{
		project->Save(project->GetFileName());
	}
}
void B9Layout::SaveAs()
{
	bool success;

    QSettings settings;

    QString filename = CROSS_OS_GetSaveFileName(this, tr("Save Layout"),
                    settings.value("WorkingDir").toString(),
                                                tr("B9 Layout (*.b9l)"),QStringList("b9l"));
	if(filename.isEmpty())
	{
        return;
	}

	success = project->Save(filename);
	if(!success)
	{
        QMessageBox::warning(this, tr("B9Layout"), tr("Unable To Save Project"),QMessageBox::Ok);
		return;
	}

    settings.setValue("WorkingDir",QFileInfo(filename).absolutePath());
}

//interface
void B9Layout::OnChangeTab(int idx)
{
    if(idx == 1)
    {
        SetSupportMode(true);
        ui.actionSupportMode->blockSignals(true);
        ui.actionSupportMode->setChecked(true);
        ui.actionSupportMode->blockSignals(false);
    }
    if(idx == 0)
    {
        ui.actionSupportMode->blockSignals(true);
        ui.actionSupportMode->setChecked(false);
        ui.actionSupportMode->blockSignals(false);
        SetSupportMode(false);
    }
}

void B9Layout::SetXYPixelSizePreset(QString size)
{
	project->SetPixelSize(size.toDouble());
	project->CalculateBuildArea();
    UpdateExtentsUI();
}

void B9Layout::SetZLayerThickness(QString thick)
{
	project->SetPixelThickness(thick.toDouble());
	project->CalculateBuildArea();
    UpdateExtentsUI();
}

void B9Layout::SetProjectorX(QString x)
{
	project->SetResolution(QVector2D(x.toInt(),project->GetResolution().y()));
	project->CalculateBuildArea();
}
void B9Layout::SetProjectorY(QString y)
{
	project->SetResolution(QVector2D(project->GetResolution().x(),y.toInt()));
	project->CalculateBuildArea();
}
void B9Layout::SetProjectorPreset(int index)
{
    switch(index)
    {
        case 0:
            SetProjectorX(QString().number(1024));
            SetProjectorY(QString().number(768));

            break;
        case 1:
            SetProjectorX(QString().number(1280));
            SetProjectorY(QString().number(768));

            break;
        case 2:
            SetProjectorX(QString().number(1920));
            SetProjectorY(QString().number(1080));
            break;
        case 3:
            SetProjectorX(QString().number(1920));
            SetProjectorY(QString().number(1200));
            break;
        case 4:
            SetProjectorX(QString().number(2048));
            SetProjectorY(QString().number(1536));

            break;
        default:


            break;

    }
    UpdateExtentsUI();

}

void B9Layout::SetZHeight(QString z)
{
	project->SetBuildSpaceSize(QVector3D(project->GetBuildSpace().x(),project->GetBuildSpace().x(), z.toDouble()));

}

void B9Layout::SetAttachmentSurfaceThickness(QString num)
{
    project->SetAttachmentSurfaceThickness(num.toDouble());
}

void B9Layout::UpdateBuildSpaceUI()
{
    int pixi;
    int proi;

    //pixel sizes
    if(project->GetPixelSize() == 50)
        pixi=0;
    else if(project->GetPixelSize() == 75)
        pixi=1;
    else if(project->GetPixelSize() == 100)
        pixi=2;

    //projector resolutions
    if(project->GetResolution() == QVector2D(1024,768))
        proi=0;
    else if(project->GetResolution() == QVector2D(1280,768))
        proi=1;
    else if(project->GetResolution() == QVector2D(1920,1080))
        proi=2;
    else if(project->GetResolution() == QVector2D(1920,1200))
        proi=3;
    else if(project->GetResolution() == QVector2D(2048,1536))
        proi=4;



    ui.pixelsizecombo->setCurrentIndex(pixi);
    ui.projectorcombo->setCurrentIndex(proi);

}


void B9Layout::UpdateExtentsUI()
{

    ui.Print_Extents_X->setText(QString::number(ProjectData()->GetBuildSpace().x(),'g',4));
    ui.Print_Extents_Y->setText(QString::number(ProjectData()->GetBuildSpace().y(),'g',4));
    ui.Print_Extents_Z->setText(QString::number(ProjectData()->GetBuildSpace().z(),'g',4));

}




void B9Layout::BuildInterface()
{
    unsigned int i, indx;

    //toolbar items
    ui.fileToolBar->addAction(ui.actionNew_Project);
    ui.fileToolBar->addAction(ui.actionOpen_Project);
    ui.fileToolBar->addAction(ui.actionSave);

    ui.modelToolBar->addAction(ui.actionSelection);
    ui.modelToolBar->addAction(ui.actionMove);
    ui.modelToolBar->addAction(ui.actionSpin);
    ui.modelToolBar->addAction(ui.actionOrientate);
    ui.modelToolBar->addAction(ui.actionScale);

    //support tool bar goes here.
    ui.supportToolBar->addAction(ui.Support_Add_Supports_action);
    ui.supportToolBar->addAction(ui.Support_Delete_Supports_action);
    ui.supportToolBar->addAction(ui.Support_Modify_Support_action);
    ui.supportToolBar->hide();



    ui.modelModifierToolBar->addAction(ui.actionDrop_To_Floor);
    ui.modelModifierToolBar->addAction(ui.actionDuplicate);
    ui.modelModifierToolBar->addAction(ui.actionReset_Rotation);


    ui.viewToolBar->addAction(ui.actionPerspective);
    ui.viewToolBar->addAction(ui.actionTop_View);
    ui.viewToolBar->addAction(ui.actionFront_View);
    ui.viewToolBar->addAction(ui.actionRight_View);
    ui.viewToolBar->addAction(ui.actionLeft_View);
    ui.viewToolBar->addAction(ui.actionBottom_View);
    ui.viewToolBar->addAction(ui.actionBack_View);


    //edit Support tab
    //fill the top, mid and lower combos with the valid names of attachement data.
    ui.Support_Top_AttachType_comboBox->blockSignals(true);
    ui.Support_Bottom_AttachType_comboBox->blockSignals(true);
    ui.Support_Mid_AttachType_comboBox->blockSignals(true);
    for(i = 0; i < B9SupportStructure::AttachmentDataList.size(); i++)
    {
        ui.Support_Top_AttachType_comboBox->addItem(B9SupportStructure::AttachmentDataList[i].GetName());
        ui.Support_Mid_AttachType_comboBox->addItem(B9SupportStructure::AttachmentDataList[i].GetName());
        ui.Support_Bottom_AttachType_comboBox->addItem(B9SupportStructure::AttachmentDataList[i].GetName());
        ui.Support_Base_AttachType_comboBox->addItem(B9SupportStructure::AttachmentDataList[i].GetName());
    }
    ui.Support_Top_AttachType_comboBox->blockSignals(false);
    ui.Support_Bottom_AttachType_comboBox->blockSignals(false);
    ui.Support_Mid_AttachType_comboBox->blockSignals(false);

    ui.Support_Top_AngleFactor_horizontalSlider->setMaximum(100);
    ui.Support_Top_AngleFactor_horizontalSlider->setMinimum(0);
    ui.Support_Bottom_AngleFactor_horizontalSlider->setMaximum(100);
    ui.Support_Bottom_AngleFactor_horizontalSlider->setMinimum(0);

    //show top support tab
    ui.Support_Section_Information_Box->setCurrentIndex(0);


    //base plate defaults are really held in the gui
    indx = ui.Support_Base_AttachType_comboBox->findText("Cylinder");
    ui.Support_Base_AttachType_comboBox->blockSignals(true);
        ui.Support_Base_AttachType_comboBox->setCurrentIndex(indx);
    ui.Support_Base_AttachType_comboBox->blockSignals(false);
    ui.Support_Base_Coverage_horizontalSlider->blockSignals(true);
        ui.Support_Base_Coverage_horizontalSlider->setValue(100);//percent
    ui.Support_Base_Coverage_horizontalSlider->blockSignals(false);
    ui.Support_Base_Coverage_label->setText("100%");
    ui.Support_Base_Length_lineEdit->setText("0.5");


    //connections:
    QObject::connect(project,SIGNAL(DirtChanged(bool)),this,SLOT(UpdateInterface()));
    QObject::connect(ui.actionCenter_View,SIGNAL(activated()),pWorldView,SLOT(CenterView()));
    QObject::connect(ui.actionTop_View,SIGNAL(activated()),pWorldView,SLOT(TopView()));
    QObject::connect(ui.actionRight_View,SIGNAL(activated()),pWorldView,SLOT(RightView()));
    QObject::connect(ui.actionFront_View,SIGNAL(activated()),pWorldView,SLOT(FrontView()));
    QObject::connect(ui.actionBack_View,SIGNAL(activated()),pWorldView,SLOT(BackView()));
    QObject::connect(ui.actionLeft_View,SIGNAL(activated()),pWorldView,SLOT(LeftView()));
    QObject::connect(ui.actionBottom_View,SIGNAL(activated()),pWorldView,SLOT(BottomView()));
    QObject::connect(ui.TabWidget,SIGNAL(currentChanged(int)),this,SLOT(OnChangeTab(int)));
    QObject::connect(ui.actionSupportMode,SIGNAL(triggered(bool)),this,SLOT(SetSupportMode(bool)));

    QObject::connect(ui.actionPerspective,SIGNAL(toggled(bool)),pWorldView,SLOT(SetPerpective(bool)));
    QObject::connect(ui.ModelList,SIGNAL(itemSelectionChanged()),this,SLOT(RefreshSelectionsFromList()));
    QObject::connect(ui.SupportList,SIGNAL(itemSelectionChanged()),this,SLOT(RefreshSupportSelectionsFromList()));

    pXRaySlider = new QSlider(ui.viewToolBar);
    pXRaySlider->setOrientation(Qt::Horizontal);
    pXRaySlider->setMaximum(1000);
    pXRaySlider->setMinimum(0);
    pXRaySlider->setValue(500);
    pXRaySlider->setEnabled(false);
    ui.xRayToolBar->addAction(ui.actionX_Ray_Vision);
    ui.xRayToolBar->addWidget(pXRaySlider);

    QObject::connect(pXRaySlider,SIGNAL(valueChanged(int)),this,SLOT(OnXRayChange(int)));

}



void B9Layout::UpdateInterface()
{
    if(ui.ModelList->selectedItems().size() <= 0 )//no items selected.
    {
        ui.menuModelModifiers->setEnabled(false);
        ui.modelModifierToolBar->hide();
        ui.duplicateButton->setEnabled(false);
        ui.RemoveButton->setEnabled(false);
    }
    else//1 or more items selected
    {
        if(!SupportModeInst())
        {
            ui.menuModelModifiers->setEnabled(true);
            ui.modelModifierToolBar->show();

        }
        ui.duplicateButton->setEnabled(true);
        ui.RemoveButton->setEnabled(true);
    }
    if(ui.ModelList->selectedItems().size() <= 0 || ui.ModelList->selectedItems().size() > 1)
        //no or multiple items selected.
	{

        ui.TabWidget->blockSignals(true);
            ui.TabWidget->setTabEnabled(1,false);
        ui.TabWidget->blockSignals(false);
        ui.actionSupportMode->setEnabled(false);

        ui.ModelTranslationBox->hide();

	}
    else //exactly ONE object selected
	{
        ui.ModelTranslationBox->show();

        ui.actionSupportMode->setEnabled(true);
        ui.TabWidget->blockSignals(true);
        ui.TabWidget->setTabEnabled(1,true);
        ui.TabWidget->blockSignals(false);

        B9ModelInstance* inst = FindInstance(ui.ModelList->selectedItems()[0]);
        ui.posx->setText(QString().number(inst->GetPos().x(),'f',2));
        ui.posy->setText(QString().number(inst->GetPos().y(),'f',2));
        ui.posz->setText(QString().number(inst->GetMinBound().z(),'f',2));
		
        ui.rotz->setText(QString().number(inst->GetRot().z()));
        ui.rotx->setText(QString().number(inst->GetRot().x()));
        ui.roty->setText(QString().number(inst->GetRot().y()));

        ui.Model_Spin_horizontalSlider->blockSignals(true);
            ui.Model_Spin_horizontalSlider->setValue(inst->GetRot().z());
        ui.Model_Spin_horizontalSlider->blockSignals(false);

		ui.scalex->setText(QString().number(inst->GetScale().x()));
		ui.scaley->setText(QString().number(inst->GetScale().y()));
		ui.scalez->setText(QString().number(inst->GetScale().z()));

        ui.flipx->setChecked(inst->GetFlipped());

		ui.modelsizex->setText(QString().number(inst->GetMaxBound().x() - inst->GetMinBound().x()));
		ui.modelsizey->setText(QString().number(inst->GetMaxBound().y() - inst->GetMinBound().y()));
		ui.modelsizez->setText(QString().number(inst->GetMaxBound().z() - inst->GetMinBound().z()));
	}

    //refresh support structure tab as well.
    if(SupportModeInst() != NULL)//in support mode
    {
        ui.menuSupport_Tools->setEnabled(true);
        ui.menuModelModifiers->setEnabled(false);
        ui.menuModelTools->setEnabled(false);
        ui.actionContour_Aid->setEnabled(true);
        ui.modelModifierToolBar->hide();
        ui.modelToolBar->hide();
        ui.supportToolBar->show();
        ui.xRayToolBar->show();
        ui.actionX_Ray_Vision->setEnabled(true);
        ui.actionHide_Supports->setEnabled(true);

        if(pWorldView->GetTool() == "SUPPORTADD")
        {
            ui.supportInformationBox->show();
            ui.Support_Reset_To_Vertical_button->hide();
            ui.Support_Set_To_Straight_button->hide();

            ui.Support_Reset_Light_button->setCheckable(true);
            ui.Support_Reset_Medium_button->setCheckable(true);
            ui.Support_Reset_Heavy_button->setCheckable(true);

            //start off with the right button checked
            QSettings appSettings;
            appSettings.beginGroup("USERSUPPORTPARAMS");
            QString weight = appSettings.value("ADDPRESETWEIGHT","LIGHT").toString();
            if(weight == "LIGHT") ui.Support_Reset_Light_button->setChecked(true);
            if(weight == "MEDIUM") ui.Support_Reset_Medium_button->setChecked(true);
            if(weight == "HEAVY") ui.Support_Reset_Heavy_button->setChecked(true);
        }

        if(pWorldView->GetTool() == "SUPPORTDELETE")
        {
            ui.supportInformationBox->hide();
             ui.Support_Reset_To_Vertical_button->hide();
             ui.Support_Set_To_Straight_button->hide();
        }

        if(pWorldView->GetTool() == "SUPPORTMODIFY")
            if(currSelectedSupports.size() <= 0)//no items selected.
            {
               ui.supportInformationBox->hide();
            }
            else
            {
                ui.supportInformationBox->show();
                ui.Support_Reset_To_Vertical_button->show();
                ui.Support_Set_To_Straight_button->show();

                ui.Support_Reset_Light_button->setCheckable(false);
                ui.Support_Reset_Medium_button->setCheckable(false);
                ui.Support_Reset_Heavy_button->setCheckable(false);


                PushSupportProperties();
            }

        //fill the base plate gui region
        PushBasePlateProperties();
    }
    else//out of support mode
    {
        ui.menuSupport_Tools->setEnabled(false);
        ui.menuModelTools->setEnabled(true);
        ui.modelToolBar->show();
        ui.supportToolBar->hide();
        ui.actionContour_Aid->setEnabled(false);
        ui.xRayToolBar->hide();
        ui.actionX_Ray_Vision->setEnabled(false);
        ui.actionHide_Supports->setEnabled(false);

        UpdateExtentsUI();

    }
}
void B9Layout::PushTranslations()
{
    QString scalex;
        QString scaley;
        QString scalez;
        if(ui.scalelock->isChecked())
        {
            scalex = ui.scalex->text();
            scaley = ui.scalex->text();
            scalez = ui.scalex->text();
            ui.scaley->blockSignals(true);
                ui.scaley->setText(scalex);
            ui.scaley->blockSignals(false);
            ui.scalez->blockSignals(true);
                ui.scalez->setText(scalex);
            ui.scalez->blockSignals(true);
        }
        else
        {
            scalex = ui.scalex->text();
            scaley = ui.scaley->text();
            scalez = ui.scalez->text();
        }

        SetSelectionPos(ui.posx->text().toDouble(),0,0,1);
        SetSelectionPos(0,ui.posy->text().toDouble(),0,2);
        SetSelectionPos(0,0,ui.posz->text().toDouble(),3);

        SetSelectionRot(QVector3D(ui.rotx->text().toDouble(),
                                  ui.roty->text().toDouble(),
                                  ui.rotz->text().toDouble()));

        SetSelectionScale(scalex.toDouble(),0,0,1);
        SetSelectionScale(0,scaley.toDouble(),0,2);
        SetSelectionScale(0,0,scalez.toDouble(),3);

        for(unsigned int i = 0; i < GetSelectedInstances().size(); i++)
        {
            GetSelectedInstances()[i]->UpdateBounds();
        }
        UpdateInterface();
}

//when the spin slider changes value by the user.
void B9Layout::OnModelSpinSliderChanged(int val)
{
    SetSelectionRot(QVector3D(ui.rotx->text().toDouble(),
                              ui.roty->text().toDouble(),
                              val));

    ui.rotz->setText(QString::number(val));
}
void B9Layout::OnModelSpinSliderReleased()
{
    for(unsigned int i = 0; i < GetSelectedInstances().size(); i++)
    {
        GetSelectedInstances()[i]->UpdateBounds();
    }
    UpdateInterface();
}

void B9Layout::LockScale(bool lock)
{
	if(lock)
	{
		SetSelectionScale(ui.scalex->text().toDouble(),0,0,1);
		SetSelectionScale(0,ui.scalex->text().toDouble(),0,2);
		SetSelectionScale(0,0,ui.scalex->text().toDouble(),3);
        UpdateInterface();
	}
}


//tools interface
void B9Layout::SetTool(QString toolname)
{
    if(toolname == "MODELSELECT")
        SetToolModelSelect();
    else if(toolname == "MODELMOVE")
        SetToolModelMove();
    else if(toolname == "MODELSPIN")
        SetToolModelSpin();
    else if(toolname == "MODELORIENTATE")
        SetToolModelOrientate();
    else if(toolname == "MODELSCALE")
        SetToolModelScale();
    else if(toolname == "SUPPORTMODIFY")
        SetToolSupportModify();
    else if(toolname == "SUPPORTADD")
        SetToolSupportAdd();
    else if(toolname == "SUPPORTDELETE")
        SetToolSupportDelete();
}

void B9Layout::SetToolModelSelect()
{
    pWorldView->SetTool("MODELSELECT");
    ui.actionSelection->setChecked(true);
	ui.actionMove->setChecked(false);
    ui.actionOrientate->setChecked(false);
    ui.actionSpin->setChecked(false);
    ui.actionScale->setChecked(false);
}
void B9Layout::SetToolModelMove()
{
    pWorldView->SetTool("MODELMOVE");
    ui.actionMove->setChecked(true);
	ui.actionSelection->setChecked(false);
    ui.actionOrientate->setChecked(false);
    ui.actionSpin->setChecked(false);
    ui.actionScale->setChecked(false);
}
void B9Layout::SetToolModelSpin()
{
    pWorldView->SetTool("MODELSPIN");
    ui.actionOrientate->setChecked(false);
    ui.actionSpin->setChecked(true);
	ui.actionMove->setChecked(false);
	ui.actionSelection->setChecked(false);
    ui.actionScale->setChecked(false);
}
void B9Layout::SetToolModelOrientate()
{
    pWorldView->SetTool("MODELORIENTATE");
    ui.actionOrientate->setChecked(true);
    ui.actionSpin->setChecked(false);
    ui.actionMove->setChecked(false);
    ui.actionSelection->setChecked(false);
    ui.actionScale->setChecked(false);
}

void B9Layout::SetToolModelScale()
{
    pWorldView->SetTool("MODELSCALE");
    ui.actionScale->setChecked(true);
    ui.actionMove->setChecked(false);
    ui.actionSelection->setChecked(false);
    ui.actionOrientate->setChecked(false);
    ui.actionSpin->setChecked(false);
}

//SupportTools interface
void B9Layout::SetToolSupportModify()
{
    pWorldView->SetTool("SUPPORTMODIFY");
    ui.Support_Modify_Support_action->blockSignals(true);
        ui.Support_Modify_Support_action->setChecked(true);
    ui.Support_Modify_Support_action->blockSignals(false);

    ui.Support_Add_Supports_action->blockSignals(true);
        ui.Support_Add_Supports_action->setChecked(false);
    ui.Support_Add_Supports_action->blockSignals(false);

    ui.Support_Delete_Supports_action->blockSignals(true);
        ui.Support_Delete_Supports_action->setChecked(false);
    ui.Support_Delete_Supports_action->blockSignals(false);

    UpdateInterface();
}

void B9Layout::SetToolSupportAdd()
{
    pWorldView->SetTool("SUPPORTADD");
    ui.Support_Add_Supports_action->blockSignals(true);
        ui.Support_Add_Supports_action->setChecked(true);
    ui.Support_Add_Supports_action->blockSignals(false);

    ui.Support_Modify_Support_action->blockSignals(true);
        ui.Support_Modify_Support_action->setChecked(false);
    ui.Support_Modify_Support_action->blockSignals(false);

    ui.Support_Delete_Supports_action->blockSignals(true);
        ui.Support_Delete_Supports_action->setChecked(false);
    ui.Support_Delete_Supports_action->blockSignals(false);

    //when were adding nothing should be selected
    DeSelectAllSupports();

    UpdateInterface();

    FillSupportParamsWithDefaults();


}

void B9Layout::SetToolSupportDelete()
{
    pWorldView->SetTool("SUPPORTDELETE");
    ui.Support_Add_Supports_action->blockSignals(true);
        ui.Support_Add_Supports_action->setChecked(false);
    ui.Support_Add_Supports_action->blockSignals(false);

    ui.Support_Modify_Support_action->blockSignals(true);
        ui.Support_Modify_Support_action->setChecked(false);
    ui.Support_Modify_Support_action->blockSignals(false);

    ui.Support_Delete_Supports_action->blockSignals(true);
        ui.Support_Delete_Supports_action->setChecked(true);
    ui.Support_Delete_Supports_action->blockSignals(false);

    //when were deleteing nothing should be selected
    DeSelectAllSupports();

    UpdateInterface();
}


void B9Layout::ExitToolAction()
{
    pWorldView->ExitToolAction();
}



//contour aid
void B9Layout::OnToggleContourAid(bool tog)
{
    useContourAid = tog;
}

bool B9Layout::ContourAidEnabled()
{
    return useContourAid;
}

//xray vision
void B9Layout::OnToggleXRay(bool tog)
{

    if(tog)
    {
        pXRaySlider->setEnabled(true);
        useXRayVision = true;
    }
    else
    {
        pXRaySlider->setEnabled(false);
        useXRayVision = false;
    }
}

void B9Layout::OnXRayChange(int val)
{
    xRayPercentage = float(val)/1000.0;
}

bool B9Layout::XRayEnabled()
{
    return useXRayVision;
}

float B9Layout::XRayPercentage()
{
    return xRayPercentage;
}

//support hiding
bool B9Layout::HidingSupports()
{
    return hideSupports;
}

void B9Layout::OnToggleSupportHiding(bool tog)
{
    hideSupports = tog;
}





//model
B9ModelInstance* B9Layout::AddModel(QString filepath, bool bypassVisuals)
{
    QSettings settings;

	if(filepath.isEmpty())
	{
		filepath = QFileDialog::getOpenFileName(this,
            tr("Open Model"), settings.value("WorkingDir").toString(), tr("Models (*.stl)"));

		//cancel button
		if(filepath.isEmpty())
			return NULL;
	}
    //by this point we should have a valid file path, if we dont - abort.
    if(!QFileInfo(filepath).exists())
    {
        return NULL;
    }

	//if the file has already been opened and is in the project, we dont want to load in another! instead we want to make a new instance
    for(unsigned int i = 0; i < ModelDataList.size(); i++)
	{
		if(ModelDataList[i]->GetFilePath() == filepath)
		{
			return ModelDataList[i]->AddInstance();//make a new instance
		}
	}

    ModelData* pNewModel = new ModelData(this,bypassVisuals);
	
	bool success = pNewModel->LoadIn(filepath);
	if(!success)
	{
		delete pNewModel;
		return NULL;
	}

    //update registry
    settings.setValue("WorkingDir",QFileInfo(filepath).absolutePath());
	
	//add to the list
	ModelDataList.push_back(pNewModel);

	//make an Instance of the model!
    B9ModelInstance* pNewInst = pNewModel->AddInstance();
	project->UpdateZSpace();

    //select it too
    SelectOnly(pNewInst);

	return pNewInst;
}
void B9Layout::RemoveAllInstances()
{
    unsigned int m;
    unsigned int i;

    std::vector<B9ModelInstance*> allinstlist;
	for(m=0;m<this->ModelDataList.size();m++)
	{
		ModelDataList[m]->loadedcount = 0;//also reset the index counter for instances!
		for(i=0;i<ModelDataList[m]->instList.size();i++)
		{
            allinstlist.push_back(ModelDataList[m]->instList[i]);
		}
	}
    for(i=0;i<allinstlist.size();i++)
    {
        delete allinstlist[i];
    }

	CleanModelData();
}
void B9Layout::CleanModelData()
{
    unsigned int m;
	std::vector<ModelData*> templist;
	for(m=0;m<ModelDataList.size();m++)
	{
		if(ModelDataList[m]->instList.size() > 0)
		{
			templist.push_back(ModelDataList[m]);
		}
		else
		{
			delete ModelDataList[m];
		}
	}
	ModelDataList.clear();
	ModelDataList = templist;
}

void B9Layout::AddTagToModelList(QListWidgetItem* item)
{
	ui.ModelList->addItem(item);
}

B9ModelInstance* B9Layout::FindInstance(QListWidgetItem* item)
{
    unsigned int d;
    unsigned int i;
	for(d=0;d<ModelDataList.size();d++)
	{
		for(i=0;i<ModelDataList[d]->instList.size();i++)
		{
			if(ModelDataList[d]->instList[i]->listItem == item)
			{
				return ModelDataList[d]->instList[i];
			}
		}
	}
	return NULL;
}


//selection
void B9Layout::RefreshSelectionsFromList()
{
	int i;
	for(i=0;i<ui.ModelList->count();i++)
	{
        B9ModelInstance* inst = FindInstance(ui.ModelList->item(i));
		if(inst == NULL)
			return;
		
		if(!ui.ModelList->item(i)->isSelected())
		{
			DeSelect(FindInstance(ui.ModelList->item(i)));
		}
		else if(ui.ModelList->item(i)->isSelected())
		{
			Select(FindInstance(ui.ModelList->item(i)));
		}
	}
}
void B9Layout::Select(B9ModelInstance* inst)
{
    //qDebug() << inst << "added to selection";
	inst->SetSelected(true);
    UpdateInterface();
}
void B9Layout::DeSelect(B9ModelInstance* inst)
{
    //qDebug() << inst << "removed from selection";
	inst->SetSelected(false);
    UpdateInterface();
}
void B9Layout::SelectOnly(B9ModelInstance* inst)
{
	DeSelectAll();
	Select(inst);
}
void B9Layout::SelectOnly(B9SupportStructure* sup)
{
    DeSelectAllSupports();
    SelectSupport(sup);
}


void B9Layout::DeSelectAll()
{
    unsigned int m;
    unsigned int i;
	for(m=0;m<ModelDataList.size();m++)
	{
		for(i=0;i<ModelDataList[m]->instList.size();i++)
		{
			DeSelect(ModelDataList[m]->instList[i]);
		}
	}
}

void B9Layout::SetSelectionPos(double x, double y, double z, int axis)
{
    int i;
    for(i=0;i<ui.ModelList->selectedItems().size();i++)
    {
        B9ModelInstance* inst = FindInstance(ui.ModelList->selectedItems()[i]);
        if(axis==0)
            inst->SetPos(QVector3D(x,y,z));
        else if(axis==1)
            inst->SetPos(QVector3D(x,inst->GetPos().y(),inst->GetPos().z()));
        else if(axis==2)
            inst->SetPos(QVector3D(inst->GetPos().x(),y,inst->GetPos().z()));
        else if(axis==3)
            inst->SetPos(QVector3D(inst->GetPos().x(),inst->GetPos().y(),z + inst->GetPos().z() - inst->GetMinBound().z()));
    }
}
void B9Layout::SetSelectionRot(QVector3D newRot)
{
    int i;
    for(i=0;i<ui.ModelList->selectedItems().size();i++)
    { 
        B9ModelInstance* inst = FindInstance(ui.ModelList->selectedItems()[i]);

        inst->SetRot(newRot);
    }
}
void B9Layout::SetSelectionScale(double x, double y, double z, int axis)
{
    int i;
    for(i=0;i<ui.ModelList->selectedItems().size();i++)
    {
        B9ModelInstance* inst = FindInstance(ui.ModelList->selectedItems()[i]);
        if(axis==0)
            inst->SetScale(QVector3D(x,y,z));
        else if(axis==1)
            inst->SetScale(QVector3D(x,inst->GetScale().y(),inst->GetScale().z()));
        else if(axis==2)
            inst->SetScale(QVector3D(inst->GetScale().x(),y,inst->GetScale().z()));
        else if(axis==3)
            inst->SetScale(QVector3D(inst->GetScale().x(),inst->GetScale().y(),z));
    }
}
void B9Layout::SetSelectionFlipped(bool flipped)
{
    unsigned int i;
    std::vector<B9ModelInstance*> instList = GetSelectedInstances();

    for(i=0; i < instList.size(); i++)
    {
        instList[i]->SetFlipped(flipped);
    }

}


void B9Layout::DropSelectionToFloor()
{
    unsigned int i;
	for(i = 0; i < GetSelectedInstances().size(); i++)
	{
		GetSelectedInstances()[i]->RestOnBuildSpace();
	}
    UpdateInterface();
}
void B9Layout::ResetSelectionRotation()
{
    unsigned int i;
    for(i = 0; i < GetSelectedInstances().size(); i++)
    {
        GetSelectedInstances()[i]->SetRot(QVector3D(0,0,0));
        GetSelectedInstances()[i]->UpdateBounds();
    }
    UpdateInterface();
}


void B9Layout::DuplicateSelection()
{
    unsigned int i;
    B9ModelInstance* inst;
    B9ModelInstance* newinst;
    B9ModelInstance* compareinst;
    B9SupportStructure* newSup;
	bool good = true;
	double x;
	double y;
	double xkeep;
	double ykeep;
    std::vector<B9ModelInstance*> sellist = GetSelectedInstances();
	for(i=0;i<sellist.size();i++)
	{
		inst = sellist[i];
		
		double xsize = inst->GetMaxBound().x() - inst->GetMinBound().x();
		double ysize = inst->GetMaxBound().y() - inst->GetMinBound().y();
		
        xkeep = 0;
        ykeep = 0;
		for(x = -project->GetBuildSpace().x()*0.5 + xsize/2; x <= project->GetBuildSpace().x()*0.5 - xsize/2; x += xsize + 1)
		{
			for(y = -project->GetBuildSpace().y()*0.5 + ysize/2; y <= project->GetBuildSpace().y()*0.5 - ysize/2; y += ysize + 1)
			{
				good = true;
                for(unsigned int d=0;d<ModelDataList.size();d++)
				{
                    for(unsigned int n=0;n<ModelDataList[d]->instList.size();n++)
					{
						compareinst = ModelDataList[d]->instList[n];

						if(((x - xsize/2) < compareinst->GetMaxBound().x()) && ((x + xsize/2) > compareinst->GetMinBound().x()) && ((y - ysize/2) < compareinst->GetMaxBound().y()) && ((y + ysize/2) > compareinst->GetMinBound().y()))
						{
							good = false;
						}
					}
				}
				if(good)
				{
					xkeep = x;
					ykeep = y;
				}
				
			}
		}
		
		newinst = inst->pData->AddInstance();
		newinst->SetPos(QVector3D(xkeep,ykeep,inst->GetPos().z()));
		newinst->SetRot(inst->GetRot());
		newinst->SetScale(inst->GetScale());
        newinst->SetFlipped(inst->GetFlipped());
        newinst->SetBounds(inst->GetMaxBound() + (newinst->GetPos() - inst->GetPos()),
                           inst->GetMinBound() + (newinst->GetPos() - inst->GetPos()));
        //dup supports
        for(i = 0; i < inst->GetSupports().size(); i++)
        {
            newSup = newinst->AddSupport();
            newSup->CopyFrom(inst->GetSupports()[i]);
            newSup->SetInstanceParent(newinst);

        }//and base plate
        if(inst->GetBasePlate())
        {
            newinst->EnableBasePlate();
            newinst->GetBasePlate()->CopyFrom(inst->GetBasePlate());
            newinst->GetBasePlate()->SetInstanceParent(newinst);
        }

		SelectOnly(newinst);
	}
}

//Upper level del action branching
void B9Layout::DeleteSelection()//delete whatever is selected - support or instance..
{
    if(SupportModeInst())
    {
        DeleteSelectedSupports();
    }
    else
    {
        DeleteSelectedInstances();
    }
}

void B9Layout::DeleteSelectedInstances()
{
    unsigned int i;
    std::vector<B9ModelInstance*> list = GetSelectedInstances();

    if(SupportModeInst())
        return;

	for(i=0;i < list.size();i++)
	{
		delete list[i];	
	}
	//cleanup any unnessecary modeldata
	CleanModelData();
    UpdateInterface();
	project->UpdateZSpace();
}



//Support Mode/////////////////////////////////
void B9Layout::SetSupportMode(bool tog)
{
    if(tog && (currInstanceInSupportMode == NULL))//go into support mode
    {
        qDebug() << "Entering Support Mode";
        //make sure weve selected somthing
        if(!GetSelectedInstances().size())
            return;


        ui.TabWidget->blockSignals(true);
            ui.TabWidget->setCurrentIndex(1);
        ui.TabWidget->blockSignals(false);



        //we can assume weve selected something...
        currInstanceInSupportMode = GetSelectedInstances()[0];

        //if the instance is on the ground, raise it so we dont get crunched supports.
        if(currInstanceInSupportMode->GetMinBound().z() < 0.01 && !currInstanceInSupportMode->GetSupports().size())
        {
            oldModelConstricted = true;
            currInstanceInSupportMode->Move(QVector3D(0,0,5));
        }else oldModelConstricted = false;

        currInstanceInSupportMode->SetInSupportMode(true);
        //bake the instance in a manner similar to slice preparation
        //but without support baking!
        currInstanceInSupportMode->BakeGeometry();
        currInstanceInSupportMode->SortBakedTriangles();
        currInstanceInSupportMode->AllocateTriContainers(0.1);
        currInstanceInSupportMode->FillTriContainers();
        currInstanceInSupportMode->FormTriPickDispLists();

        oldZoom = pWorldView->GetZoom();
        oldPan = pWorldView->GetPan();
        oldRot = pWorldView->GetRotation();
        oldTool = pWorldView->GetTool();

        pWorldView->SetRevolvePoint(currInstanceInSupportMode->GetPos());
        pWorldView->SetPan(QVector3D(0,0,0));
        pWorldView->SetZoom(100.0);


        //set tool to support addition
        SetToolSupportAdd();

        FillSupportList();//refresh the list gui.
    }
    else if(!tog && (currInstanceInSupportMode != NULL))
    {
        qDebug() << "Exiting Support Mode";

        ui.TabWidget->blockSignals(true);
            ui.TabWidget->setCurrentIndex(0);
        ui.TabWidget->blockSignals(false);

        if(currInstanceInSupportMode != NULL)
        {
            currInstanceInSupportMode->SetInSupportMode(false);
            currInstanceInSupportMode->FreeTriPickDispLists();
            currInstanceInSupportMode->UnBakeGeometry();
            currInstanceInSupportMode->FreeTriContainers();

            if(oldModelConstricted && !currInstanceInSupportMode->GetSupports().size())
                currInstanceInSupportMode->Move(QVector3D(0,0,-5));
            else
                currInstanceInSupportMode->SetPos(currInstanceInSupportMode->GetPos());//nudge to fix supports

            currInstanceInSupportMode = NULL;
        }
        pWorldView->SetRevolvePoint(QVector3D(0,0,0));
        pWorldView->SetPan(oldPan);
        pWorldView->SetZoom(oldZoom);
        SetTool(oldTool);

        //pWorldView->setXRotation(oldRot.x());
        //pWorldView->setYRotation(oldRot.y());
        //pWorldView->setZRotation(oldRot.z());

    }
    //if entering or leaving we always deselect all supports
    DeSelectAllSupports();

    UpdateInterface();
}

void B9Layout::FillSupportList()
{
    unsigned int s;
    ui.SupportList->clear();
    std::vector<B9SupportStructure*> supList;

    if(!SupportModeInst())
        return;

    supList = SupportModeInst()->GetSupports();


    ui.SupportList->blockSignals(true);
    for(s = 0; s < supList.size(); s++)
    {
        ui.SupportList->addItem("Support " + QString::number(supList[s]->SupportNumber));
    }
    ui.SupportList->blockSignals(false);
}

void B9Layout::UpdateSupportList()
{
    unsigned int i, j;
    QListWidgetItem* cItem = NULL;
    B9SupportStructure* cSup = NULL;
    std::vector<B9SupportStructure*>* selSups = GetSelectedSupports();


    ui.SupportList->blockSignals(true);
    for(i = 0; i < ui.SupportList->count(); i++)
    {
        cItem = ui.SupportList->item(i);

        cItem->setSelected(false);
        for(j = 0; j < selSups->size(); j++)
        {
            cSup = selSups->at(j);
            if(cItem->text().remove("Support ").toInt() == cSup->SupportNumber)
            {
                cItem->setSelected(true);
            }
        }
    }
    ui.SupportList->blockSignals(false);
}



B9SupportStructure* B9Layout::FindSupportByName(QString name)
{
    unsigned int s;
    unsigned int searchNum = name.remove("Support ").toInt();
    std::vector<B9SupportStructure*> supList;

    if(!currInstanceInSupportMode)
    {
        qDebug() << "WARNING: finding support out of support mode";
         return NULL;
    }

    supList = currInstanceInSupportMode->GetSupports();

    for(s = 0; s < supList.size(); s++)
    {
        if(searchNum == supList[s]->SupportNumber)
        {
            return supList[s];
        }
    }
    return NULL;
}


void B9Layout::RefreshSupportSelectionsFromList()
{
    int l;

    //we need to disable selection in add and delete mode
    if(pWorldView->GetTool() == "SUPPORTADD"
    || pWorldView->GetTool() == "SUPPORTDELETE")
    {
        //UpdateSupportList();
        //return;
        SetToolSupportModify();
        RefreshSupportSelectionsFromList();
        return;
    }

    for(l = 0; l < ui.SupportList->count(); l++)
    {
        B9SupportStructure* sup = FindSupportByName(ui.SupportList->item(l)->text());
        if(sup == NULL)
        {
            qDebug() << "Warning - RefreshSupportSelectionsFromList couldnt match with real support";
            return;
        }

        if(!ui.SupportList->item(l)->isSelected())
        {
            DeSelectSupport(sup);
        }
        else if(ui.SupportList->item(l)->isSelected())
        {
            SelectSupport(sup);
        }
    }
}



B9ModelInstance* B9Layout::SupportModeInst()
{
    return currInstanceInSupportMode;
}

void B9Layout::SelectSupport(B9SupportStructure* sup)
{
    //first see if the support is already selected, we dont want duplicates.
    int i;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {
        if(sup == currSelectedSupports[i])
            return;
    }

    //qDebug() << "Support: "<< sup << " added to selection";
    sup->SetSelected(true);
    currSelectedSupports.push_back(sup);

    UpdateInterface();


}

std::vector<B9SupportStructure*>* B9Layout::GetSelectedSupports()
{
    return &currSelectedSupports;
}

bool B9Layout::IsSupportSelected(B9SupportStructure* sup)
{
    unsigned int i;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {
        if(currSelectedSupports[i] == sup)
            return true;
    }

    return false;
}

void B9Layout::DeSelectSupport(B9SupportStructure* sup)
{
    unsigned int s;
    std::vector<B9SupportStructure*> keepers;

    for(s = 0; s < currSelectedSupports.size(); s++)
    {
        if(currSelectedSupports[s] == sup)
        {
            //qDebug() << "Support: " << sup << " removed from selection";
            sup->SetSelected(false);
        }
        else
            keepers.push_back(currSelectedSupports[s]);
    }
    currSelectedSupports.clear();
    currSelectedSupports = keepers;

    UpdateInterface();

}

void B9Layout::DeSelectAllSupports()
{
    //qDebug() << "De-Selecting All Supports, selection list size: " << currSelectedSupports.size();
    while(currSelectedSupports.size())
    {
        DeSelectSupport(currSelectedSupports[0]);
    }
    UpdateSupportList();

}


void B9Layout::DeleteSelectedSupports()//called from remove button.
{
    unsigned int s;
    if(!SupportModeInst())
        return;

    for(s = 0; s < currSelectedSupports.size(); s++)
    {
        SupportModeInst()->RemoveSupport(currSelectedSupports[s]);
    }
    currSelectedSupports.clear();

    FillSupportList();

    UpdateInterface();
}

void B9Layout::DeleteSupport(B9SupportStructure* pSup)
{
    if(SupportModeInst() != NULL)
    {
        if(IsSupportSelected(pSup))
            DeSelectSupport(pSup);

        SupportModeInst()->RemoveSupport(pSup);
    }

    FillSupportList();

    UpdateInterface();
}

void B9Layout::MakeSelectedSupportsVertical()
{
    unsigned int i;
    B9SupportStructure* pSup;

    if(SupportModeInst() == NULL)
        return;

    for(i = 0; i < currSelectedSupports.size(); i++)
    {
        pSup = currSelectedSupports[i];
        pSup->SetBottomPoint(QVector3D(pSup->GetTopPivot().x(),
                                       pSup->GetTopPivot().y(),
                                       pSup->GetBottomPoint().z()));
    }
}

void B9Layout::MakeSelectedSupportsStraight()
{
    unsigned int i;
    B9SupportStructure* pSup;
    QVector3D lenVec;
    QVector3D topNorm, bottomNorm;

    if(SupportModeInst() == NULL)
        return;


    for(i = 0; i < currSelectedSupports.size(); i++)
    {
        pSup = currSelectedSupports[i];

        lenVec = pSup->GetTopPoint() - pSup->GetBottomPoint();
        lenVec.normalize();

        topNorm = pSup->GetTopNormal();
        bottomNorm = pSup->GetBottomNormal();

        topNorm = lenVec;
        topNorm.normalize();

        bottomNorm = -lenVec;
        bottomNorm.normalize();


        pSup->SetTopNormal(topNorm);
        if(!pSup->GetIsGrounded())
            pSup->SetBottomNormal(bottomNorm);

        pSup->SetTopAngleFactor(1.0);
        if(!pSup->GetIsGrounded())
            pSup->SetBottomAngleFactor(1.0);

    }

    UpdateInterface();
}

//Support Properties changes
void B9Layout::OnSupport_Top_AttachType_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetTopAttachShape(ui.Support_Top_AttachType_comboBox->currentText());
    }

    if(updateInterface)
    UpdateInterface();

    if(pWorldView->GetTool() == "SUPPORTADD")
    {
    QSettings appSettings;
    appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.beginGroup("SUPPORT_TOP");
        appSettings.setValue("ATTACHSHAPE",ui.Support_Top_AttachType_comboBox->currentText());
        appSettings.endGroup();
    appSettings.endGroup();
    }

}
void B9Layout::OnSupport_Top_Radius_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetTopRadius(ui.Support_Top_Radius_lineEdit->text().toDouble());
    }

    if(updateInterface)
    UpdateInterface();

    if(pWorldView->GetTool() == "SUPPORTADD")
    {
    QSettings appSettings;
    appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.beginGroup("SUPPORT_TOP");
        appSettings.setValue("RADIUS",ui.Support_Top_Radius_lineEdit->text().toDouble());
        appSettings.endGroup();
    appSettings.endGroup();
    }
}
void B9Layout::OnSupport_Top_Length_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetTopLength(ui.Support_Top_Length_lineEdit->text().toDouble());
    }

    if(updateInterface)
    UpdateInterface();

    if(pWorldView->GetTool() == "SUPPORTADD")
    {
    QSettings appSettings;
    appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.beginGroup("SUPPORT_TOP");
        appSettings.setValue("LENGTH",ui.Support_Top_Length_lineEdit->text().toDouble());
        appSettings.endGroup();
    appSettings.endGroup();
    }
}
void B9Layout::OnSupport_Top_Penetration_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetTopPenetration(ui.Support_Top_Penetration_horizontalSlider->value()*0.01);
    }

    if(updateInterface)
    UpdateInterface();

    ui.Support_Top_Penetration_label->setText(QString::number(ui.Support_Top_Penetration_horizontalSlider->value()*0.01));

    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
            appSettings.beginGroup("SUPPORT_TOP");
            appSettings.setValue("PENETRATION",ui.Support_Top_Penetration_horizontalSlider->value()*0.01);
            appSettings.endGroup();
        appSettings.endGroup();
    }
}
void B9Layout::OnSupport_Top_AngleFactor_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetTopAngleFactor(ui.Support_Top_AngleFactor_horizontalSlider->value()*0.01);
    }

    if(updateInterface)
    UpdateInterface();

    ui.Support_Top_AngleFactor_label->setText(QString::number(ui.Support_Top_AngleFactor_horizontalSlider->value()*0.01));

    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
            appSettings.beginGroup("SUPPORT_TOP");
            appSettings.setValue("ANGLEFACTOR",ui.Support_Top_AngleFactor_horizontalSlider->value()*0.01);
            appSettings.endGroup();
        appSettings.endGroup();
    }
}
void B9Layout::OnSupport_Mid_AttachType_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetMidAttachShape(ui.Support_Mid_AttachType_comboBox->currentText());
    }

    if(updateInterface)
    UpdateInterface();

    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
            appSettings.beginGroup("SUPPORT_MID");
            appSettings.setValue("ATTACHSHAPE",ui.Support_Mid_AttachType_comboBox->currentText());
            appSettings.endGroup();
        appSettings.endGroup();
    }
}
void B9Layout::OnSupport_Mid_Radius_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {  selSup = currSelectedSupports[i];
       selSup->SetMidRadius(ui.Support_Mid_Radius_lineEdit->text().toDouble());
    }

    if(updateInterface)
    UpdateInterface();

    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
            appSettings.beginGroup("SUPPORT_MID");
            appSettings.setValue("RADIUS",ui.Support_Mid_Radius_lineEdit->text().toDouble());
            appSettings.endGroup();
        appSettings.endGroup();
    }
}
void B9Layout::OnSupport_Bottom_AttachType_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetBottomAttachShape(ui.Support_Bottom_AttachType_comboBox->currentText());
    }

    if(updateInterface)
    UpdateInterface();

    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.beginGroup("SUPPORT_BOTTOM_GROUNDED");
        appSettings.setValue("ATTACHSHAPE",ui.Support_Bottom_AttachType_comboBox->currentText());
        appSettings.endGroup();
        appSettings.endGroup();
    }
}
void B9Layout::OnSupport_Bottom_Radius_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetBottomRadius(ui.Support_Bottom_Radius_lineEdit->text().toDouble());
    }

    if(updateInterface)
    UpdateInterface();


    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
            appSettings.beginGroup("SUPPORT_BOTTOM_GROUNDED");
            appSettings.setValue("RADIUS",ui.Support_Bottom_Radius_lineEdit->text().toDouble());
            appSettings.endGroup();
        appSettings.endGroup();
    }
}
void B9Layout::OnSupport_Bottom_Length_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetBottomLength(ui.Support_Bottom_Length_lineEdit->text().toDouble());
    }

    if(updateInterface)
    UpdateInterface();


    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
            appSettings.beginGroup("SUPPORT_BOTTOM_GROUNDED");
                appSettings.setValue("LENGTH",ui.Support_Bottom_Length_lineEdit->text().toDouble());
            appSettings.endGroup();
        appSettings.endGroup();
    }
}
void B9Layout::OnSupport_Bottom_Penetration_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetBottomPenetration(ui.Support_Bottom_Penetration_horizontalSlider->value()*0.01);
    }

    if(updateInterface)
    UpdateInterface();

    ui.Support_Bottom_Penetration_label->setText(QString::number(ui.Support_Bottom_Penetration_horizontalSlider->value()*0.01));

    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
            appSettings.beginGroup("SUPPORT_BOTTOM_GROUNDED");
                appSettings.setValue("PENETRATION",ui.Support_Bottom_Penetration_horizontalSlider->value()*0.01);
            appSettings.endGroup();
        appSettings.endGroup();
    }
}
void B9Layout::OnSupport_Bottom_AngleFactor_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetBottomAngleFactor(ui.Support_Bottom_AngleFactor_horizontalSlider->value()*0.01);
    }

    if(updateInterface)
    UpdateInterface();

    ui.Support_Bottom_AngleFactor_label->setText(QString::number(ui.Support_Bottom_AngleFactor_horizontalSlider->value()*0.01));

    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
            appSettings.beginGroup("SUPPORT_BOTTOM_GROUNDED");
                appSettings.setValue("ANGLEFACTOR",ui.Support_Bottom_AngleFactor_horizontalSlider->value()*0.01);
            appSettings.endGroup();
        appSettings.endGroup();
    }
}




//Base Plate creation and destruction is handled here in addition to other base plate properties.
void B9Layout::OnBasePlatePropertiesChanged()
{
    B9SupportStructure* basePlate;
    int sliderPercent;
    double instRad;
    double actualRad;

    if(SupportModeInst() == NULL)
        return;


    basePlate = SupportModeInst()->GetBasePlate();

    //Creation/Destruction
    if(ui.Support_Base_GroupBox->isChecked() && (basePlate == NULL))
    {
        SupportModeInst()->EnableBasePlate();
    }

    if(!ui.Support_Base_GroupBox->isChecked() && (basePlate != NULL))
    {
        SupportModeInst()->DisableBasePlate();
    }

    //check again
    basePlate = SupportModeInst()->GetBasePlate();

    if(basePlate != NULL)
    {
        basePlate->SetBottomAttachShape(ui.Support_Base_AttachType_comboBox->currentText());
        sliderPercent = ui.Support_Base_Coverage_horizontalSlider->value();
        instRad = QVector2D(SupportModeInst()->GetMaxBound() - SupportModeInst()->GetMinBound()).length();

        instRad = instRad*0.5;
        actualRad = instRad*sliderPercent*0.01;
        basePlate->SetBottomRadius(actualRad);

        basePlate->SetBottomLength(ui.Support_Base_Length_lineEdit->text().toDouble());
    }

    UpdateInterface();
}




//called when selection changes primarily
void B9Layout::PushSupportProperties()
{
   B9SupportStructure* selSup;
   int indx;


   //do minimal gui updating in add mode
   if(!currSelectedSupports.size())
        return;

   selSup = currSelectedSupports[0];//TODO determine common properties and use those to fill
                                    //gui, instead of just the first support

   ui.Support_Top_Radius_lineEdit->setText(QString::number(selSup->GetTopRadius()));
   ui.Support_Mid_Radius_lineEdit->setText(QString::number(selSup->GetMidRadius()));
   ui.Support_Bottom_Radius_lineEdit->setText(QString::number(selSup->GetBottomRadius()));

   ui.Support_Top_Length_lineEdit->setText(QString::number(selSup->GetTopLength()));
   ui.Support_Mid_Length_lineEdit->setText(QString::number(selSup->GetMidLength()));
   ui.Support_Bottom_Length_lineEdit->setText(QString::number(selSup->GetBottomLength()));

   ui.Support_Top_AngleFactor_label->setText(QString::number(selSup->GetTopAngleFactor()*100.0).append("%"));
   ui.Support_Bottom_AngleFactor_label->setText(QString::number(selSup->GetBottomAngleFactor()*100.0).append("%"));

   ui.Support_Top_Penetration_label->setText(QString::number(selSup->GetTopPenetration()*100.0).append("um"));
   ui.Support_Bottom_Penetration_label->setText(QString::number(selSup->GetBottomPenetration()*100.0).append("um"));

   indx = ui.Support_Top_AttachType_comboBox->findText(selSup->GetTopAttachShape());
   ui.Support_Top_AttachType_comboBox->blockSignals(true);
    ui.Support_Top_AttachType_comboBox->setCurrentIndex(indx);
   ui.Support_Top_AttachType_comboBox->blockSignals(false);

   indx = ui.Support_Mid_AttachType_comboBox->findText(selSup->GetMidAttachShape());
   ui.Support_Mid_AttachType_comboBox->blockSignals(true);
    ui.Support_Mid_AttachType_comboBox->setCurrentIndex(indx);
   ui.Support_Mid_AttachType_comboBox->blockSignals(false);

   indx = ui.Support_Bottom_AttachType_comboBox->findText(selSup->GetBottomAttachShape());
   ui.Support_Bottom_AttachType_comboBox->blockSignals(true);
    ui.Support_Bottom_AttachType_comboBox->setCurrentIndex(indx);
   ui.Support_Bottom_AttachType_comboBox->blockSignals(false);

   //Angle Factor GUIs
   ui.Support_Top_AngleFactor_horizontalSlider->blockSignals(true);
    ui.Support_Top_AngleFactor_horizontalSlider->setValue(selSup->GetTopAngleFactor()*100.0);
   ui.Support_Top_AngleFactor_horizontalSlider->blockSignals(false);
   ui.Support_Bottom_AngleFactor_horizontalSlider->blockSignals(true);
    ui.Support_Bottom_AngleFactor_horizontalSlider->setValue(selSup->GetBottomAngleFactor()*100.0);
   ui.Support_Bottom_AngleFactor_horizontalSlider->blockSignals(false);

   //Penetration GUIs
   ui.Support_Top_Penetration_horizontalSlider->blockSignals(true);
    ui.Support_Top_Penetration_horizontalSlider->setValue(selSup->GetTopPenetration()*100.0);
   ui.Support_Top_Penetration_horizontalSlider->blockSignals(false);
   ui.Support_Bottom_Penetration_horizontalSlider->blockSignals(true);
    ui.Support_Bottom_Penetration_horizontalSlider->setValue(selSup->GetBottomPenetration()*100.0);
   ui.Support_Bottom_Penetration_horizontalSlider->blockSignals(false);

}

void B9Layout::PushBasePlateProperties()
{
    B9SupportStructure* basePlate = SupportModeInst()->GetBasePlate();
    int indx;

    if(basePlate == NULL)
    {  
        ui.Support_Base_GroupBox->setChecked(false);
        ui.Support_Base_Frame->hide();
    }
    else
    {
        ui.Support_Base_GroupBox->setChecked(true);
        ui.Support_Base_Frame->show();

        indx = ui.Support_Base_AttachType_comboBox->findText(basePlate->GetBottomAttachShape());
        ui.Support_Base_Coverage_label->setText(QString::number(ui.Support_Base_Coverage_horizontalSlider->value()) + QString("%"));
        ui.Support_Base_AttachType_comboBox->setCurrentIndex(indx);
        ui.Support_Base_Length_lineEdit->setText(QString::number(basePlate->GetBottomLength()));
    }
}


void B9Layout::ResetSupportLight()//connected to push button will always use hardcoded values!
{
    B9SupportStructure::FillRegistryDefaults(true,"LIGHT");
    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        FillSupportParamsWithDefaults();

        ui.Support_Reset_Heavy_button->setChecked(false);
        ui.Support_Reset_Medium_button->setChecked(false);
    }
    else if(pWorldView->GetTool() == "SUPPORTMODIFY")
    {
        FillSupportParamsWithDefaults();

        OnSupport_Top_AttachType_Changed(false);
        OnSupport_Top_Radius_Changed(false);
        OnSupport_Top_Length_Changed(false);
        OnSupport_Top_Penetration_Changed(false);
        OnSupport_Top_AngleFactor_Changed(false);
        OnSupport_Mid_AttachType_Changed(false);
        OnSupport_Mid_Radius_Changed(false);
        //TODO THERE IS A DIFFERENCT BETWEEN GROUNDED AND NON GROUNDED WITH GROuPS
        OnSupport_Bottom_Length_Changed(false);
    }

    UpdateInterface();
}
void B9Layout::ResetSupportMedium()//connected to push button will always use hardcoded values!
{
    B9SupportStructure::FillRegistryDefaults(true,"MEDIUM");
    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        FillSupportParamsWithDefaults();

        ui.Support_Reset_Heavy_button->setChecked(false);
        ui.Support_Reset_Light_button->setChecked(false);
    }
    else if(pWorldView->GetTool() == "SUPPORTMODIFY")
    {
        FillSupportParamsWithDefaults();

        OnSupport_Top_AttachType_Changed(false);
        OnSupport_Top_Radius_Changed(false);
        OnSupport_Top_Length_Changed(false);
        OnSupport_Top_Penetration_Changed(false);
        OnSupport_Top_AngleFactor_Changed(false);
        OnSupport_Mid_AttachType_Changed(false);
        OnSupport_Mid_Radius_Changed(false);
        //TODO THERE IS A DIFFERENCT BETWEEN GROUNDED AND NON GROUNDED WITH GROuPS
        OnSupport_Bottom_Length_Changed(false);
    }
    UpdateInterface();
}
void B9Layout::ResetSupportHeavy()//connected to push button will always use hardcoded values!
{
    B9SupportStructure::FillRegistryDefaults(true,"HEAVY");
    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        FillSupportParamsWithDefaults();

        ui.Support_Reset_Light_button->setChecked(false);
        ui.Support_Reset_Medium_button->setChecked(false);
    }
    else if(pWorldView->GetTool() == "SUPPORTMODIFY")
    {
        FillSupportParamsWithDefaults();

        OnSupport_Top_AttachType_Changed(false);
        OnSupport_Top_Radius_Changed(false);
        OnSupport_Top_Length_Changed(false);
        OnSupport_Top_Penetration_Changed(false);
        OnSupport_Top_AngleFactor_Changed(false);
        OnSupport_Mid_AttachType_Changed(false);
        OnSupport_Mid_Radius_Changed(false);
        //TODO THERE IS A DIFFERENCT BETWEEN GROUNDED AND NON GROUNDED WITH GROuPS
        OnSupport_Bottom_Length_Changed(false);
    }
    UpdateInterface();
}

//fill the support parameter box will default params.
void B9Layout::FillSupportParamsWithDefaults()
{
    int indx;

    QSettings appSettings;
    appSettings.beginGroup("USERSUPPORTPARAMS");
    appSettings.beginGroup("SUPPORT_TOP");
        indx = ui.Support_Top_AttachType_comboBox->findText(appSettings.value("ATTACHSHAPE").toString());
        ui.Support_Top_AttachType_comboBox->blockSignals(true);
            ui.Support_Top_AttachType_comboBox->setCurrentIndex(indx);
        ui.Support_Top_AttachType_comboBox->blockSignals(false);
        ui.Support_Top_AngleFactor_horizontalSlider->blockSignals(true);
            ui.Support_Top_AngleFactor_horizontalSlider->setValue(appSettings.value("ANGLEFACTOR").toDouble()*100);
        ui.Support_Top_AngleFactor_horizontalSlider->blockSignals(false);
        ui.Support_Top_AngleFactor_label->setText(appSettings.value("ANGLEFACTOR").toString());
        ui.Support_Top_Length_lineEdit->setText(appSettings.value("LENGTH").toString());
        ui.Support_Top_Penetration_horizontalSlider->blockSignals(true);
            ui.Support_Top_Penetration_horizontalSlider->setValue(appSettings.value("PENETRATION").toDouble()*100);
        ui.Support_Top_Penetration_horizontalSlider->blockSignals(false);
        ui.Support_Top_Penetration_label->setText(appSettings.value("PENETRATION").toString());
        ui.Support_Top_Radius_lineEdit->setText(appSettings.value("RADIUS").toString());
    appSettings.endGroup();

    appSettings.beginGroup("SUPPORT_MID");
        indx = ui.Support_Mid_AttachType_comboBox->findText(appSettings.value("ATTACHSHAPE").toString());
        ui.Support_Mid_AttachType_comboBox->blockSignals(true);
            ui.Support_Mid_AttachType_comboBox->setCurrentIndex(indx);
        ui.Support_Mid_AttachType_comboBox->blockSignals(false);
        ui.Support_Mid_Radius_lineEdit->setText(appSettings.value("RADIUS").toString());
    appSettings.endGroup();

    appSettings.beginGroup("SUPPORT_BOTTOM_GROUNDED");
        indx = ui.Support_Bottom_AttachType_comboBox->findText(appSettings.value("ATTACHSHAPE").toString());
        ui.Support_Bottom_AttachType_comboBox->blockSignals(true);
            ui.Support_Bottom_AttachType_comboBox->setCurrentIndex(indx);
        ui.Support_Bottom_AttachType_comboBox->blockSignals(false);
        ui.Support_Bottom_AngleFactor_horizontalSlider->blockSignals(true);
            ui.Support_Bottom_AngleFactor_horizontalSlider->setValue(appSettings.value("ANGLEFACTOR").toDouble()*100);
        ui.Support_Bottom_AngleFactor_horizontalSlider->blockSignals(false);
        ui.Support_Bottom_AngleFactor_label->setText(appSettings.value("ANGLEFACTOR").toString());
        ui.Support_Bottom_Length_lineEdit->setText(appSettings.value("LENGTH").toString());
        ui.Support_Bottom_Penetration_horizontalSlider->blockSignals(true);
            ui.Support_Bottom_Penetration_horizontalSlider->setValue(appSettings.value("PENETRATION").toDouble()*100);
        ui.Support_Bottom_Penetration_horizontalSlider->blockSignals(false);
        ui.Support_Bottom_Penetration_label->setText(appSettings.value("PENETRATION").toString());
        ui.Support_Bottom_Radius_lineEdit->setText(appSettings.value("RADIUS").toString());
    appSettings.endGroup();
    appSettings.endGroup();
}





//overall slicing routine, returns success.
bool B9Layout::SliceWorld()
{
    QSettings settings;

    QString filename = CROSS_OS_GetSaveFileName(this, tr("Export Slices"),
                                                settings.value("WorkingDir").toString() + "/" + ProjectData()->GetJobName(),
                                                tr("B9 Job (*.b9j);;SLC (*.slc)"));

    if(filename.isEmpty())//cancell button
	{
        return false;
	}

    QString Format = QFileInfo(filename).completeSuffix();
    settings.setValue("WorkingDir",QFileInfo(filename).absolutePath());

    if(Format.toLower() == "b9j")
	{
        if(SliceWorldToJob(filename))
        {
             QMessageBox::information(0,"Finished","Slicing Completed\n\nAll layers sliced and job file saved.");
             return true;
        }
        else
        {
            QMessageBox::information(0,"Canceled","Slicing Canceled\n\nnothing was saved.");
            return false;
        }
	}
    else if(Format.toLower() == "slc")
	{
        if(SliceWorldToSlc(filename))
        {
            QMessageBox::information(0,"Finished","Slicing Completed\n\nAll layers sliced and slc file saved.");
            return true;

        }
        else
        {
            QMessageBox::information(0,"Cancelled","Slicing Cancelled\n\nnothing was saved.");
            return false;
        }
	}

    return false;
}

//slicing to a job file!
bool B9Layout::SliceWorldToJob(QString filename)
{
    unsigned int m,i,l;
    unsigned int totalSliceOps = 0;
    unsigned int globalLayers;
    int nummodels = 0;
    B9ModelInstance* pInst;
	double zhieght = project->GetBuildSpace().z();
	double thickness = project->GetPixelThickness()*0.001;
    QString jobname = project->GetJobName();
    QString jobdesc = project->GetJobDescription();
	CrushedPrintJob* pMasterJob = NULL;
    Slice* pSlice;
    bool moreSlicesToCome;

	//calculate how many layers we need
    globalLayers = qCeil(zhieght/thickness);

	//calculate how many models there are
	for(m=0;m<ModelDataList.size();m++)
	{
		for(i=0;i<ModelDataList[m]->instList.size();i++)
		{
            pInst = ModelDataList[m]->instList[i];
            totalSliceOps += qCeil((pInst->GetMaxBound().z() - pInst->GetMinBound().z())/thickness);
		}
    }

    //make a loading bar
    LoadingBar progressbar(0, totalSliceOps);
    QObject::connect(&progressbar,SIGNAL(rejected()),this,SLOT(CancelSlicing()));
    progressbar.setDescription("Slicing to Job..");
    progressbar.setValue(0);
    QApplication::processEvents();


	//make a master job file for use later
	pMasterJob = new CrushedPrintJob();
    pMasterJob->setName(jobname);
    pMasterJob->setDescription(jobdesc);
	pMasterJob->setXYPixel(QString().number(project->GetPixelSize()/1000));
	pMasterJob->setZLayer(QString().number(project->GetPixelThickness()/1000));
    pMasterJob->clearAll(globalLayers);//fills the master job with the needed layers


    //FOR Each Model Instance
	for(m=0;m<ModelDataList.size();m++)
    {
		for(i=0;i<ModelDataList[m]->instList.size();i++)
		{
            B9ModelInstance* inst = ModelDataList[m]->instList[i];
            inst->PrepareForSlicing(thickness);

			//slice all layers and add to instance's job file
            for(l = 0; l < globalLayers; l++)
            {
                //if we are in the model's z - bounds
                if((double)l*thickness <= inst->GetMaxBound().z() && (double)l*thickness >= inst->GetMinBound().z()-0.5*thickness)
                {
                    inst->pSliceSet->QueNewSlice(l*thickness + thickness*0.5,l);
                }
            }
            if(nummodels == 1)
                inst->pSliceSet->SetSingleModelCompressHint(true);
            else
                inst->pSliceSet->SetSingleModelCompressHint(false);

            do
            {
                pSlice = inst->pSliceSet->ParallelCreateSlices(moreSlicesToCome,pMasterJob);
                if(pSlice != NULL)
                {
                    delete pSlice;
                    progressbar.setValue(progressbar.GetValue() + 1);
                    QApplication::processEvents();
                }

                if(cancelslicing)
                {
                    cancelslicing = false;
                    inst->FreeFromSlicing();
                    return false;
                }

            }while(moreSlicesToCome);



            inst->FreeFromSlicing();
		}
	}
	
    QFile* pf = new QFile(filename);

    pMasterJob->saveCPJ(pf);
    delete pf;
	delete pMasterJob;
	

	pWorldView->makeCurrent();

	cancelslicing = false;

    return true;
}

//slicing to a slc file!
bool B9Layout::SliceWorldToSlc(QString filename)
{
    unsigned int m;
    unsigned int i;
	int l;
	int numlayers;
	int nummodels = 0;

	double zhieght = project->GetBuildSpace().z();
	double thickness = project->GetPixelThickness()*0.001;

    Slice* currSlice = NULL;
    bool moreSlicesToCome;

	//calculate how many layers we need
	numlayers = qCeil(zhieght/thickness);
	//calculate how many models there are
	for(m=0;m<ModelDataList.size();m++)
	{
		for(i=0;i<ModelDataList[m]->instList.size();i++)
		{
			nummodels++;
		}
	}

    //make a loading bar
        LoadingBar progressbar(0, numlayers*nummodels);
        QObject::connect(&progressbar,SIGNAL(rejected()),this,SLOT(CancelSlicing()));
        progressbar.setDescription("Exporting SLC..");
        progressbar.setValue(0);
        QApplication::processEvents();

	
    //create an slc exporter
    SlcExporter slc(filename.toStdString());
    if(!slc.SuccessOpen())
    {
        QMessageBox msgBox;
        msgBox.setText("Unable To Open Slc File!");
        msgBox.exec();
    }
    //write the header
    slc.WriteHeader("heeeeelllllloooooo");
    slc.WriteReservedSpace();
    slc.WriteSampleTableSize(1);
    slc.WriteSampleTable(0.0,float(thickness),0.0f);



	//for each modelinstance
	for(m=0;m<ModelDataList.size();m++)
	{
		for(i=0;i<ModelDataList[m]->instList.size();i++)
		{
            B9ModelInstance* inst = ModelDataList[m]->instList[i];
            inst->PrepareForSlicing(thickness);

            //slice all layers and add to instance's job file
			for(l = 0; l < numlayers; l++)
			{
				//make sure we are in the model's z - bounds
				if(l*thickness <= inst->GetMaxBound().z() && l*thickness >= inst->GetMinBound().z())
				{
                    inst->pSliceSet->QueNewSlice(l*thickness + thickness*0.5,l);
                }
            }

            do
            {
                currSlice = inst->pSliceSet->ParallelCreateSlices(moreSlicesToCome,0);
                if(currSlice != NULL)
                {
                    progressbar.setValue(progressbar.GetValue() + 1);
                    QApplication::processEvents();
                    slc.WriteNewSlice(currSlice->realAltitude, currSlice->loopList.size());
                    currSlice->WriteToSlc(&slc);
                    delete currSlice;
                }


                if(cancelslicing)
                {
                    cancelslicing = false;
                    inst->FreeFromSlicing();
                    return false;
                }

            }while(moreSlicesToCome);



            inst->FreeFromSlicing();
		}
	}

    slc.WriteNewSlice(0.0,0xFFFFFFFF);
    return true;
}


void B9Layout::CancelSlicing()
{
	cancelslicing = true;
}


//exporting
void B9Layout::PromptExportToSTL()
{
    QString filename;
    QSettings settings;
    filename = CROSS_OS_GetSaveFileName(this, tr("Export To STL"),
                                        settings.value("WorkingDir").toString() + "/" + ProjectData()->GetFileName(),
                                        tr("stl (*.stl)"));

    if(filename.isEmpty())
        return;

    if(ExportToSTL(filename))
    {
        QMessageBox msgBox;
        msgBox.setText("Stl Export Complete");
        msgBox.exec();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Unable To Export stl File");
        msgBox.exec();
    }
}

 bool B9Layout::ExportToSTL(QString filename)
 {
     unsigned int i;
     unsigned long int t;
     B9ModelInstance* pInst = NULL;
     Triangle3D* pTri = NULL;
     bool fileOpened;

     B9ModelWriter exporter(filename, fileOpened);


     if(!fileOpened)
         return false;

     SetSupportMode(false);

     for(i = 0; i < GetAllInstances().size(); i++)
     {
         pInst = GetAllInstances().at(i);
         pInst->BakeGeometry(true);

         for(t = 0; t < pInst->triList.size(); t++)
         {
             pTri = pInst->triList[t];
             exporter.WriteNextTri(pTri);
         }

         pInst->UnBakeGeometry();
     }

     exporter.Finalize();



     return true;
 }








//////////////////////////////////////////////////////
//Private
//////////////////////////////////////////////////////


///////////////////////////////////////////////////
//Events
///////////////////////////////////////////////////
void B9Layout::keyPressEvent(QKeyEvent * event )
{
    if(event->key() == Qt::Key_Escape)
    {
        SetSupportMode(false);
    }

    pWorldView->keyPressEvent(event);

}
void B9Layout::keyReleaseEvent(QKeyEvent * event )
{
    pWorldView->keyReleaseEvent(event);

}
void B9Layout::mousePressEvent(QMouseEvent *event)
{
    //pWorldView->mousePressEvent(event);
    event->accept();
}
void B9Layout::mouseReleaseEvent(QMouseEvent *event)
{
    //pWorldView->mouseReleaseEvent(event);
    event->accept();
}

void B9Layout::hideEvent(QHideEvent *event)
{
    emit eventHiding();

    pWorldView->pDrawTimer->stop();


    event->accept();
}
void B9Layout::showEvent(QShowEvent *event)
{

    pWorldView->pDrawTimer->start();
    showMaximized();
    event->accept();
}

void B9Layout::closeEvent ( QCloseEvent * event )
{

    //if the layout is dirty - ask the user if they want to save.
    if(project->IsDirtied())
    {
        QMessageBox msgBox;
        msgBox.setText("The layout has been modified.");
         msgBox.setInformativeText("Do you want to save your changes?");
         msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
         msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();


        switch (ret)
        {
          case QMessageBox::Save:
                SaveAs();
              break;
          case QMessageBox::Discard:
                //nothing
              break;
          case QMessageBox::Cancel:
                event->ignore();
                return;
              break;
          default:
              break;
        }
    }

    //when we close the window - we want to make a new project.
    //because we might open the window again and we want a fresh start.
    New();
    event->accept();

}
void B9Layout::contextMenuEvent(QContextMenuEvent *event)
{
    /*
        QMenu menu(this);
        menu.addAction(ui.actionDelete);
        menu.addAction(ui.actionDrop_To_Floor);
        menu.addSeparator();
        menu.addAction(ui.actionMove);
        menu.addAction(ui.actionScale);

        menu.exec(event->globalPos());
    */
}
