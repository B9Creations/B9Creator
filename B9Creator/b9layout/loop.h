#ifndef LOOP_H
#define LOOP_H

#include <vector>
#include "triangulate.h"

class Segment;
class Slice;

class Loop
{
public:
	Loop(Segment* startingSeg, Slice* parentSlice);
	~Loop();
	Slice* pSlice;

	std::vector<Segment*> segListp;
	Segment* pOriginSeg;//the segment that the loop starts with
	int numSegs;
	
	
	Vector2dVector polygonStrip;
	Vector2dVector triangleStrip;//ordered list of vertexes to be rendered, created by the triangulator

	bool isfill;

	//Debug Flags
	bool isComplete;//indicates if the loop is sealed off completely.
	bool isPatched;
	double totalAngle;//total angular change around the loop
	

	void ResetOrigin();
	
	int Grow();
	
	void AttachSegment(Segment* seg);

	bool SealHole(Segment* pLastSeg);
	
	void Simplify();//removes the smallest segment from the loop and seals up the gap.
	
	bool ThrowoutSmallestSegment();

	int NudgeSharedPoints();//finds "self touching" segment points and nudges them out of the way.
	
	int DetermineType();//determines if the loop is a fill or void;

	void formPolygon();
	
	bool formTriangleStrip();

	void Destroy();//dissacosiates all segments from loop.

	void RenderTriangles();//opengl rendering code

	int CorrectDoubleBacks();

	bool AttemptSplitUp(Slice* pslice);

private:

};
#endif