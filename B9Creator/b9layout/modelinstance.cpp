#include "modelinstance.h"
#include "utlilityfunctions.h"
#include <QtOpenGL>

unsigned char ModelInstance::gColorID[3] = {0,0,0};
QColor ModelInstance::selectedcolor = QColor(0,200,200);


///////////////////////////////////
//Public
//////////////////////////////////

ModelInstance::ModelInstance(ModelData* parent)
{
	pData = parent;
	pSliceSet = new SliceSet(this);
	listItem = new QListWidgetItem();
	parent->pMain->AddTagToModelList(listItem);


	pos = QVector3D(0,0,0);
	rot = QVector3D(0,0,0);
	scale = QVector3D(1,1,1);

	maxbound = parent->maxbound;
	minbound = parent->minbound;
	
	//selection
	isselected = false;

	//color
    normalcolor = QColor(125,125,150);
	currcolor = normalcolor;
	
	
	
	//picking
	pickcolor[0] = gColorID[0];
    pickcolor[1] = gColorID[1];
    pickcolor[2] = gColorID[2];
 
    gColorID[0]++;
    if(gColorID[0] >= 255)
    {
        gColorID[0] = 0;
        gColorID[1]++;
        if(gColorID[1] >= 255)
        {
                gColorID[1] = 0;
                gColorID[2]++;
        }
	}
	RestOnBuildSpace();
}
ModelInstance::~ModelInstance()
{
	//the instance removes itself from the parent's list
    unsigned int i;
	std::vector<ModelInstance*> temp;
	for(i=0;i<pData->instList.size();i++)
	{
		if(pData->instList[i] != this)
		{
			temp.push_back(pData->instList[i]);
		}
	}
	pData->instList.clear();
	pData->instList = temp;
	
	pData = NULL;
	delete listItem;
	delete pSliceSet;
	UnBakeGeometry();
}

void ModelInstance::SetTag(QString tag)
{
	listItem->setText(tag);
}

//translation
void ModelInstance::RestOnBuildSpace()
{
	SetZ(pos.z() - minbound.z());
}
void ModelInstance::SetPos(QVector3D pos)
{
	maxbound += pos - this->pos;
	minbound += pos - this->pos;
	this->pos = pos;

	pData->pMain->project->UpdateZSpace();
}
void ModelInstance::SetX(double x)
{
	SetPos(QVector3D(x,pos.y(),pos.z()));
}
void ModelInstance::SetY(double y)
{
	SetPos(QVector3D(pos.x(),y,pos.z()));
}
void ModelInstance::SetZ(double z)
{
    SetPos(QVector3D(pos.x(),pos.y(),z));
}

void ModelInstance::SetScale(QVector3D scale)
{
	this->scale = scale;
	CorrectScale();
}
void ModelInstance::SetScaleX(double x)
{
	SetScale(QVector3D(x,scale.y(),scale.z()));
}
void ModelInstance::SetScaleY(double y)
{
	SetScale(QVector3D(scale.x(),y,scale.z()));
}
void ModelInstance::SetScaleZ(double z)
{
	SetScale(QVector3D(scale.x(),scale.y(),z));
}

void ModelInstance::SetRot(QVector3D r)
{
	this->rot = r;
	CorrectRot();
}
	
//Incremental
void ModelInstance::Scale(QVector3D scalar)
{
	SetScale(QVector3D(scale.x() + scalar.x(),scale.y() + scalar.y(),scale.z() + scalar.z()));
}
void ModelInstance::Move(QVector3D translation)
{
	SetPos(QVector3D(pos.x() + translation.x(),pos.y() + translation.y(),pos.z() + translation.z()));
}
void ModelInstance::Rotate(QVector3D rotation)
{
	SetRot(QVector3D(rot.x() + rotation.x(),rot.y() + rotation.y(),rot.z() + rotation.z()));
}

//gets
QVector3D ModelInstance::GetPos()
{
	return pos;
}
QVector3D ModelInstance::GetRot()
{
	return rot;
}
QVector3D ModelInstance::GetScale()
{
	return scale;
}
QVector3D ModelInstance::GetMaxBound()
{
	return maxbound;
}
QVector3D ModelInstance::GetMinBound()
{
	return minbound;
}



//selection
void ModelInstance::SetSelected(bool sel)
{
	isselected = sel;
	
	if(sel)
	{
		currcolor = selectedcolor;
	}
	else
	{
		currcolor = normalcolor;
	}
	listItem->setSelected(sel);
}


//render
void ModelInstance::RenderGL()
{
	//do a smooth visual transition.
	visualcolor.setRedF(visualcolor.redF() + (currcolor.redF() - visualcolor.redF())/2.0);
	visualcolor.setGreenF(visualcolor.greenF() + (currcolor.greenF() - visualcolor.greenF())/2.0);
	visualcolor.setBlueF(visualcolor.blueF() + (currcolor.blueF() - visualcolor.blueF())/2.0);

	glPushMatrix();
		glColor3f(visualcolor.redF(),visualcolor.greenF(),visualcolor.blueF());
				glTranslatef(pos.x(),pos.y(),pos.z());
					
					glRotatef(rot.x(), 1.0, 0.0, 0.0);
					glRotatef(rot.y(), 0.0, 1.0, 0.0);
					glRotatef(rot.z(), 0.0, 0.0, 1.0);

						glScalef(scale.x(),scale.y(),scale.z());
							
							glCallList(pData->displaylistindx);
							
	glPopMatrix();
}
void ModelInstance::RenderPickGL()
{
	glPushMatrix();
		glColor3f(pickcolor[0]/255.0,pickcolor[1]/255.0,pickcolor[2]/255.0);
			glTranslatef(pos.x(),pos.y(),pos.z());
			
				glRotatef(rot.x(), 1.0, 0.0, 0.0);
				glRotatef(rot.y(), 0.0, 1.0, 0.0);
				glRotatef(rot.z(), 0.0, 0.0, 1.0);
					
					glScalef(scale.x(),scale.y(),scale.z());
						
						glCallList(pData->displaylistindx);
	glPopMatrix();
}


//geometry
void ModelInstance::BakeGeometry()
{
    unsigned int t;
    unsigned int v;



	UnBakeGeometry();
	
	//while we are at it, update the instances bounds as well
	maxbound = QVector3D(-999999.0,-999999.0,-999999.0);
	minbound = QVector3D(999999.0,999999.0,999999.0);

	//copy the triangles from pData into the list with transforms applied
	for(t = 0;t < pData->triList.size(); t++)
	{
		
		Triangle3D* pNewTri = new Triangle3D(pData->triList[t]);
		
		for(v=0;v<3;v++)
		{
			//scale first
			pNewTri->vertex[v] += (pData->triList[t].vertex[v]*(scale - QVector3D(1,1,1)));

			//Rotate second
			RotateVector(pNewTri->vertex[v],rot.z(),QVector3D(0,0,1));//z
			RotateVector(pNewTri->vertex[v],rot.y(),QVector3D(0,1,0));//y
			RotateVector(pNewTri->vertex[v],rot.x(),QVector3D(1,0,0));//x
			
			
			//Translate third
			pNewTri->vertex[v] += pos;
		}
		//rotate the normal as well!
		RotateVector(pNewTri->normal,rot.z(),QVector3D(0,0,1));
		RotateVector(pNewTri->normal,rot.y(),QVector3D(0,1,0));
		RotateVector(pNewTri->normal,rot.x(),QVector3D(1,0,0));
	
		//update the triangles bounds
		pNewTri->UpdateBounds();

		if(pNewTri->maxBound.x() > this->maxbound.x())
		{
			this->maxbound.setX(pNewTri->maxBound.x());
		}
		if(pNewTri->maxBound.y() > this->maxbound.y())
		{
			this->maxbound.setY(pNewTri->maxBound.y());
		}
		if(pNewTri->maxBound.z() > this->maxbound.z())
		{
			this->maxbound.setZ(pNewTri->maxBound.z());
		}
		
		if(pNewTri->minBound.x() < this->minbound.x())
		{
			this->minbound.setX(pNewTri->minBound.x());
		}
		if(pNewTri->minBound.y() < this->minbound.y())
		{
			this->minbound.setY(pNewTri->minBound.y());
		}
		if(pNewTri->minBound.z() < this->minbound.z())
		{
			this->minbound.setZ(pNewTri->minBound.z());
		}
		
		triList.push_back(pNewTri);
	}


}
void ModelInstance::UnBakeGeometry()
{
    for(unsigned int i=0;i<triList.size();i++)
	{
		delete triList[i];
	}
	triList.clear();
}
void ModelInstance::UpdateBounds()
{
	BakeGeometry();
	UnBakeGeometry();

	//tell the project to update the overal bounds!
	pData->pMain->project->UpdateZSpace();
	pData->pMain->UpdateTranslationInterface();
}

//////////////////////////////////////////////////////////
//Private
///////////////////////////////////////////////////////////

void ModelInstance::CorrectRot()
{
	while (rot.x() < 0)
        rot += QVector3D(360,0,0);
    while (rot.x() > 360 )
        rot -= QVector3D(360,0,0);

	while (rot.y() < 0)
        rot += QVector3D(0,360,0);
    while (rot.y() > 360 )
        rot -= QVector3D(0,360,0);

	while (rot.z() < 0)
        rot += QVector3D(0,0,360);
    while (rot.z() > 360 )
        rot -= QVector3D(0,0,360);
	
}
void ModelInstance::CorrectScale()
{

	if(scale.x() <= 0)
		scale.setX(0);

	if(scale.y() <= 0)
		scale.setY(0);

	if(scale.z() <= 0)
		scale.setZ(0);


}
