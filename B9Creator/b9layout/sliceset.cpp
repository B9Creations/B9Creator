#include "sliceset.h"
#include "slice.h"
#include "math.h"

#include <QtDebug>


SliceSet::SliceSet(ModelInstance* pParentInstance)
{
	if(!pParentInstance)
	{
		qDebug() << "SliceSet Constructor - No Parent Clone Specified!";
	}
	pInstance = pParentInstance;
	pSliceData = NULL;
}
SliceSet::~SliceSet()
{
	pInstance = NULL;
	if(pSliceData != NULL)
	{
		delete pSliceData;
		pSliceData = NULL;
	}
}
bool SliceSet::GenerateSlice(double realAltitude)
{
	int segments;
	int loops;

	//destroy the old slice if there is one:
	if(pSliceData != NULL)
	{
		qDebug() << "old slice data deleted";
		delete pSliceData;
		pSliceData = NULL;
	}

	//make a new one:
	qDebug() << "SliceSet::GenerateSlice: Creating New Slice At Altitude: " << realAltitude;
	
	pSliceData = new Slice(realAltitude);

	segments = pSliceData->GenerateSegments(pInstance);//actually generate the segments inside the slice

	pSliceData->ConnectSegmentNeighbors();//connect adjacent segments

	loops = pSliceData->GenerateLoops();//generate loop structures

	qDebug() << "SliceSet::GenerateSlice: Segments: " << segments;
	qDebug() << "SliceSet::GenerateSlice: Loops: " << loops;

	return true;
}
