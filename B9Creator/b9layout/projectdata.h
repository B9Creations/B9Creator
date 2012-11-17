#ifndef PROJECTDATA_H
#define PROJECTDATA_H
#include <QStringList>
#include <QVector3D>
#include <QVector2D>
#include <vector>
#include "b9layout.h"


class B9Layout;
class ProjectData : public QObject
{
	Q_OBJECT
public:
	ProjectData();
	~ProjectData();

	//File Access:
	void New();//Clears out internal project data, creates a new one with defualts
	bool Open(QString filepath); //returns success
	bool Save(QString filepath); //returns success

	//Structure Access:
	//Gets
	bool IsDirtied();
	QString GetFileName();//untitled if not saved yet
	QStringList GetModelFileList();
	QVector3D GetBuildSpace();
	double GetPixelSize();
    double GetPixelThickness();
	QVector2D GetResolution();

	//Sets
	void SetDirtied(bool dirt);
	void SetFileName(QString name);
	
	void SetBuildSpaceSize(QVector3D size);
	void SetPixelSize(double size);
    void SetPixelThickness(double thick);
	void SetResolution(QVector2D dim);


	void CalculateBuildArea();
	void UpdateZSpace();//calculates the size of the z box based on instance bounds
	
    B9Layout* pMain;
signals:
	void DirtChanged(bool dirt);
	void ProjectLoaded();

private:
	

	bool dirtied;
	QString filename;
	QStringList modelfilelist;
	QVector3D dimentions;
	QVector2D resolution;
	double xypixel;
	double zthick;
	QString jobfile;

	void SetDefaults();

};
#endif
