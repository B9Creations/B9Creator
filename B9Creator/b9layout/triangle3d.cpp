#include "triangle3d.h"
#include "utlilityfunctions.h"
#include <QDebug>

Triangle3D::Triangle3D()
{
	int i;
	normal*=0.0;//make the normal 0,0,0

	for(i=0; i < 3; i++)
	{
		vertex[i]*= 0.0;//make the vertex all reside on 0,0,0
	}

	maxBound.setX(-99999999.0);
	maxBound.setY(-99999999.0);
	maxBound.setZ(-99999999.0);

	minBound.setX(99999999.0);
	minBound.setY(99999999.0);
	minBound.setZ(99999999.0);
}

void Triangle3D::UpdateBounds()
{
	int i;
	
	//reset the bounds:
	maxBound.setX(-99999999.0);
	maxBound.setY(-99999999.0);
	maxBound.setZ(-99999999.0);

	minBound.setX(99999999.0);
	minBound.setY(99999999.0);
	minBound.setZ(99999999.0);

	for(i=0; i < 3; i++)
	{
		//max
		if(vertex[i].x() > maxBound.x())
		{
			maxBound.setX(vertex[i].x());
		}
		if(vertex[i].y() > maxBound.y())
		{
			maxBound.setY(vertex[i].y());
		}
		if(vertex[i].z() > maxBound.z())
		{
			maxBound.setZ(vertex[i].z());
		}

		//min
		if(vertex[i].x() < minBound.x())
		{
			minBound.setX(vertex[i].x());
		}
		if(vertex[i].y() < minBound.y())
		{
			minBound.setY(vertex[i].y());
		}
		if(vertex[i].z() < minBound.z())
		{
			minBound.setZ(vertex[i].z());
		}	
	}
}

bool Triangle3D::IsBad()
{
	double d = Distance3D(maxBound,minBound);
	if(IsZero(d,0.00001))//for possible double error.
	{	
		return true;
	}
	return false;
}

bool Triangle3D::IntersectsXYPlane(double realAltitude)
{
	if(IsBad() || ParallelXYPlane())
	{	
		return false;
	}
	if(maxBound.z() > realAltitude && minBound.z() <= realAltitude)
	{
		return true;
	}
	//the triangle is not in bounds at all:
	return false;
}

bool Triangle3D::ParallelXYPlane()
{
	if((vertex[0].z() == vertex[1].z())&&(vertex[0].z() == vertex[2].z()))
	{
		return true;
	}
	return false;
}