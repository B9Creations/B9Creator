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

#include "utlilityfunctions.h"
#include "math.h"
#include <algorithm>
#include "qmath.h"
#include "segment.h"
#include <QVector2D>
#include <QVector3D>

//Utility Function implementation
bool IsZero(double number, double tolerance)
{
	if(fabs(number) <= tolerance)
	{
		return true;
	}
	return false;
}

bool PointsShare(QVector2D point1, QVector2D point2, double tolerance)
{

	if(IsZero(point2.x() - point1.x(), tolerance) && IsZero(point2.y() - point1.y(),tolerance))
	{
		return true;
	}
	return false;
}
int PointLineCompare(QVector2D pointm, QVector2D dir, QVector2D quarrypoint)//returns 1 if point is on right, -1 if point is on left
{
	//double MAx = (quarrypoint.x() - pointm.x());
	//double MAy = (quarrypoint.y() - pointm.y());

	double position = (dir.x()*(quarrypoint.y() - pointm.y())) - (dir.y()*(quarrypoint.x() - pointm.x()));
	return -int(ceil(position));
}

bool SegmentIntersection(QVector2D &result, QVector2D seg11, QVector2D seg12, QVector2D seg21, QVector2D seg22)
{
	// Store the values for fast access and easy
	// equations-to-code conversion
	double x1 = seg11.x(), x2 = seg12.x(), x3 = seg21.x(), x4 = seg22.x();
	double y1 = seg11.y(), y2 = seg12.y(), y3 = seg21.y(), y4 = seg22.y();
 
	double d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	// If d is zero, there is no intersection
	if (d == 0) return NULL;

	// Get the x and y
	double pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
	double x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
	double y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
 
	// Check if the x and y coordinates are within both lines
	if ( x < std::min(x1, x2) || x > std::max(x1, x2) ||
	x < std::min(x3, x4) || x > std::max(x3, x4) ) return false;
	if ( y < std::min(y1, y2) || y > std::max(y1, y2) ||
	y < std::min(y3, y4) || y > std::max(y3, y4) ) return false;
 
	// Return the point of intersection
	result.setX(x);
	result.setY(y);
	return true;
}

bool SegmentsAffiliated(Segment* seg1, Segment* seg2, double epsilon)
{
	if(Distance2D(seg1->p2,seg2->p1) < epsilon || Distance2D(seg1->p1,seg2->p1) < epsilon || Distance2D(seg1->p2,seg2->p2) < epsilon || Distance2D(seg1->p1,seg2->p2) < epsilon)
			return true;

	return false;
}

double Distance2D(QVector2D point1, QVector2D point2)
{
	return sqrt( pow((point2.x()-point1.x()),2) + pow((point2.y()-point1.y()),2));
}

double Distance3D(QVector3D point1, QVector3D point2)
{
	return sqrt( pow((point2.x()-point1.x()),2) + pow((point2.y()-point1.y()),2) + pow((point2.z()-point1.z()),2));
}

void RotateVector(QVector3D &vec, double angledeg, QVector3D axis)//choose 1 axis of rotation at a time..
{
	double prevx;
	double prevy;
	double prevz;
	double cosval = qCos( angledeg * TO_RAD );
	double sinval = qSin( angledeg * TO_RAD );

	if(axis.x())
	{
		prevx = vec.x();
		prevy = vec.y();
		prevz = vec.z();
		vec.setY( prevy * cosval - prevz * sinval);
		vec.setZ( prevy * sinval + prevz * cosval);
	}

	if(axis.y())
	{
		prevx = vec.x();
		prevy = vec.y();
		prevz = vec.z();
		vec.setZ( prevz * cosval - prevx * sinval);
		vec.setX( prevz * sinval + prevx * cosval);
	}

	if(axis.z())
	{
		prevx = vec.x();
		prevy = vec.y();
		prevz = vec.z();
		vec.setX( prevx * cosval - prevy * sinval);
		vec.setY( prevx * sinval + prevy * cosval);
	}
}
