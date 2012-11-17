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
		bool jumpout = false;

		double resx;
		double resy;
		
		//input file operations here TODO
		QFile file(filepath);
		QString buff;
		if(!file.open(QIODevice::ReadOnly))
		{
			return false;
		}
		QTextStream in(&file);//begin text streaming.
		filename = in.readLine();//get project name
		
		buff = in.readLine();//eat startmodellist
		while(!jumpout)
		{
			in.skipWhiteSpace();
			buff = in.readLine();
			
			if(buff != "endinstancelist")
			{
				ModelInstance* inst = pMain->AddModel(buff);
				
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
			
				inst->SetPos(QVector3D(xp,yp,zp));
				inst->SetRot(QVector3D(xr,yr,zr));
				inst->SetScale(QVector3D(xs,ys,zs));
			}
			else
			{
				jumpout = true;
			}
		}

		in >> buff >> resx >> resy;
		in >> buff >> xypixel;
		in >> buff >> zthick;


		SetResolution(QVector2D(resx,resy));
		CalculateBuildArea();


		SetDirtied(false);
		emit ProjectLoaded();
		return true;
	}
	bool ProjectData::Save(QString filepath) //returns success
	{
		//output file operations here.TODO
		QFile file(filepath);

		if(!file.open(QIODevice::Truncate | QIODevice::WriteOnly))
		{
			return false;
		}
		QTextStream out(&file);//begin text streaming.
		out << filepath << '\n';//write project name
		
		out << "startinstancelist" << '\n';//begin model list
        for(unsigned int m=0;m<pMain->ModelDataList.size();m++)
		{
            for(unsigned int i=0;i<pMain->ModelDataList[m]->instList.size();i++)
			{
				ModelInstance* inst = pMain->ModelDataList[m]->instList[i];
                out << pMain->ModelDataList[m]->GetFilePath() << '\n' << inst->GetPos().x() <<" "<< inst->GetPos().y() <<" "<< inst->GetPos().z();
				out <<" "<< inst->GetRot().x() <<" "<< inst->GetRot().y() <<" "<< inst->GetRot().z();
				out <<" "<< inst->GetScale().x() <<" "<< inst->GetScale().y() <<" "<< inst->GetScale().z() << '\n';
			}
		}
		out << "endinstancelist" << '\n';//end model list

		out << "resolution " << GetResolution().x() << " " << GetResolution().y() << '\n';
		out << "XYpixelsize " << GetPixelSize() << '\n';
        out << "Zlayer " << GetPixelThickness() << '\n';

		SetDirtied(false);
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
		return filename;
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

	//Sets
	void ProjectData::SetDirtied(bool dirt)
	{
		emit DirtChanged(dirt);
		if(dirt != dirtied)
		{
			
		}
		dirtied = dirt;
	}
	void ProjectData::SetFileName(QString name)
	{
		filename = name;
		SetDirtied(true);
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
		SetFileName("untitled");
		SetPixelSize(100);
		SetPixelThickness(100);
		SetResolution(QVector2D(1024,768));
		CalculateBuildArea();
		SetBuildSpaceSize(QVector3D(dimentions.x(),dimentions.y(),100));
	}
