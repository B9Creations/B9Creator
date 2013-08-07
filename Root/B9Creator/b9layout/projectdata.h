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

#ifndef PROJECTDATA_H
#define PROJECTDATA_H
#include <QStringList>
#include <QVector3D>
#include <QVector2D>
#include <vector>
#include "b9layout.h"

#define LAYOUT_FILE_VERSION 14

class B9Layout;
class ProjectData : public QObject
{
	Q_OBJECT
public:
	ProjectData();
	~ProjectData();

	//File Access:
	void New();//Clears out internal project data, creates a new one with defualts
	bool Open(QString filepath); //returns success
	bool Save(QString filepath); //returns success

	//Structure Access:
	//Gets
	bool IsDirtied();
	QString GetFileName();//untitled if not saved yet
	QStringList GetModelFileList();
	QVector3D GetBuildSpace();
	double GetPixelSize();
    double GetPixelThickness();
	QVector2D GetResolution();
    QString GetJobName();
    QString GetJobDescription();

	//Sets
	void SetDirtied(bool dirt);

	void SetBuildSpaceSize(QVector3D size);
	void SetPixelSize(double size);
    void SetPixelThickness(double thick);
	void SetResolution(QVector2D dim);
    void SetJobName(QString);
    void SetJobDescription(QString);

	void CalculateBuildArea();
	void UpdateZSpace();//calculates the size of the z box based on instance bounds
	
    B9Layout* pMain;
signals:
	void DirtChanged(bool dirt);
	void ProjectLoaded();

private:
	

	bool dirtied;
    QString mfilename;
	QStringList modelfilelist;
	QVector3D dimentions;
	QVector2D resolution;
	double xypixel;
	double zthick;
    QString jobExportName;//used when slicing to a job file.
    QString jobExportDesc;

	void SetDefaults();

};
#endif
