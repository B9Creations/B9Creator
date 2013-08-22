/*************************************************************************************
//
//  LICENSE INFORMATION
//
//  BCreator(tm)
//  Software for the control of the 3D Printer, "B9Creator"(tm)
//
//  Copyright 2011-2012 B9Creations, LLC
//  B9Creations(tm) and B9Creator(tm) are trademarks of B9Creations, LLC
//
//  This file is part of B9Creator
//
//    B9Creator is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    B9Creator is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with B9Creator .  If not, see <http://www.gnu.org/licenses/>.
//
//  The above copyright notice and this permission notice shall be
//    included in all copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
*************************************************************************************/

#include "slice.h"
#include "math.h"
#include "geometricfunctions.h"
#include <QtDebug>

#include <QtOpenGL>//for the open gl commands in render()...


Slice::Slice(double alt, int layerIndx)
{
	realAltitude = alt;
    this->layerIndx = layerIndx;
	numLoops = 0;
    inProccessing = false;
    pImg = NULL;
}
Slice::~Slice()
{
    unsigned int s;
	//delete the Segments via pointers
	for(s=0;s<segmentList.size();s++)
	{
		delete segmentList[s];
	}
	segmentList.clear();

    if(pImg != NULL)
        delete pImg;

}

void Slice::AddSegment(Segment* pSeg)
{
	segmentList.push_back(pSeg);
}

int Slice::GenerateSegments(B9ModelInstance* inputInstance)
{
    unsigned int t;
	int v1;
	int v2;
	int cmpcount = 0;//0 or 1 for knowing what point your trying to find.;

	QVector3D* thisvert = NULL;//local pointer to make quick access to vertices
	QVector3D* thatvert = NULL;

    std::vector<Triangle3D*>* rangedTriangles;

	double xdisp;
	double ydisp;
	double zdisp;
	double planefraction;

	int intersections = 0;


	//Triangle Splitting here:
    //Get the right container of triangles and use that as the list
    //(we used to use the triList which was O(n^2))
    rangedTriangles = inputInstance->GetTrianglesAroundZ(realAltitude);


    for(t = 0; t < rangedTriangles->size(); t++)//for each triangle in the model
	{
		//we want to create a temporary pointer to the currenct triangle
        Triangle3D* pTransTri = rangedTriangles->at(t);

		//test if the triangle intersects the XY plane of this slice!
		if(!pTransTri->IntersectsXYPlane(realAltitude))
		{
            continue;
		}
			
		intersections++;
		cmpcount = 0;
		
		//create 1 new segment object for the end result 
		Segment* seg1 = new Segment;
		QVector2D points[2];
		for(v1=0;v1<3;v1++)//for 1 or 2 triangle verts ABOVE the plane:
		{
			thisvert = &pTransTri->vertex[v1];
			if(thisvert->z() <= realAltitude)//we only want to compare FROM above the plane by convention (yes this means flush triangles below the plane)
			{
				continue;
			}
			for(v2=0; v2<3; v2++)//to every other triangle vert
			{
				if(v2 == v1)
				{continue;}

				thatvert = &pTransTri->vertex[v2];

				//are both points on the same side of plane?
				//if so we dont want to compare
				if((thatvert->z() > realAltitude))
				{
					continue;
				}
					
				cmpcount++;
				//common
				//displacments (final - initial)
				xdisp = thatvert->x() - thisvert->x();
				ydisp = thatvert->y() - thisvert->y();
				zdisp = thatvert->z() - thisvert->z();

                planefraction = (thisvert->z() - realAltitude)/fabs(zdisp);//0-1 fraction of where the plane is in relation to the z distance between the 2 verts.
				//(0 would be the plane is at the hieght of thisvert)

				points[cmpcount-1].setX(thisvert->x() + xdisp*planefraction);
				points[cmpcount-1].setY(thisvert->y() + ydisp*planefraction);
			}
		}
	
		//initiallize the segment.
		seg1->normal.setX(pTransTri->normal.x());
		seg1->normal.setY(pTransTri->normal.y());

		seg1->p1.setX(points[0].x());
		seg1->p1.setY(points[0].y());

		seg1->p2.setX(points[1].x());
		seg1->p2.setY(points[1].y());

		seg1->normal.normalize();

		seg1->CorrectPointOrder();//to match the normal convention!
			
		AddSegment(seg1);
	}
	return segmentList.size();
}

void Slice::SortSegmentsByX()
{
    std::sort(segmentList.begin(), segmentList.end(),Segment::lessthanX);
}

void Slice::ConnectSegmentNeighbors()
{
	unsigned int s;
    unsigned int potential;
    unsigned int s2;
	double potentialangle;
	double potentialDist;
	double minDist = 10000.0;

	Segment* thisSeg = NULL;
	Segment* thatSeg = NULL;
    QVector2D* thisPoint = NULL;

    QVector2D* thatPoint = NULL;

    std::vector<Segment*> sameXStripSegs;
    std::vector<Segment*> potentialLeadSegs;

    Segment* finalLeadSeg = NULL;

	for(s = 0; s < segmentList.size(); s++)//compare from every segment
	{
		thisSeg = segmentList[s];
		thisPoint = &thisSeg->p2;//compare from the 2nd point on "this" segment

		if(thisSeg->leadingSeg)//no need to add a connection if there already is one!
			continue;
		
		potentialLeadSegs.clear();//clear out the potentials list
		

        GetSegmentsAroundX(sameXStripSegs, thisPoint->x());


        //////////////////////////////////////
        for(s2 = 0; s2 < sameXStripSegs.size(); s2++)//to every other segment in ring
		{
			//make sure its not the same segment
			if(s == s2)
			{continue;}
			
            thatSeg = sameXStripSegs[s2];
			if(thatSeg->trailingSeg)//already connected to a trailing segment...
				continue;
			
			thatPoint = &thatSeg->p1;//to the first point of "that" segment
            if(IsZero(Distance2D(*thisPoint, *thatPoint),0.03))//they are close enough to each other
            {
				potentialLeadSegs.push_back(thatSeg);
            }
		}
        //////////////////////////////////////

		//sort through and pick from the potential pile
		//we want to pick a segment with the sharpest change in direction!
		//
		//
		//1>>>>>>>>>A>>>>>>>>2 1>>>>>>>>B>>>>>>>>>2
		//                    ^
		//             large delta angle/ Right Wieghted
		minDist = 100000.0;
		finalLeadSeg = NULL;
		potentialangle = 0.0;
		for(potential = 0; potential < potentialLeadSegs.size(); potential++)
		{
			thatSeg = potentialLeadSegs[potential];
			//potentialDot = (thisSeg->normal.x() * thatSeg->normal.x()) + (thisSeg->normal.y() * thatSeg->normal.y()); //gives a number indicating how sharp the angle is, -1 is the sharpest (-1,1)
			//potentialCross = (thisSeg->normal.x() * thatSeg->normal.y()) - (thisSeg->normal.y() * thatSeg->normal.x());//gives a number indicating a right or left turn. (uphill is positive), (downhill is negative) (-1,1)
			potentialDist = Distance2D(thisSeg->p2, thatSeg->p1);
			
			if(potentialDist < minDist)
			{
				minDist = potentialDist;
				finalLeadSeg = potentialLeadSegs[potential];
			}
			
		}
		if(finalLeadSeg)
		{
			thisSeg->leadingSeg = finalLeadSeg;
			finalLeadSeg->trailingSeg = thisSeg;

			thisSeg->p2 = finalLeadSeg->p1;
			thisSeg->FormNormal();
		}
	}
}



int Slice::GenerateLoops()
{
    unsigned int s;
    bool triangulateSuccess;
    int fillVoidIndicator;


	for(s=0; s<segmentList.size(); s++)//pick a segment, any segment
	{
		if(!segmentList[s]->pLoop)//only if it doesnt belong to a loop already
		{
			Loop newLoop(segmentList[s], this);
			if(newLoop.Grow() >= 3)//this filters out single segment danglers, and 1dimentional loops
			{
				loopList.push_back(newLoop);//add to the list of loops for this slice
				numLoops++;
			}
			else//failed loop
			{
				newLoop.Destroy();//segment's parent pointer is set back to null
				//these passed-up segments should not be claimed by a loop again.
				//the new loop will now fall out of scope.
			}
		}
	}


    //run through various filters in before determining fill or void
    for(unsigned int l = 0; l < loopList.size(); l++)//looplist.size keeps growing..
    {
        loopList[l].Simplify();//remove small segments
        loopList[l].CorrectDoubleBacks();
    }

    for(unsigned int l = 0; l < loopList.size(); l++)//looplist.size keeps growing..
    {
        loopList[l].AttemptSplitUp(this);//split figure eights.
    }

    for(unsigned int l = 0; l < loopList.size(); l++)
    {
        fillVoidIndicator = loopList[l].DetermineTypeBySides();


        loopList[l].formPolygon();

        triangulateSuccess = loopList[l].formTriStrip();
        if(!triangulateSuccess)
            qDebug() << "B9Loop: Warning Tesselator Memory Starved!";
    }


    return numLoops;
}


void Slice::Render()
{
    unsigned int l;

    glBlendFunc(GL_ONE,GL_ONE);

	//for each filled loop
	for(l=0;l<loopList.size();l++)
	{	
        if(loopList[l].isfill )
        {
            glColor3ub(1,0,0);
            loopList[l].RenderTriangles();
        }
        if(!loopList[l].isfill)
        {
            glColor3ub(0,1,0);
            loopList[l].RenderTriangles();
        }
    }
}
void Slice::RenderOutlines()
{
    unsigned int s;


    for(s = 0; s < segmentList.size();s++)
    {

        glBegin(GL_LINES);


        glVertex2d( segmentList[s]->p1.x(), segmentList[s]->p1.y());
        glVertex2d( segmentList[s]->p2.x(), segmentList[s]->p2.y());

        glEnd();
    }

}



void Slice::DebugRender(bool normals, bool connections, bool fills, bool outlines)
{
    unsigned int l;
    unsigned int s;

    glBlendFunc(GL_ONE,GL_ONE);
	//for each loop

	for(l=0;l<loopList.size();l++)
	{	
		if(fills)
		{
			if(loopList[l].isfill)
			{	
				glColor3b(100,0,0);//filled
			}
			else
			{
				glColor3b(0,100,0);//void
			}

			loopList[l].RenderTriangles();
		}

        for(s = 0; s < loopList[l].segListp.size();s++)
		{
			if(normals)
			{
				//debug normals
				glColor3f(0.0,0.0,1.0);
				glBegin(GL_LINES); 
				glVertex2d( (loopList[l].segListp[s]->p2.x() + loopList[l].segListp[s]->p1.x())/2.0, (loopList[l].segListp[s]->p2.y() + loopList[l].segListp[s]->p1.y())/2.0);
				glVertex2d( (loopList[l].segListp[s]->p2.x() + loopList[l].segListp[s]->p1.x())/2.0 + loopList[l].segListp[s]->normal.x(), (loopList[l].segListp[s]->p2.y() + loopList[l].segListp[s]->p1.y())/2.0 + loopList[l].segListp[s]->normal.y()); 
				glEnd();
				glColor3f(1.0,1.0,1.0);
			}
			if(connections)
			{
				//yellow line for showing actual linking
				if(loopList[l].segListp[s]->leadingSeg)
				{
					glColor3f(1.0,1.0,0.0);
					glBegin(GL_LINES); 
					glVertex2d( (loopList[l].segListp[s]->p2.x() + loopList[l].segListp[s]->p1.x())/2.0, (loopList[l].segListp[s]->p2.y() + loopList[l].segListp[s]->p1.y())/2.0);
					glVertex2d( (loopList[l].segListp[s]->leadingSeg->p2.x() + loopList[l].segListp[s]->leadingSeg->p1.x())/2.0, (loopList[l].segListp[s]->leadingSeg->p2.y() + loopList[l].segListp[s]->leadingSeg->p1.y())/2.0); 
					glEnd();
					glColor3f(1.0,1.0,1.0);
				}
			}
			if(outlines)
			{
				glColor3f(double(l)/loopList.size(),0,1);
				glBegin(GL_LINES); 
				glVertex2d( loopList[l].segListp[s]->p1.x(), loopList[l].segListp[s]->p1.y());
				glVertex2d( loopList[l].segListp[s]->p2.x(), loopList[l].segListp[s]->p2.y()); 
				glEnd();
			}
		}
	}





}





//export helpers
void Slice::WriteToSlc(SlcExporter* pslc)
{
    unsigned int l;
	int p;
	int polystripsize;
	for(l=0;l<loopList.size();l++)
	{
		polystripsize = loopList[l].polygonStrip.size();
		pslc->WriteBoundryHeader(polystripsize + 1,0);
		for(p = polystripsize - 1; p >= 0; p--)
		{
			pslc->WriteBoundryVert(loopList[l].polygonStrip[p].x(),loopList[l].polygonStrip[p].y());
		}
		//this last cord is so the last point equals the first poit by slc specifications.
		pslc->WriteBoundryVert(loopList[l].polygonStrip[polystripsize - 1].x(),loopList[l].polygonStrip[polystripsize-1].y());
	}
}



//private
bool Slice::TestIntersection(QVector2D &vec,Segment* seg1, Segment* seg2)
{
	bool intersection;
	//	return false;
	intersection= SegmentIntersection(vec, seg1->p1, seg1->p2, seg2->p1, seg2->p2);
	if(intersection)
	{
		if(IsZero(Distance2D(seg1->p2, seg2->p1),0.001) || IsZero(Distance2D(seg1->p1, seg2->p2),0.001))
			return false;
		if(IsZero(Distance2D(seg1->p1, seg2->p1),0.001) || IsZero(Distance2D(seg1->p2, seg2->p2),0.001))
			return false;

		return true;
	}
	return false;
}

//fills outList with segments only around the specified x cordinate
void Slice::GetSegmentsAroundX(std::vector<Segment*> &outList, double x)
{
    const double buffer = 0.04;//needs to be bigger than potentialSegs distance.
    unsigned long int mid,high,low;
    Segment* curSeg;
    outList.clear();

    high = segmentList.size()-1;
    low = 0;

    //binary search
    while (high >= low)
    {
        mid = (high + low)/2;
        curSeg = segmentList[mid];

        if (curSeg->p1.x() < (x - buffer))
            low = mid + 1;
        else if (curSeg->p1.x() > (x + buffer))
            high = mid - 1;
        else
        {   //were in the ball park
            while((mid > 0) && (segmentList[mid]->p1.x() > (x - buffer)))
            {
                mid--;
            }
            while((mid <= segmentList.size()-1) && segmentList[mid]->p1.x() < (x + buffer))
            {
                outList.push_back(segmentList[mid]);
                mid++;
            }
            return;
        }
    }

}
