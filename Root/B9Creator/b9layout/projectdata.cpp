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

#include "projectdata.h"
#include <QDebug>
#include <QFile>


//////////////////////////////////
//Public
//////////////////////////////////

	//constructors/destructors
	ProjectData::ProjectData() : QObject()
	{
		New();

	}
	ProjectData::~ProjectData()
	{
	}

	//File Access:
	void ProjectData::New()//Clears out internal project data, creates a new one with defualts
	{
		SetDefaults();
        SetDirtied(false);
	}
	bool ProjectData::Open(QString filepath) //returns success
    {
        int FileVersion;
		bool jumpout = false;

		double resx;
		double resy;
        bool flipped = false;

        QSettings settings;

        //input file operations here
		QFile file(filepath);
        QString buff;//for io operations
        QString modelpath;//current model this function is trying to load.
		if(!file.open(QIODevice::ReadOnly))
		{
			return false;
        }
        SetDirtied(false);//presumably good from here so undirty the project

		QTextStream in(&file);//begin text streaming.


        in >> buff;
        if(buff == "ver")//we are looking at a post 1.4 b9 layout file
        {
            in >> buff;
            FileVersion = buff.toInt();//should be 14 or greater.
            in.skipWhiteSpace();
            mfilename = in.readLine();//get project name
        }
        else//old file - we need to read first line for project name
        {
            FileVersion = 13;

            in.seek(0);//go back and re-read.
            mfilename = in.readLine();//get project name
        }
        qDebug() << "Layout Version: " << FileVersion;

		buff = in.readLine();//eat startmodellist
		while(!jumpout)
		{
			in.skipWhiteSpace();
			buff = in.readLine();
			
			if(buff != "endinstancelist")
			{
                modelpath = buff;
                ModelInstance* pinst = pMain->AddModel(modelpath);//inst will be null if it cant find the file.
				
				in >> buff;
				double xp = buff.toDouble(); in >> buff;
				double yp = buff.toDouble(); in >> buff;
				double zp = buff.toDouble();
				in >> buff;
				double xr = buff.toDouble(); in >> buff;
				double yr = buff.toDouble(); in >> buff;
				double zr = buff.toDouble();
				in >> buff;
				double xs = buff.toDouble(); in >> buff;
				double ys = buff.toDouble(); in >> buff;
                double zs = buff.toDouble();
                if(FileVersion >= 14)//version 14 added instance flipping
                {
                    in >> buff;
                    flipped = (bool)buff.toInt();
                }
                else
                    flipped = false;


                if(pinst == NULL)
                {
                    SetDirtied(true);
                    //1st chance to relocate the model..look in working directory.
                    QString modelfilename = QFileInfo(modelpath).fileName();
                    pinst = pMain->AddModel(QFileInfo(filepath).absolutePath() + "/" + modelfilename);

                    if(pinst == NULL)//2nd chance popup dialog asking if the user wants to manually find the file.
                    {
                         QApplication::restoreOverrideCursor();
                         QMessageBox msgBox;
                         msgBox.setText("the model: " + modelfilename + "can not be located");
                         msgBox.setInformativeText("Do you want to locate it manually?");
                         msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Discard);
                         msgBox.setDefaultButton(QMessageBox::Yes);
                         int response = msgBox.exec();
                         if(response == QMessageBox::Yes)
                         {
                             QString correctedfilepath = QFileDialog::getOpenFileName(0, QString("Locate File: " + modelfilename),
                         settings.value("WorkingDir").toString(),QString(modelfilename+"(*" + QFileInfo(modelfilename).suffix() + ")"));
                             if(!correctedfilepath.isEmpty())
                             {
                                 pinst = pMain->AddModel(correctedfilepath);

                             }


                             if(pinst == NULL)//otherwise the user failed to find one
                             {
                                 QMessageBox msgBox;
                                 msgBox.setText("substitute file unable to be inported.");
                                 msgBox.exec();
                             }


                         }
                    }
                }
                if(pinst != NULL)
                {
                    pinst->SetPos(QVector3D(xp,yp,zp));
                    pinst->SetRot(QVector3D(xr,yr,zr));
                    pinst->SetScale(QVector3D(xs,ys,zs));
                    pinst->SetFlipped(flipped);
                    pinst->UpdateBounds();//in order that zhieght is good and all.
                }
			}
			else
			{
				jumpout = true;
			}
		}

		in >> buff >> resx >> resy;
		in >> buff >> xypixel;


		SetResolution(QVector2D(resx,resy));
		CalculateBuildArea();


        mfilename = filepath;
		emit ProjectLoaded();
		return true;
	}
	bool ProjectData::Save(QString filepath) //returns success
	{

		QFile file(filepath);

		if(!file.open(QIODevice::Truncate | QIODevice::WriteOnly))
		{
			return false;
		}
		QTextStream out(&file);//begin text streaming.
        out << "ver " << LAYOUT_FILE_VERSION << '\n';
		out << filepath << '\n';//write project name
		
		out << "startinstancelist" << '\n';//begin model list
        for(unsigned int m=0;m<pMain->ModelDataList.size();m++)
		{
            for(unsigned int i=0;i<pMain->ModelDataList[m]->instList.size();i++)
			{
				ModelInstance* inst = pMain->ModelDataList[m]->instList[i];
                out << pMain->ModelDataList[m]->GetFilePath() << '\n' << inst->GetPos().x() <<" "<< inst->GetPos().y() <<" "<< inst->GetPos().z();
				out <<" "<< inst->GetRot().x() <<" "<< inst->GetRot().y() <<" "<< inst->GetRot().z();
                out <<" "<< inst->GetScale().x() <<" "<< inst->GetScale().y() <<" "<< inst->GetScale().z();
                out <<" " << inst->GetFlipped() << '\n';
			}
		}
		out << "endinstancelist" << '\n';//end model list

		out << "resolution " << GetResolution().x() << " " << GetResolution().y() << '\n';
		out << "XYpixelsize " << GetPixelSize() << '\n';


		SetDirtied(false);
        mfilename = filepath;
		return true;
	}

	//Structure Access:
	//Gets
	bool ProjectData::IsDirtied()
	{
		return dirtied;
	}
	QString ProjectData::GetFileName()//untitled if not saved yet
	{
        return mfilename;
	}
	QStringList ProjectData::GetModelFileList()
	{
		return modelfilelist;
	}
	QVector3D ProjectData::GetBuildSpace()
	{
		return dimentions;
	}
	double ProjectData::GetPixelSize()
	{
		return  xypixel;
	}
    double ProjectData::GetPixelThickness()
    {
        return  zthick;
    }
	QVector2D ProjectData::GetResolution()
	{
		return resolution;
	}
    QString ProjectData::GetJobName()
    {
        return jobExportName;
    }
    QString ProjectData::GetJobDescription()
    {
        return jobExportDesc;
    }



	//Sets
	void ProjectData::SetDirtied(bool dirt)
	{
        dirtied = dirt;
	}

	
	void ProjectData::SetBuildSpaceSize(QVector3D size)
	{
		dimentions = size;
		SetDirtied(true);
	}
	void ProjectData::SetPixelSize(double size)
	{
		xypixel = size;
		SetDirtied(true);
	}
    void ProjectData::SetPixelThickness(double thick)
    {
        zthick = thick;
        SetDirtied(true);
    }
    void ProjectData::SetResolution(QVector2D dim)
	{
		resolution = dim;
		SetDirtied(true);
	}

    void ProjectData::SetJobName(QString name)
    {
        jobExportName = name;
    }

    void ProjectData::SetJobDescription(QString desc)
    {
        jobExportDesc = desc;
    }




	void ProjectData::CalculateBuildArea()
	{
		SetBuildSpaceSize(QVector3D((xypixel/1000.0)*resolution.x(),(xypixel/1000.0)*resolution.y(),dimentions.z()));
	}
	void ProjectData::UpdateZSpace()
	{
        unsigned int m;
        unsigned int i;
		double max = 0.0;
		for(m=0;m<pMain->ModelDataList.size();m++)
		{
			for(i=0;i<pMain->ModelDataList[m]->instList.size();i++)
			{
				ModelInstance* inst = pMain->ModelDataList[m]->instList[i];
				if(inst->GetMaxBound().z() > max)
					max = inst->GetMaxBound().z();

			}
		}

		SetBuildSpaceSize(QVector3D(dimentions.x(),dimentions.y(),max));
	}


//////////////////////////////////
//Private
//////////////////////////////////

	void ProjectData::SetDefaults()
	{
        mfilename = "untitled";
		SetPixelSize(100);
		SetPixelThickness(100);
		SetResolution(QVector2D(1024,768));
		CalculateBuildArea();
		SetBuildSpaceSize(QVector3D(dimentions.x(),dimentions.y(),100));
	}
