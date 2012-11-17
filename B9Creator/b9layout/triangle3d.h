#ifndef TRIANGLE3D_H
#define TRIANGLE3D_H

#include <QVector3D>

class Triangle3D
{
public:
	QVector3D normal;
	QVector3D vertex[3];

	//specs
	QVector3D maxBound;
	QVector3D minBound;

	Triangle3D();

	void UpdateBounds();
	bool IsBad(); //returns true if the triangle has no "area" or no bounds.
	bool ParallelXYPlane();
	bool IntersectsXYPlane(double realAltitude);
};

#endif