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

#include "triangle3d.h"
#include "geometricfunctions.h"
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

Triangle3D::~Triangle3D()
{

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
void Triangle3D::UpdateNormalFromGeom()
{
    normal = QVector3D::crossProduct(vertex[1] - vertex[0], vertex[2] - vertex[0]);
    normal.normalize();
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
