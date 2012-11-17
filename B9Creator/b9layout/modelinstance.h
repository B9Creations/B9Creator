#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H
#include <QVector3D>
#include <QColor>
#include <QListWidget>
#include "triangle3d.h"
#include "modeldata.h"
#include "sliceset.h"
class ModelData;
class Triangle3D;
class SliceSet;
class ModelInstance
{
public:
	ModelInstance(ModelData* parent);
	~ModelInstance();

	
	//translation
	void RestOnBuildSpace(); //rests the model as low as possible on the print area.
	void SetPos(QVector3D pos);
	void SetX(double x);
	void SetY(double y);
	void SetZ(double z);

	void SetScale(QVector3D scale);
	void SetScaleX(double x);
	void SetScaleY(double y);
	void SetScaleZ(double z);

	void SetRot(QVector3D r);

	//Incremental
	void Scale(QVector3D scalar);
	void Move(QVector3D translation);
	void Rotate(QVector3D rotation);

	//gets
	QVector3D GetPos();
	QVector3D GetRot();
	QVector3D GetScale();
	QVector3D GetMaxBound();
	QVector3D GetMinBound();

	//selection
	void SetTag(QString tag);
	void SetSelected(bool sel);

	
	//render
	void RenderGL();//renders the instance using the modeldata's displaylist with this instances transforms applied
	void RenderPickGL();//simple flat rendering with color id


	//geometry
	void BakeGeometry();//copies triangle data from the model data with applied transforms, also calculates bounds
	void UnBakeGeometry();//frees up the triangle data of this model.
	void UpdateBounds();//updates the bounds, this is somewhat time consuming!
	std::vector<Triangle3D*> triList;


	QListWidgetItem* listItem;
	ModelData* pData;
	SliceSet* pSliceSet;//slice set
	unsigned char pickcolor[3];//this instances pick color!
private:

	//translation
	QVector3D pos;
	QVector3D rot;
	QVector3D scale;
	QVector3D maxbound;
	QVector3D minbound;

	void CorrectRot();//puts rotation in 0-360 form
	void CorrectScale();//does not allow 0 or negative values

	//selection
	static unsigned char gColorID[3];
	bool isselected;
	static QColor selectedcolor;
	QColor normalcolor;
	QColor currcolor;
	QColor visualcolor;//actual rendering color - is always smoothely running to match currcolor.
};
#endif