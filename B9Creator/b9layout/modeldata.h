#ifndef MODELDATA_H
#define MODELDATA_H

#include <QString>
#include "b9layout.h"
#include "triangle3d.h"

#include "modelinstance.h"


#include <vector>

class aiScene;
class ModelData {
public:
    ModelData(B9Layout* main);
	~ModelData();
	
	QString GetFilePath();
	QString GetFileName();
	
	//data loading
	bool LoadIn(QString filepath); //returns success
	
	//instance
	ModelInstance* AddInstance();
	int loadedcount;


	//render
	unsigned int displaylistindx;

	//geometry
	std::vector<Triangle3D> triList;
	QVector3D maxbound;
	QVector3D minbound;

	std::vector<ModelInstance*> instList;
    B9Layout* pMain;
private:
	
	
	
	QString filepath;//physical filepath
	QString filename;//filename (larry.stl)

	//utility
	void CenterModel();//called by loadin to adjust the model to have a center at 0,0,0
	
	const aiScene* pScene;

	//render
	void FormDisplayList();
	
};
#endif
