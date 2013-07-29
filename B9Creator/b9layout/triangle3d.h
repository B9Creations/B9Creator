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
    ~Triangle3D();

	void UpdateBounds();
    void UpdateNormalFromGeom();//recomputes normal from vertex's
	bool IsBad(); //returns true if the triangle has no "area" or no bounds.
	bool ParallelXYPlane();
	bool IntersectsXYPlane(double realAltitude);

    static bool GreaterTopAltitude(Triangle3D* t1, Triangle3D* t2)
    {
        return (t1->maxBound.z() < t2->maxBound.z());
    }

    static bool GreaterBottomAltitude(Triangle3D* t1, Triangle3D* t2)
    {
        return (t1->minBound.z() < t2->minBound.z());
    }
};

#endif
