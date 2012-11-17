#include "b9layout.h"
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
#include "slicedebugger.h"
#include "SlcExporter.h"


//////////////////////////////////////////////////////
//Public
//////////////////////////////////////////////////////
B9Layout::B9Layout(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags)
{
	ui.setupUi(this);

	//Initialize project data
    project = new ProjectData();
    project->pMain = this;
	

	//Create the worldview and attach it to the window
    pWorldView = new WorldView(NULL,this);

    ui.WorldViewContext->addWidget(pWorldView);
    pWorldView->show();
    pWorldView->pDrawTimer->stop();
	
    pslicedebugger = new SliceDebugger(this,this,Qt::Window);

	//slicing
	cancelslicing = false;


	//toolbar items
	ui.mainToolBar->addAction(ui.actionNew_Project);
	ui.mainToolBar->addAction(ui.actionOpen_Project);
	ui.mainToolBar->addAction(ui.actionSave);
	ui.mainToolBar->addSeparator();


	ui.mainToolBar->addAction(ui.actionSelection);
	ui.mainToolBar->addAction(ui.actionMove);
	ui.mainToolBar->addAction(ui.actionRotate);

	//connections:
    QObject::connect(project,SIGNAL(DirtChanged(bool)),this,SLOT(UpdateInterface()));
    QObject::connect(ui.actionCenter_View,SIGNAL(activated()),pWorldView,SLOT(CenterView()));
    QObject::connect(ui.ModelList,SIGNAL(itemSelectionChanged()),this,SLOT(RefreshSelectionsFromList()));
	

    UpdateTranslationInterface();
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
	delete pslicedebugger;
}


//////////////////////////////////////////////////////
//Public Slots
//////////////////////////////////////////////////////

//debug interface
void B9Layout::OpenDebugWindow()
{
	pslicedebugger->show();
	pslicedebugger->BakeTests();
}

//file
void B9Layout::New()
{
	project->New();
}
QString B9Layout::Open()
{
	bool success;

    QSettings settings;


	QString filename = QFileDialog::getOpenFileName(this,
             tr("Open Layout"), settings.value("LayoutDir").toString(), tr("B9Layout (*.B9L)"));

	if(filename.isEmpty())
    {return "";}


    RemoveAllInstances(); //remove instances.
	success = project->Open(filename);

	if(!success)
	{
        QMessageBox::warning(this, tr("B9Layout"), tr("Unable To Open Layout"),QMessageBox::Ok);
        return "";
	}

	CleanModelData();// now delete unneeded model data

    //set recent directory.
    settings.setValue("LayoutDir", QFileInfo(filename).absolutePath());

    return filename;

}
void B9Layout::Save()
{
	if(project->IsDirtied() || project->GetFileName() == "untitled")
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

    QString filename = QFileDialog::getSaveFileName(this, tr("Save Layout"),
                    settings.value("LayoutDir").toString(),
                            tr("B9 Layout (*.B9L)"));
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

    settings.setValue("LayoutDir",QFileInfo(filename).absolutePath());
}

//interface
void B9Layout::SetXYPixelSize(QString size)
{
	project->SetPixelSize(size.toDouble());
	project->CalculateBuildArea();
}
void B9Layout::SetZLayerThickness(QString thick)
{
	project->SetPixelThickness(thick.toDouble());
	project->CalculateBuildArea();
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
void B9Layout::SetZHeight(QString z)
{
	project->SetBuildSpaceSize(QVector3D(project->GetBuildSpace().x(),project->GetBuildSpace().x(), z.toDouble()));

}

//modeltranslation interface;
void B9Layout::UpdateTranslationInterface()
{

	if(ui.ModelList->selectedItems().size() <= 0 || ui.ModelList->selectedItems().size() > 1)//no items selected.
	{
		ui.ModelTranslationBox->setEnabled(false);
		ui.posx->clear();
		ui.posy->clear();
		ui.posz->clear();
		ui.rotx->clear();
		ui.roty->clear();
		ui.rotz->clear();
		ui.scalex->clear();
		ui.scaley->clear();
		ui.scalez->clear();
		ui.modelsizex->clear();
		ui.modelsizey->clear();
		ui.modelsizez->clear();
	}
	else
	{
		ui.ModelTranslationBox->setEnabled(true);
		ModelInstance* inst = FindInstance(ui.ModelList->selectedItems()[0]);
		ui.posx->setText(QString().number(inst->GetPos().x()));
		ui.posy->setText(QString().number(inst->GetPos().y()));
		ui.posz->setText(QString().number(inst->GetPos().z()));
		
		ui.rotx->setText(QString().number(inst->GetRot().x()));
		ui.roty->setText(QString().number(inst->GetRot().y()));
		ui.rotz->setText(QString().number(inst->GetRot().z()));

		ui.scalex->setText(QString().number(inst->GetScale().x()));
		ui.scaley->setText(QString().number(inst->GetScale().y()));
		ui.scalez->setText(QString().number(inst->GetScale().z()));

		ui.modelsizex->setText(QString().number(inst->GetMaxBound().x() - inst->GetMinBound().x()));
		ui.modelsizey->setText(QString().number(inst->GetMaxBound().y() - inst->GetMinBound().y()));
		ui.modelsizez->setText(QString().number(inst->GetMaxBound().z() - inst->GetMinBound().z()));
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

		ui.scaley->setText(scalex);
		ui.scalez->setText(scalex);
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
	SetSelectionRot(ui.rotx->text().toDouble(),0,0,1);
	SetSelectionRot(0,ui.roty->text().toDouble(),0,2);
	SetSelectionRot(0,0,ui.rotz->text().toDouble(),3);
	SetSelectionScale(scalex.toDouble(),0,0,1);
	SetSelectionScale(0,scaley.toDouble(),0,2);
	SetSelectionScale(0,0,scalez.toDouble(),3);

    for(unsigned int i = 0; i < GetSelectedInstances().size(); i++)
	{
		GetSelectedInstances()[i]->UpdateBounds();
	}
	UpdateTranslationInterface();
}

void B9Layout::LockScale(bool lock)
{
	if(lock)
	{
		SetSelectionScale(ui.scalex->text().toDouble(),0,0,1);
		SetSelectionScale(0,ui.scalex->text().toDouble(),0,2);
		SetSelectionScale(0,0,ui.scalex->text().toDouble(),3);
		UpdateTranslationInterface();
	}
}

//tools interface
void B9Layout::SetToolPointer()
{
	pWorldView->SetTool("pointer");
	ui.actionMove->setChecked(false);
	ui.actionRotate->setChecked(false);
}
void B9Layout::SetToolMove()
{
	pWorldView->SetTool("move");
	ui.actionSelection->setChecked(false);
	ui.actionRotate->setChecked(false);
}
void B9Layout::SetToolRotate()
{
	pWorldView->SetTool("rotate");
	ui.actionMove->setChecked(false);
	ui.actionSelection->setChecked(false);
}


//model
ModelInstance* B9Layout::AddModel(QString filepath)
{
    QSettings settings;

	if(filepath.isEmpty())
	{
		filepath = QFileDialog::getOpenFileName(this,
            tr("Open Model"), settings.value("AddModelDir").toString(), tr("Models (*.stl *.obj)"));

		//cancel button
		if(filepath.isEmpty())
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

	ModelData* pNewModel = new ModelData(this);
	
	bool success = pNewModel->LoadIn(filepath);
	if(!success)
	{
		delete pNewModel;
		return NULL;
	}

    //update registry
    settings.setValue("AddModelDir",QFileInfo(filepath).absolutePath());
	
	//add to the list
	ModelDataList.push_back(pNewModel);

	//make an Instance of the model!
	ModelInstance* pNewInst = pNewModel->AddInstance();
	project->UpdateZSpace();
	return pNewInst;
}
void B9Layout::RemoveAllInstances() //does not call cleanmodeldata!
{
    unsigned int m;
    unsigned int i;

	for(m=0;m<this->ModelDataList.size();m++)
	{
		ModelDataList[m]->loadedcount = 0;//also reset the index counter for instances!
		for(i=0;i<ModelDataList[m]->instList.size();i++)
		{
			delete ModelDataList[m]->instList[i];
		}
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
ModelInstance* B9Layout::FindInstance(QListWidgetItem* item)
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
		ModelInstance* inst = FindInstance(ui.ModelList->item(i));
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
void B9Layout::Select(ModelInstance* inst)
{
	qDebug() << inst << "added to selection";
	inst->SetSelected(true);
	UpdateTranslationInterface();
}
void B9Layout::DeSelect(ModelInstance* inst)
{
	qDebug() << inst << "removed from selection";
	inst->SetSelected(false);
	UpdateTranslationInterface();
}
void B9Layout::SelectOnly(ModelInstance* inst)
{
	DeSelectAll();
	Select(inst);
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
		ModelInstance* inst = FindInstance(ui.ModelList->selectedItems()[i]);
		if(axis==0)
		{
			inst->SetPos(QVector3D(x,y,z));
		}
		else if(axis==1)
		{
			inst->SetPos(QVector3D(x,inst->GetPos().y(),inst->GetPos().z()));
		}
		else if(axis==2)
		{
			inst->SetPos(QVector3D(inst->GetPos().x(),y,inst->GetPos().z()));
		}
		else if(axis==3)
		{
			inst->SetPos(QVector3D(inst->GetPos().x(),inst->GetPos().y(),z));
		}
	}
}
void B9Layout::SetSelectionRot(double x, double y, double z, int axis)
{
	int i;
	for(i=0;i<ui.ModelList->selectedItems().size();i++)
	{
		ModelInstance* inst = FindInstance(ui.ModelList->selectedItems()[i]);
		if(axis==0)
		{
			inst->SetRot(QVector3D(x,y,z));
		}
		else if(axis==1)
		{
			inst->SetRot(QVector3D(x,inst->GetRot().y(),inst->GetRot().z()));
		}
		else if(axis==2)
		{
			inst->SetRot(QVector3D(inst->GetRot().x(),y,inst->GetRot().z()));
		}
		else if(axis==3)
		{
			inst->SetRot(QVector3D(inst->GetRot().x(),inst->GetRot().y(),z));
		}
	}
}
void B9Layout::SetSelectionScale(double x, double y, double z, int axis)
{
	int i;
	for(i=0;i<ui.ModelList->selectedItems().size();i++)
	{
		ModelInstance* inst = FindInstance(ui.ModelList->selectedItems()[i]);
		if(axis==0)
		{
			inst->SetScale(QVector3D(x,y,z));
		}
		else if(axis==1)
		{
			inst->SetScale(QVector3D(x,inst->GetScale().y(),inst->GetScale().z()));
		}
		else if(axis==2)
		{
			inst->SetScale(QVector3D(inst->GetScale().x(),y,inst->GetScale().z()));
		}
		else if(axis==3)
		{
			inst->SetScale(QVector3D(inst->GetScale().x(),inst->GetScale().y(),z));
		}
	}
}
void B9Layout::DropSelectionToFloor()
{
    unsigned int i;
	for(i = 0; i < GetSelectedInstances().size(); i++)
	{
		GetSelectedInstances()[i]->RestOnBuildSpace();
	}
}
void B9Layout::DuplicateSelection()
{
    unsigned int i;
	ModelInstance* inst;
	ModelInstance* newinst;
	ModelInstance* compareinst;
	bool good = true;
	double x;
	double y;
	double xkeep;
	double ykeep;
	std::vector<ModelInstance*> sellist = GetSelectedInstances();
	for(i=0;i<sellist.size();i++)
	{
		inst = sellist[i];
		
		double xsize = inst->GetMaxBound().x() - inst->GetMinBound().x();
		double ysize = inst->GetMaxBound().y() - inst->GetMinBound().y();
		
		xkeep = 0;//inst->GetPos().x();
		ykeep = 0;//inst->GetPos().y();
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
		newinst->UpdateBounds();
		SelectOnly(newinst);
		
		
		
	}
}
std::vector<ModelInstance*> B9Layout::GetSelectedInstances()
{
	std::vector<ModelInstance*> insts;
	int i;
	for(i=0;i<ui.ModelList->selectedItems().size();i++)
	{
		insts.push_back(FindInstance(ui.ModelList->selectedItems()[i]));
	}
	return insts;
}
void B9Layout::DeleteSelectedInstances()
{
    unsigned int i;
	std::vector<ModelInstance*> list = GetSelectedInstances();
	for(i=0;i < list.size();i++)
	{
		delete list[i];	
	}
	//cleanup any unnessecary modeldata
	CleanModelData();
	UpdateTranslationInterface();
	project->UpdateZSpace();
}



//printing..





void B9Layout::SliceWorld()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Export Slices"), "/home/jana/untitled.b9j", tr("B9 Job (*.b9j);;SLC (*.slc)"));
	if(filename.isEmpty())
	{
		return;
	}
	QString Format = QFileInfo(filename).completeSuffix();
	if(Format == "b9j")
	{
		SliceWorldToJob(filename);
	}
	else if(Format == "slc")
	{
		SliceWorldToSlc(filename);
	}
	else
	{
		return;
	}


}

//slicing to a job file!
void B9Layout::SliceWorldToJob(QString filename)
{
    unsigned int m;
    unsigned int i;
    unsigned int l;
    unsigned int numlayers;
	int nummodels = 0;
	double zhieght = project->GetBuildSpace().z();
	double thickness = project->GetPixelThickness()*0.001;
	int xsize = project->GetResolution().x();
	int ysize = project->GetResolution().y();
	int x;
	int y;
	QRgb pickedcolor;
	QRgb mastercolor;
	QPixmap pix;
	QImage img(xsize,ysize, QImage::Format_ARGB32);
	QImage imgfrommaster(xsize,ysize, QImage::Format_ARGB32);
	CrushedPrintJob* pMasterJob = NULL;


	
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
	LoadingBar progressbar(0, numlayers, this);
	QObject::connect(&progressbar,SIGNAL(rejected()),this,SLOT(CancelSlicing()));
	progressbar.setDescription("Processing World..");
	progressbar.setValue(0);
	QApplication::processEvents();


	SliceContext paintwidget(NULL, this);
	paintwidget.makeCurrent();

	//make a master job file for use later
	pMasterJob = new CrushedPrintJob();
	pMasterJob->setName(QFileInfo(filename).baseName());
	pMasterJob->setDescription("Exported by B93D");
	pMasterJob->setXYPixel(QString().number(project->GetPixelSize()/1000));
	pMasterJob->setZLayer(QString().number(project->GetPixelThickness()/1000));
	
	img.fill(Qt::black);
	for(l=0;l<numlayers;l++)
	{
		pMasterJob->addImage(&img);
		progressbar.setValue(l);
		QApplication::processEvents();
		if(cancelslicing)
		{
			delete pMasterJob;
			pWorldView->makeCurrent();
			cancelslicing = false;
			return;
		}
	}
	
	progressbar.setDescription("Slicing World..");
	progressbar.setMax(numlayers*nummodels);
	progressbar.setValue(0);
	//for each modelinstance
	for(m=0;m<ModelDataList.size();m++)
	{
		for(i=0;i<ModelDataList[m]->instList.size();i++)
		{
			ModelInstance* inst = ModelDataList[m]->instList[i];
			inst->BakeGeometry();
			
			//slice all layers and add to instance's job file
			for(l = 0; l < numlayers; l++)
			{
				//make sure we are in the model's z - bounds
				if(l*thickness <= inst->GetMaxBound().z() && l*thickness >= inst->GetMinBound().z())
				{	
					//ACTUALLY Generate the Slice.
					inst->pSliceSet->GenerateSlice(l*thickness + thickness*0.5);
					paintwidget.SetSlice(inst->pSliceSet->pSliceData);
					
					
					pix = paintwidget.renderPixmap(xsize,ysize);
					img = pix.toImage();
					
				
					for(x = 0; x < xsize; x++)
					{
						for(y = 0; y < ysize; y++)
						{
							pickedcolor = img.pixel(x,y);
							if(qRed(pickedcolor) || qGreen(pickedcolor))
							{
								int result = qRed(pickedcolor) - qGreen(pickedcolor);
								if(result > 0)
								{
									result = 255;
								}
								img.setPixel(x,y,QColor(result,0,0).rgb());
							}
						}
					}
					
					
					QApplication::processEvents();


					pMasterJob->setCurrentSlice(l);
					imgfrommaster.fill(Qt::black);
					pMasterJob->inflateCurrentSlice(&imgfrommaster);
					for(x = 0; x < xsize; x++)
					{
						for(y = 0; y < ysize; y++)
						{
							pickedcolor = img.pixel(x,y);
							mastercolor = imgfrommaster.pixel(x,y); 
							if(qRed(pickedcolor) || qRed(mastercolor))
							{
								imgfrommaster.setPixel(x,y,QColor(255,255,255).rgb());
							}
						}
					}
					pMasterJob->crushCurrentSlice(&imgfrommaster);
					QApplication::processEvents();

					//update progress bar
					progressbar.setValue(progressbar.GetValue() + 1);
				
					if(cancelslicing)
					{
						cancelslicing = false;
						delete pMasterJob;
						pWorldView->makeCurrent();
						inst->UnBakeGeometry();
						return;
					}
				}
			}
			inst->UnBakeGeometry();
		}
	}
	
    QFile* pf = new QFile(filename);

    pMasterJob->saveCPJ(pf);
    delete pf;
	delete pMasterJob;
	

	pWorldView->makeCurrent();

	cancelslicing = false;
}

//slicing to a slc file!
void B9Layout::SliceWorldToSlc(QString filename)
{
    unsigned int m;
    unsigned int i;
	int l;
	int numlayers;
	int nummodels = 0;

	double zhieght = project->GetBuildSpace().z();
	double thickness = project->GetPixelThickness()*0.001;

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
	LoadingBar progressbar(0, numlayers*nummodels, this);
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
			ModelInstance* inst = ModelDataList[m]->instList[i];
			inst->BakeGeometry();
			//slice all layers and add to instance's job file
			for(l = 0; l < numlayers; l++)
			{
				//make sure we are in the model's z - bounds
				if(l*thickness <= inst->GetMaxBound().z() && l*thickness >= inst->GetMinBound().z())
				{
					
					//ACTUALLY Generate the Slice.
					inst->pSliceSet->GenerateSlice(l*thickness + thickness*0.5);
					slc.WriteNewSlice(l*thickness + thickness*0.5,inst->pSliceSet->pSliceData->loopList.size());
					inst->pSliceSet->pSliceData->WriteToSlc(&slc);
				}

				progressbar.setValue(progressbar.GetValue() + 1);
				QApplication::processEvents();
				if(cancelslicing)
				{
						cancelslicing = false;
						inst->UnBakeGeometry();
						return;
				}

			}
			inst->UnBakeGeometry();
		}
	}

	slc.WriteNewSlice(0.0,0xFFFFFFFF);
	//slc falls out of scope (automatically closes the file.)
}



void B9Layout::CancelSlicing()
{
	cancelslicing = true;
}




//////////////////////////////////////////////////////
//Private
//////////////////////////////////////////////////////



///////////////////////////////////////////////////
//Events
///////////////////////////////////////////////////
void B9Layout::keyPressEvent(QKeyEvent * event )
{
}
void B9Layout::keyReleaseEvent(QKeyEvent * event )
{
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

    event->accept();
}
