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

#include "modeldata.h"

#include <assimp/cimport.h>       // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <QtOpenGL>
#include <QFileInfo>
#include <iostream>



//////////////////////////////////////
//Public 
//////////////////////////////////////
ModelData::ModelData(B9Layout* main)
{
	pMain = main;
	filepath = "";
	loadedcount=0;

	maxbound = QVector3D(-999999.0,-999999.0,-999999.0);
	minbound = QVector3D(999999.0,999999.0,999999.0);

    displaylistindx = glGenLists(2);//get an available display index
    displaylistflippedindx = displaylistindx + 1;
    qDebug() << "ModelData created with display list: " << displaylistindx;
    qDebug() << " and a flipped display list of: " << displaylistflippedindx;

}

ModelData::~ModelData()
{
    unsigned int i;


    triList.clear();


	for(i=0; i < instList.size();i++)
	{
		delete instList[i];
	}


    glDeleteLists(displaylistindx,2);



    qDebug() << "ModelData Deleted with displaylist " << displaylistindx;
    qDebug() << " and a flipped display list of: " << displaylistflippedindx;
}

QString ModelData::GetFilePath()
{	
	return filepath;
}
QString ModelData::GetFileName()
{
	return filename;
}

//data loading
bool ModelData::LoadIn(QString filepath)
{	
    unsigned int m;
    unsigned int t;
    unsigned int i;

    Triangle3D newtri;
    const struct aiFace* face;

	
	this->filepath = filepath;
	
	if(filepath.isEmpty())
		return false;
	
	//extract filename from path!
	filename = QFileInfo(filepath).baseName();

	//AI_CONFIG_PP_FD_REMOVE = aiPrimitiveType_POINTS | aiPrimitiveType_LINES;
    pScene = aiImportFile(filepath.toAscii(), aiProcess_Triangulate);// | aiProcess_JoinIdenticalVertices); //trian
	
    if(pScene == NULL)//assimp cant handle the file - lets try our own reader.
	{
		//display Assimp Error
		QMessageBox msgBox;
		msgBox.setText("Assimp Error:  " + QString().fromAscii(aiGetErrorString()));
		msgBox.exec();

        aiReleaseImport(pScene);

		return false;
	}

    qDebug() << "Model imported with " << pScene->mMeshes[0]->mNumFaces << " faces.";
	

	for (m = 0; m < pScene->mNumMeshes; m++) 
	{
		const aiMesh* mesh = pScene->mMeshes[m];
		
	    for (t = 0; t < mesh->mNumFaces; t++)
		{
            face = &mesh->mFaces[t];
			
			if(face->mNumIndices == 3)
			{
				for(i = 0; i < face->mNumIndices; i++) 
				{
					int index = face->mIndices[i];
				
                    newtri.normal.setX(mesh->mNormals[index].x);
                    newtri.normal.setY(mesh->mNormals[index].y);
                    newtri.normal.setZ(mesh->mNormals[index].z);
			
                    newtri.vertex[i].setX(mesh->mVertices[index].x);
                    newtri.vertex[i].setY(mesh->mVertices[index].y);
                    newtri.vertex[i].setZ(mesh->mVertices[index].z);
				}
                newtri.UpdateBounds();
                triList.push_back(newtri);

			}
		}
	}

    aiReleaseImport(pScene);

    qDebug() << "Loaded triangles: " << triList.size();
	//now center it!
	CenterModel();

	//generate a displaylist
    int displayerror = FormDisplayList();

    //check for errors in display list creation (if its a large model the card may run out of memory.

    if(displayerror){
    while(displayerror)//loop and see if there are additional errors as well.
    {
        //display Assimp Error
        qDebug() << "Display List Error: " << displayerror; //write to log as well.
        QMessageBox msgBox;

        switch(displayerror)
        {
        case GL_OUT_OF_MEMORY:
            msgBox.setText("OpenGL Error:  GL_OUT_OF_MEMORY\nModel is too large to render on your graphics card.");
            break;
        case GL_INVALID_ENUM:
            msgBox.setText("OpenGL Error:  GL_INVALID_ENUM");
            break;
        case GL_INVALID_VALUE:
            msgBox.setText("OpenGL Error:  GL_INVALID_VALUE");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            msgBox.setText("OpenGL Error:  GL_INVALID_FRAMEBUFFER_OPERATION");
            break;
        case GL_STACK_UNDERFLOW:
            msgBox.setText("OpenGL Error:  GL_STACK_UNDERFLOW");
            break;
        case GL_STACK_OVERFLOW:
            msgBox.setText("OpenGL Error:  GL_STACK_OVERFLOW");
            break;
        default:
            break;
        }

        msgBox.exec();
        displayerror = glGetError();
    }
    return false;
    }




	return true;
}

//instance
ModelInstance* ModelData::AddInstance()
{
	loadedcount++;
	ModelInstance* pNewInst = new ModelInstance(this);
	instList.push_back(pNewInst);
	pNewInst->SetTag(filename + " " + QString().number(loadedcount));
	return pNewInst;
}


//////////////////////////////////////
//Private
//////////////////////////////////////
void ModelData::CenterModel()
{
	//figure out what to current center of the models counds is..
    unsigned int f;
    unsigned int v;
	QVector3D center;


	for(f=0;f<triList.size();f++)
	{
		for(v=0;v<3;v++)
		{
			//update the models bounds.
			//max
            if(triList[f].maxBound.x() > maxbound.x())
			{
                maxbound.setX(triList[f].vertex[v].x());
			}
            if(triList[f].maxBound.y() > maxbound.y())
			{
                maxbound.setY(triList[f].vertex[v].y());
			}
            if(triList[f].maxBound.z() > maxbound.z())
			{
                maxbound.setZ(triList[f].vertex[v].z());
			}
			
			//mins
            if(triList[f].minBound.x() < minbound.x())
			{
                minbound.setX(triList[f].vertex[v].x());
			}
            if(triList[f].minBound.y() < minbound.y())
            {
                minbound.setY(triList[f].vertex[v].y());
			}
            if(triList[f].minBound.z() < minbound.z())
			{
                minbound.setZ(triList[f].vertex[v].z());
			}
		}
	}

	center = (maxbound + minbound)*0.5;

    for(f=0;f<triList.size();f++)
	{
		for(v=0;v<3;v++)
		{
            triList[f].vertex[v] -= center;
		}
        triList[f].UpdateBounds(); // since we are moving every triangle, we need to update their bounds too.
	}
	maxbound -= center;
	minbound -= center;
}

//rendering
//generate opengl display list, flipx generates with inverted x normals and vertices
int ModelData::FormDisplayList() //returns opengl error enunum.
{

    unsigned int t;
	if (displaylistindx != 0) 
	{
		glNewList(displaylistindx,GL_COMPILE);
		glBegin(GL_TRIANGLES);// Drawing Using Triangles
		for(t = 0; t < triList.size(); t++)//for each triangle
		{

                glNormal3f(triList[t].normal.x(),triList[t].normal.y(),triList[t].normal.z());//normals
			
                glVertex3f( triList[t].vertex[0].x(), triList[t].vertex[0].y(), triList[t].vertex[0].z());
                glVertex3f( triList[t].vertex[1].x(), triList[t].vertex[1].y(), triList[t].vertex[1].z());
                glVertex3f( triList[t].vertex[2].x(), triList[t].vertex[2].y(), triList[t].vertex[2].z());

		}
		glEnd();
		glEndList();
	}
    else
        return glGetError();


    //form the flipped version as well.
    if (displaylistflippedindx != 0)
    {
        glNewList(displaylistflippedindx,GL_COMPILE);
        glBegin(GL_TRIANGLES);// Drawing Using Triangles
        for(t = 0; t < triList.size(); t++)//for each triangle
        {
                glNormal3f(-triList[t].normal.x(),triList[t].normal.y(),triList[t].normal.z());//normals

                glVertex3f( -triList[t].vertex[2].x(), triList[t].vertex[2].y(), triList[t].vertex[2].z());
                glVertex3f( -triList[t].vertex[1].x(), triList[t].vertex[1].y(), triList[t].vertex[1].z());
                glVertex3f( -triList[t].vertex[0].x(), triList[t].vertex[0].y(), triList[t].vertex[0].z());

        }
        glEnd();
        glEndList();
    }
    else
        return glGetError();


    return 0;
}
