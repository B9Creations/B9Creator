#ifndef SLICE_H
#define SLICE_H

#include "modelinstance.h"
#include "segment.h"
#include "loop.h"
#include "SlcExporter.h"
#include <vector>

class Triangle3D;
class Loop;

class Slice
{

public:
	
	Slice();
	Slice(double alt);
	~Slice();

	void AddSegment(Segment* pSeg);
	
	int GenerateSegments(ModelInstance* inputInstance);//returns number of initial segments
		
	void ConnectSegmentNeighbors(); //returns the number of nudges
	
	int GenerateLoops();

	void Render();//OpenGL rendering code - renders the whole slice.
	void DebugRender(bool normals = true, bool connections = true, bool fills = true, bool outlines = true);//renders with visible debug information



	//export helpers
	void WriteToSlc(SlcExporter* pslc);





	std::vector<Segment*> segmentList;//list of segments
	
	std::vector<Loop> loopList;
	int numLoops;

	double realAltitude;//in mm;

private:
	bool TestIntersection(QVector2D &vec,Segment* seg1, Segment* seg2);
};


#endif