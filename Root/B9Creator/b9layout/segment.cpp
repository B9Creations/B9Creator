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

#include "segment.h"
#include "math.h"
#include "qmath.h"
#include "geometricfunctions.h"

#include <QVector2D>


Segment::Segment()
{
	p1.setX(0.0);
	p1.setY(0.0);

	p2.setX(0.0);
	p2.setY(0.0);

	normal.setX(0.0);
	normal.setY(0.0);

	trailingSeg = NULL;
	leadingSeg = NULL;

	pLoop = NULL;

	chucked = false;
}

Segment::Segment(QVector2D point1, QVector2D point2)
{
	p1.setX(point1.x());
	p1.setY(point1.y());

	p2.setX(point2.x());
	p2.setY(point2.y());

	FormNormal();

	trailingSeg = NULL;
	leadingSeg = NULL;

	pLoop = NULL;

	chucked = false;

}

void Segment::FormNormal()
{
	double dx = p2.x() - p1.x();
	double dy = p2.y() - p1.y();

	//double theta = atan2(dy,dx);
	normal.setX(-dy);
	normal.setY(dx);

	normal.normalize(); //unit vector
}



bool Segment::CorrectPointOrder()
{
	double p2x;
	double p2y;

	QVector2D center((p2.x() + p1.x())/2.0,(p2.y() + p1.y())/2.0);
	int side = PointLineCompare(center,normal, p1);//returns 1 if point is on right, -1 if point is on left
	
	if(side < 0)
	{
		return 0; //we want the point on the left
	}
	else
	{
		//swap point data!
		p2x = p2.x();
		p2y = p2.y();

		p2.setX(p1.x());
		p2.setY(p1.y());

		p1.setX(p2x);
		p1.setY(p2y);

		return 1;
	}


}
