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

#include "b9supportstructure.h"


#include "b9layout/b9modelinstance.h"
#include "b9modelloader.h"
#include "OS_Wrapper_Functions.h"
#include "b9layout/geometricfunctions.h"
#include <QtOpenGL>



//B9SupportAttachmentData
////////////////////////////////////////////////////////////////////////


//Public
/////////////////////////////
B9SupportAttachmentData::B9SupportAttachmentData()
{
    name = "Default Attachment";
    dispListIndx = 0;
}
B9SupportAttachmentData::~B9SupportAttachmentData()
{
    if(dispListIndx != 0)
        glDeleteLists(dispListIndx,1);
}

void B9SupportAttachmentData::CenterGeometry()
{
    unsigned int t,v;
    Triangle3D* cTri = NULL;
    QVector3D MinBound(999999,999999,999999);
    QVector3D MaxBound(-999999,-9999999,-999999);
    QVector3D Center;

    for(t = 0; t < triList.size(); t++)
    {
        cTri = &triList[t];

        if(cTri->maxBound.x() > MaxBound.x())
            MaxBound.setX(cTri->maxBound.x());
        if(cTri->maxBound.y() > MaxBound.y())
            MaxBound.setY(cTri->maxBound.y());
        if(cTri->maxBound.z() > MaxBound.z())
            MaxBound.setZ(cTri->maxBound.z());

        if(cTri->minBound.x() < MinBound.x())
            MinBound.setX(cTri->minBound.x());
        if(cTri->minBound.y() < MinBound.y())
            MinBound.setY(cTri->minBound.y());
        if(cTri->minBound.z() < MinBound.z())
            MinBound.setZ(cTri->minBound.z());
    }

    Center = (MaxBound + MinBound)*0.5;

    //now we need to move all the triangles...
    for(t = 0; t < triList.size(); t++)
    {
        for(v=0;v<3;v++)
        {
            triList[t].vertex[v] -= Center;
        }
        triList[t].UpdateBounds(); // since we are moving every triangle, we need to update their bounds too.
    }
}

void B9SupportAttachmentData::FormDisplayList()
{
    unsigned int t;
    if(dispListIndx != 0)
        glDeleteLists(dispListIndx,1);

    dispListIndx = glGenLists(1);
    if(dispListIndx == 0)
    {
        qDebug() << "B9SupportAttachmentData: Error Creating display list";
        return;
    }

    glNewList(dispListIndx,GL_COMPILE);
    glBegin(GL_TRIANGLES);

    for(t = 0; t < triList.size(); t++)//for each triangle
    {
        glNormal3f( triList[t].normal.x(),triList[t].normal.y(),triList[t].normal.z());//normals

        glVertex3f( triList[t].vertex[0].x(), triList[t].vertex[0].y(), triList[t].vertex[0].z());
        glVertex3f( triList[t].vertex[1].x(), triList[t].vertex[1].y(), triList[t].vertex[1].z());
        glVertex3f( triList[t].vertex[2].x(), triList[t].vertex[2].y(), triList[t].vertex[2].z());
    }
    glEnd();
    glEndList();
}

void B9SupportAttachmentData::Render()
{
    if(dispListIndx != 0)
        glCallList(dispListIndx);

}









//B9SupportStructure
//////////////////////////////////////////////////////////////////////////

//Instantiate static members
std::vector<B9SupportAttachmentData>
    B9SupportStructure::AttachmentDataList = std::vector<B9SupportAttachmentData>();
unsigned char B9SupportStructure::sColorID[3] = {0,0,0};

//Public
////////////////////////////////////////////////////////
B9SupportStructure::B9SupportStructure(B9ModelInstance *parent)
{
    instanceParent = parent;
    isSelected = false;
    isGrounded = false;
    isVisible = true;
    isErrorGlowing = false;
    topAttachShape = NULL;
    midAttachShape = NULL;
    bottomAttachShape = NULL;

    //these are "very loose defaults"
    topRadius = 0.5;
    midRadius = 0.5;
    bottomRadius = 2.0;

    bottomPoint = QVector3D(0,0,0);
    topPoint = QVector3D(0,0,1);

    topLength = 1.0;
    bottomLength = 0.5;

    topPenetration = 0.4;
    bottomPenetration = 0.4;

    topNormal = QVector3D(0,0,1.0);
    bottomNormal = QVector3D(0,0,-1.0);
    topAngleFactor = 0.20;
    bottomAngleFactor = 0.20;

    SetTopAttachShape("Cone 25%");
    SetMidAttachShape("Cylinder");
    SetBottomAttachShape("Cylinder");

    ForceUpdate();
}

B9SupportStructure::~B9SupportStructure()
{

}

void B9SupportStructure::AssignPickId()
{
    //picking color incrementor.
    pickcolor[0] = sColorID[0];
    pickcolor[1] = sColorID[1];
    pickcolor[2] = sColorID[2];

    sColorID[0]++;
    if(sColorID[0] >= 255)
    {
        sColorID[0] = 0;
        sColorID[1]++;
        if(sColorID[1] >= 255)
        {
                sColorID[1] = 0;
                sColorID[2]++;
        }
    }
}

void B9SupportStructure::CopyFrom(B9SupportStructure* referenceSupport)
{
    instanceParent = referenceSupport->instanceParent;

    topAttachShape = referenceSupport->topAttachShape;
    midAttachShape = referenceSupport->midAttachShape;
    bottomAttachShape = referenceSupport->bottomAttachShape;
    SetIsGrounded(referenceSupport->GetIsGrounded());
    SetTopPoint(referenceSupport->GetTopPoint());
    SetBottomPoint(referenceSupport->GetBottomPoint());
    SetTopRadius(referenceSupport->GetTopRadius());
    SetMidRadius(referenceSupport->GetMidRadius());
    SetBottomRadius(referenceSupport->GetBottomRadius());
    SetTopLength(referenceSupport->GetTopLength());
    SetBottomLength(referenceSupport->GetBottomLength());
    SetTopPenetration(referenceSupport->GetTopPenetration());
    SetBottomPenetration(referenceSupport->GetBottomPenetration());
    SetTopNormal(referenceSupport->GetTopNormal());
    SetBottomNormal(referenceSupport->GetBottomNormal());
    SetTopAngleFactor(referenceSupport->GetTopAngleFactor());
    SetBottomAngleFactor(referenceSupport->GetBottomAngleFactor());

}


//Public Static Functions
////////////////////////////////////////////////////////////////////

//Imports
void B9SupportStructure::ImportAttachmentDataFromStls()
{
    int i;
    bool s;
    bool triangleError;
    STLTri* pLoadedTri = NULL;
    Triangle3D newtri;
    QDir appdir(CROSS_OS_GetDirectoryFromLocationTag("APPLICATION_DIR"));
    QStringList filters;
    B9SupportAttachmentData importedData;

    qDebug() << "B9SupportStructure: Importing Stl Attachments...";

    //Import All Stl in the application directory begining with "SUPPORT"
    filters << "SUPPORT_*";
    appdir.setNameFilters(filters);
    QFileInfoList stlFiles = appdir.entryInfoList();
    for(i = 0; i < stlFiles.size(); i++)
    {
        B9ModelLoader stlLoader(stlFiles[i].filePath(),s);
        if(s)
        {
            while(stlLoader.LoadNextTri(pLoadedTri,triangleError))
            {
                if(triangleError)
                {
                    qDebug() << "B9SupportStructure: ErrorLoading triangle";
                    return;
                }
                newtri.normal.setX(pLoadedTri->nx);
                newtri.normal.setY(pLoadedTri->ny);
                newtri.normal.setZ(pLoadedTri->nz);

                newtri.vertex[0].setX(pLoadedTri->x0);
                newtri.vertex[0].setY(pLoadedTri->y0);
                newtri.vertex[0].setZ(pLoadedTri->z0);
                newtri.vertex[1].setX(pLoadedTri->x1);
                newtri.vertex[1].setY(pLoadedTri->y1);
                newtri.vertex[1].setZ(pLoadedTri->z1);
                newtri.vertex[2].setX(pLoadedTri->x2);
                newtri.vertex[2].setY(pLoadedTri->y2);
                newtri.vertex[2].setZ(pLoadedTri->z2);

                delete pLoadedTri;
                newtri.UpdateBounds();

                importedData.GetTriangles()->push_back(newtri);
            }

            //save the data into the Static List.
            importedData.SetName(stlFiles[i].baseName().remove("SUPPORT_"));
            importedData.CenterGeometry();//center the triangles around 0,0,0
            B9SupportStructure::AttachmentDataList.push_back(importedData);
            //clear out importedData
            importedData = B9SupportAttachmentData();
        }
        else
        {
            qDebug() << "B9SupportStructure: Error Loading: " << stlFiles[i].fileName();
        }

    }

    qDebug() << "B9SupportStructure: Succesfully Loaded "
             << B9SupportStructure::AttachmentDataList.size()
             << " Attachment Types";
}



//Default support parameters here!
void B9SupportStructure::FillRegistryDefaults(bool reset, QString supportWeight)
{
    QSettings appSettings;

    if(!appSettings.value("SUPPORTPARAMSLOADED").toBool() || reset)
    {
        appSettings.setValue("SUPPORTPARAMSLOADED",true);
        appSettings.beginGroup("USERSUPPORTPARAMS");

        if(supportWeight == "LIGHT")
        {
            appSettings.setValue("ADDPRESETWEIGHT","LIGHT");
        appSettings.beginGroup("SUPPORT_TOP");
            appSettings.setValue("ATTACHSHAPE","Cone 25%");
                appSettings.setValue("ANGLEFACTOR",0.8);
                appSettings.setValue("LENGTH",2.0);
                appSettings.setValue("PENETRATION",0.25);
                appSettings.setValue("RADIUS",0.5);

            appSettings.endGroup();
            appSettings.beginGroup("SUPPORT_MID");
            appSettings.setValue("ATTACHSHAPE","Cylinder");
                appSettings.setValue("RADIUS",0.5);

            appSettings.endGroup();
            appSettings.beginGroup("SUPPORT_BOTTOM_GROUNDED");
                appSettings.setValue("ATTACHSHAPE","Cylinder");
                appSettings.setValue("ANGLEFACTOR",1.0);
                appSettings.setValue("LENGTH",0.25);
                appSettings.setValue("PENETRATION",0.0);
                appSettings.setValue("RADIUS",3.0);

            appSettings.endGroup();
            appSettings.beginGroup("SUPPORT_BOTTOM_NONGROUNDED");

                appSettings.setValue("ATTACHSHAPE","Cone 25%");
                appSettings.setValue("ANGLEFACTOR",0.5);
                appSettings.setValue("LENGTH",1.0);
                appSettings.setValue("PENETRATION",0.4);
                appSettings.setValue("RADIUS",0.5);

            appSettings.endGroup();
        }
        else if(supportWeight == "MEDIUM")
        {
            appSettings.setValue("ADDPRESETWEIGHT","MEDIUM");
        appSettings.beginGroup("SUPPORT_TOP");
            appSettings.setValue("ATTACHSHAPE","Cone 25%");
                appSettings.setValue("ANGLEFACTOR",0.8);
                appSettings.setValue("LENGTH",4.0);
                appSettings.setValue("PENETRATION",0.5);
                appSettings.setValue("RADIUS",1.0);

            appSettings.endGroup();
            appSettings.beginGroup("SUPPORT_MID");
            appSettings.setValue("ATTACHSHAPE","Cylinder");
                appSettings.setValue("RADIUS",1.0);

            appSettings.endGroup();
            appSettings.beginGroup("SUPPORT_BOTTOM_GROUNDED");
                appSettings.setValue("ATTACHSHAPE","Cylinder");
                appSettings.setValue("ANGLEFACTOR",1.0);
                appSettings.setValue("LENGTH",0.5);
                appSettings.setValue("PENETRATION",0.0);
                appSettings.setValue("RADIUS",6.0);

            appSettings.endGroup();
            appSettings.beginGroup("SUPPORT_BOTTOM_NONGROUNDED");

                appSettings.setValue("ATTACHSHAPE","Cone 25%");
                appSettings.setValue("ANGLEFACTOR",0.5);
                appSettings.setValue("LENGTH",2.0);
                appSettings.setValue("PENETRATION",0.5);
                appSettings.setValue("RADIUS",1.0);

            appSettings.endGroup();
        }
        else if(supportWeight == "HEAVY")
        {
            appSettings.setValue("ADDPRESETWEIGHT","HEAVY");
        appSettings.beginGroup("SUPPORT_TOP");
            appSettings.setValue("ATTACHSHAPE","Cone 25%");
                appSettings.setValue("ANGLEFACTOR",0.8);
                appSettings.setValue("LENGTH",8.0);
                appSettings.setValue("PENETRATION",0.75);
                appSettings.setValue("RADIUS",2.0);

            appSettings.endGroup();
            appSettings.beginGroup("SUPPORT_MID");
            appSettings.setValue("ATTACHSHAPE","Cylinder");
                appSettings.setValue("RADIUS",2.0);

            appSettings.endGroup();
            appSettings.beginGroup("SUPPORT_BOTTOM_GROUNDED");
                appSettings.setValue("ATTACHSHAPE","Cylinder");
                appSettings.setValue("ANGLEFACTOR",1.0);
                appSettings.setValue("LENGTH",0.5);
                appSettings.setValue("PENETRATION",0.0);
                appSettings.setValue("RADIUS",12.0);

            appSettings.endGroup();
            appSettings.beginGroup("SUPPORT_BOTTOM_NONGROUNDED");

                appSettings.setValue("ATTACHSHAPE","Cone 25%");
                appSettings.setValue("ANGLEFACTOR",0.5);
                appSettings.setValue("LENGTH",4.0);
                appSettings.setValue("PENETRATION",0.75);
                appSettings.setValue("RADIUS",2.0);

            appSettings.endGroup();
        }
        appSettings.endGroup();

    }
}

//Public Member Functions
////////////////////////////////////////////////////////////////////
void B9SupportStructure::SetInstanceParent(B9ModelInstance* parentInst)
{
    instanceParent = parentInst;
}

void B9SupportStructure::SetTopAttachShape(QString shapeName)
{
    unsigned int i;
    this->topAttachShape = NULL;
    for(i = 0; i < B9SupportStructure::AttachmentDataList.size(); i++)
    {
        if(shapeName == B9SupportStructure::AttachmentDataList[i].GetName())
        {
            this->topAttachShape = &B9SupportStructure::AttachmentDataList[i];
            if(!topAttachShape->HasDisplayList()) topAttachShape->FormDisplayList();

            return;
        }
    }
}
void B9SupportStructure::SetMidAttachShape(QString shapeName)
{
    unsigned int i;
    this->midAttachShape = NULL;
    for(i = 0; i < B9SupportStructure::AttachmentDataList.size(); i++)
    {
        if(shapeName == B9SupportStructure::AttachmentDataList[i].GetName())
        {
            this->midAttachShape = &B9SupportStructure::AttachmentDataList[i];
            if(!midAttachShape->HasDisplayList()) midAttachShape->FormDisplayList();

            return;
        }
    }
}
void B9SupportStructure::SetBottomAttachShape(QString shapeName)
{
    unsigned int i;
    this->bottomAttachShape = NULL;
    for(i = 0; i < B9SupportStructure::AttachmentDataList.size(); i++)
    {
        if(shapeName == B9SupportStructure::AttachmentDataList[i].GetName())
        {
            this->bottomAttachShape = &B9SupportStructure::AttachmentDataList[i];
            if(!bottomAttachShape->HasDisplayList()) bottomAttachShape->FormDisplayList();

            return;
        }
    }
}

void B9SupportStructure::SetTopPoint(QVector3D cord)
{
    topPoint = cord;
    ForceUpdate();
}
void B9SupportStructure::SetTopNormal(QVector3D normalVector)
{
    normalVector.normalize();
    topNormal = normalVector;

    //Quick filter for perfectly vertical serfaces
    //we need the angle factor to have some usefullness
    if(fabs(topNormal.z()) < 0.05)
    {
        topNormal.setZ(0.05);
        topNormal.normalize();
    }


    ForceUpdate();
}

void B9SupportStructure::SetTopRadius(double rad)
{
    topRadius = rad;
    ForceUpdate();
}
void B9SupportStructure::SetTopLength(double len)
{
    topLength = len;
    ForceUpdate();
}

void B9SupportStructure::SetMidRadius(double rad)
{
    midRadius = rad;
    ForceUpdate();
}
void B9SupportStructure::SetBottomRadius(double rad)
{
    bottomRadius = rad;
    ForceUpdate();
}
void B9SupportStructure::SetBottomPoint(QVector3D cord)
{
    bottomPoint = cord;
    ForceUpdate();
}
void B9SupportStructure::SetBottomNormal(QVector3D normalVector)
{
    normalVector.normalize();
    bottomNormal = normalVector;
    ForceUpdate();
}

void B9SupportStructure::SetTopAngleFactor(double factor)
{
    topAngleFactor = factor;
    ForceUpdate();
}

void B9SupportStructure::SetBottomAngleFactor(double factor)
{
    bottomAngleFactor = factor;
    ForceUpdate();
}

void B9SupportStructure::SetTopPenetration(double pen)
{
    topPenetration = pen;
    ForceUpdate();
}

void B9SupportStructure::SetBottomPenetration(double pen)
{
    bottomPenetration = pen;
    ForceUpdate();
}

void B9SupportStructure::SetBottomLength(double len)
{
    bottomLength = len;
    ForceUpdate();
}

void B9SupportStructure::SetIsGrounded(bool grnd){
    isGrounded = grnd;
    ForceUpdate();
}

void B9SupportStructure::Rotate(QVector3D deltaRot)
{

    RotateVector(topPoint, deltaRot.z(), QVector3D(0,0,1));
    RotateVector(bottomPoint, deltaRot.z(), QVector3D(0,0,1));

    RotateVector(topNormal, deltaRot.z(), QVector3D(0,0,1));
    RotateVector(bottomNormal, deltaRot.z(), QVector3D(0,0,1));

    ForceUpdate();
}

QString B9SupportStructure::GetTopAttachShape()
{
    if(topAttachShape != NULL)
        return topAttachShape->GetName();
    else
        return "";
}
QString B9SupportStructure::GetMidAttachShape()
{
    if(midAttachShape != NULL)
        return midAttachShape->GetName();
    else
        return "";
}
QString B9SupportStructure::GetBottomAttachShape()
{
    if(bottomAttachShape != NULL)
        return bottomAttachShape->GetName();
    else
        return "";
}


QVector3D B9SupportStructure::GetTopPoint()
{
    return topPoint;
}

QVector3D B9SupportStructure::GetTopPivot()
{
    return topPivot;
}

double B9SupportStructure::GetTopRadius()
{
    return topRadius;
}
double B9SupportStructure::GetMidRadius()
{
    return midRadius;
}
double B9SupportStructure::GetBottomRadius()
{
    return bottomRadius;
}
double B9SupportStructure::GetTopLength()
{
    return topLength;
}

double B9SupportStructure::GetMidLength()
{
    return midLength;
}

double B9SupportStructure::GetBottomLength()
{
    return bottomLength;
}

QVector3D B9SupportStructure::GetTopNormal()
{
    return topNormal;
}

QVector3D B9SupportStructure::GetBottomNormal()
{
    return bottomNormal;
}

double B9SupportStructure::GetTopAngleFactor()
{
    return topAngleFactor;
}

double B9SupportStructure::GetBottomAngleFactor()
{
    return bottomAngleFactor;
}

double B9SupportStructure::GetTopPenetration()
{
    return topPenetration;
}

double B9SupportStructure::GetBottomPenetration()
{
    return bottomPenetration;
}

QVector3D B9SupportStructure::GetBottomPoint()
{
    return bottomPoint;
}

QVector3D B9SupportStructure::GetBottomPivot()
{
    return (GetBottomPoint() - GetBottomNormal()*GetBottomLength());
}

bool B9SupportStructure::GetIsGrounded()
{
    return isGrounded;
}
bool B9SupportStructure::IsUpsideDown()
{
    if(topPoint.z() < bottomPoint.z())
        return true;

    return false;
}
bool B9SupportStructure::IsVertical()
{
    QVector2D disp = QVector2D(topPivot) - QVector2D(bottomPivot);
    if(disp.length() < 0.1)
        return true;

    return false;
}

bool B9SupportStructure::IsUnderground(double &depth)
{
    if(topPoint.z() + instanceParent->GetPos().z() < 0)
    {
        depth = fabs(topPoint.z() + instanceParent->GetPos().z());
        return true;
    }
    else
        return false;
}

//basically (is too short)...
bool B9SupportStructure::IsConstricted(double &depthPastContricted)
{
    double totalHieght;
    double topHieght;
    double bottomHiegth;

    if(IsUnderground(depthPastContricted))
    {
        return true;
    }



    totalHieght = topPoint.z() - bottomPoint.z();
    topHieght = fabs(topPoint.z() - topPivot.z());
    bottomHiegth = fabs(bottomPivot.z() - bottomPoint.z());

    if(totalHieght <= (topHieght + bottomHiegth))
    {
        depthPastContricted = fabs((topHieght + bottomHiegth) - totalHieght);
        return true;
    }

    depthPastContricted = 0.0;
    return false;
}




bool B9SupportStructure::IsPrintable()
{
    if(IsTopPrintable() && IsMidPrintable() && IsBottomPrintable() && !IsUpsideDown())
    {
        return true;
    }
    return false;
}

bool B9SupportStructure::IsTopPrintable()
{
    if(IsTopAngleUp())
    {
        return true;
    }
    //rad check?

    return false;
}

bool B9SupportStructure::IsTopAngleUp()
{
    if(topNormal.z() >= 0)
    {
        return true;
    }
    return false;
}

//checks if the mid section is printable.
bool B9SupportStructure::IsMidPrintable()
{
    double z;
    z = topPivot.z() - bottomPivot.z();

    if(z/midLength < SUPPORT_MID_PRINTABLE_ANGLE_RATIO )
        return false;

    //rad check?



    return true;
}

bool B9SupportStructure::IsBottomPrintable()
{
    if(IsBottomAngleDown())
    {
        return true;
    }
    //rad check?
    return false;
}

bool B9SupportStructure::IsBottomAngleDown()
{
    if(bottomNormal.z() <= 0)
    {
        return true;
    }

    return false;
}

bool B9SupportStructure::IsVisible()
{
   return isVisible;
}



//selection
void B9SupportStructure::SetSelected(bool sel)
{
    isSelected = sel;
}


//rendering

//pick rendering
void B9SupportStructure::RenderPickGL(bool renderBottom, bool renderTop)
{
    glPushMatrix();
        glColor3ub(pickcolor[0],pickcolor[1],pickcolor[2]);
        glTranslatef(instanceParent->GetPos().x(),
                     instanceParent->GetPos().y(),
                     instanceParent->GetPos().z());

        if(renderTop)
            RenderUpper(true);

        if(renderBottom)
            RenderLower(true);

    glPopMatrix();
}

void B9SupportStructure::RenderPartPickGL(bool renderBottom, bool renderTop)
{
    //render in global space
    glPushMatrix();
        glTranslatef(instanceParent->GetPos().x(),instanceParent->GetPos().y(),instanceParent->GetPos().z());

        //first draw the top
        if(topAttachShape != NULL && renderTop)
        {
            glColor3f(1.0f,0.0f,0.0f);
            RenderTopGL();
        }
        //next draw the middle section
        if(midAttachShape != NULL && renderTop)
        {
            glColor3f(0.0f,1.0f,0.0f);
            RenderMidGL();
        }

        //third draw the base
        if(bottomAttachShape != NULL && renderBottom)
        {
            glColor3f(0.0f,0.0f,1.0f);
            RenderBottomGL();
        }
    glPopMatrix();
}
void B9SupportStructure::EnableErrorGlow()
{
    isErrorGlowing = true;
}

void B9SupportStructure::DisableErrorGlow()
{
    isErrorGlowing = false;
}



//visual Rendering
void B9SupportStructure::RenderTopGL()
{
    glPushMatrix();
        glTranslatef((topPenetrationPoint.x() + topPivot.x())*0.5
                     ,(topPenetrationPoint.y() + topPivot.y())*0.5
                     ,(topPenetrationPoint.z() + topPivot.z())*0.5);

        glRotatef(topThetaZ,0.0,0.0,1.0);
        glRotatef(topThetaX,1.0,0.0,0.0);
        glScalef(topRadius*2,topRadius*2,topLength+topPenetration);
        topAttachShape->Render();

    glPopMatrix();
}

void B9SupportStructure::RenderMidGL()
{
    glPushMatrix();
        glTranslatef((topMidExtensionPoint.x() + bottomMidExtensionPoint.x())*0.5
                     ,(topMidExtensionPoint.y() + bottomMidExtensionPoint.y())*0.5
                     ,(topMidExtensionPoint.z() + bottomMidExtensionPoint.z())*0.5);


        glRotatef(midThetaZ,0.0,0.0,1.0);
        glRotatef(midThetaX,1.0,0.0,0.0);
        glScalef(midRadius*2,midRadius*2,midLength);
        midAttachShape->Render();

    glPopMatrix();
}

void B9SupportStructure::RenderBottomGL()
{
    glPushMatrix();
        glTranslatef((bottomPenetrationPoint.x() + bottomPivot.x())*0.5
                     ,(bottomPenetrationPoint.y() + bottomPivot.y())*0.5
                     ,(bottomPenetrationPoint.z() + bottomPivot.z())*0.5);

        glRotatef(bottomThetaZ,0.0,0.0,1.0);
        glRotatef(bottomThetaX+180,1.0,0.0,0.0);

        glScalef(bottomRadius*2,bottomRadius*2,bottomLength+bottomPenetration);
            bottomAttachShape->Render();
    glPopMatrix();
}

void B9SupportStructure::RenderUpper(bool disableColor, float alpha)
{
    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_COLOR_BUFFER_BIT);

    if(!disableColor)
    {
        if(!isSelected)
            glColor4f(0.0f,0.7f,0.0f,alpha);
        else
            glColor4f(0.0f,1.0f,1.0f,alpha);

    }
    if(isErrorGlowing && !disableColor)
    {
        glDisable(GL_LIGHTING);
        glColor4f(1.0f,0.0f,0.0f,alpha);
    }

    //first draw the top
    if(topAttachShape != NULL)
    {
        RenderTopGL();
    }
    //next draw the middle section
    if(midAttachShape != NULL)
    {
        RenderMidGL();
    }

    glPopAttrib();
    glPopAttrib();
}
void B9SupportStructure::RenderLower(bool disableColor, float alpha)
{
    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_COLOR_BUFFER_BIT);

    if(!disableColor)
    {
        if(!isSelected)
            glColor4f(0.0f,0.7f,0.0f,alpha);
        else
            glColor4f(0.0f,1.0f,1.0f,alpha);
    }
    if(isErrorGlowing && !disableColor)
    {
        glDisable(GL_LIGHTING);
        glColor4f(1.0f,0.0f,0.0f,alpha);
    }

    //third draw the base
    if(bottomAttachShape != NULL)
    {
        RenderBottomGL();
    }

    glPopAttrib();
    glPopAttrib();


}


void B9SupportStructure::DebugRender()
{
    if(!isVisible)
        return;

    glDisable(GL_LIGHTING);
    //For Now just daw open gl lines
    glBegin(GL_LINES);


            //PenetrationPoint->topPoint
            glColor3f(0.0f,0.0f,1.0f);
            glVertex3f(topPenetrationPoint.x(),
               topPenetrationPoint.y(),
               topPenetrationPoint.z());
            glVertex3f(topPoint.x(),
                       topPoint.y(),
                       topPoint.z());

            //TopPoint->TopPivot
            glColor3f(0.0f,1.0f,0.0f);
            glVertex3f(topPoint.x(),
               topPoint.y(),
               topPoint.z());
            glVertex3f(topPivot.x(),
                       topPivot.y(),
                       topPivot.z());

            //TopPivot->BottomPivot
            glColor3f(0.0f,1.0f,1.0f);
            glVertex3f(topPivot.x(),
               topPivot.y(),
               topPivot.z());
            glVertex3f(bottomPivot.x(),
                       bottomPivot.y(),
                       bottomPivot.z());

            //BottomPivot->BottomPoint
            glColor3f(1.0f,0.0f,0.0f);
            glVertex3f(bottomPivot.x(),
                       bottomPivot.y(),
                       bottomPivot.z());
            glVertex3f(bottomPoint.x(),
                       bottomPoint.y(),
                       bottomPoint.z());

            //BottomPoint->BottomPenetrationPoint
            glColor3f(1.0f,0.0f,1.0f);
            glVertex3f(bottomPoint.x(),
                       bottomPoint.y(),
                       bottomPoint.z());
            glVertex3f(bottomPenetrationPoint.x(),
                       bottomPenetrationPoint.y(),
                       bottomPenetrationPoint.z());

            //TopPivot->TopMidExtensionPoin
            glColor3f(1.0f,1.0f,0.0f);
            glVertex3f(topPivot.x(),
                       topPivot.y(),
                       topPivot.z());
            glVertex3f(topMidExtensionPoint.x(),
                       topMidExtensionPoint.y(),
                       topMidExtensionPoint.z());


    glEnd();

    glEnable(GL_LIGHTING);
}

void B9SupportStructure::SetVisible(bool vis)
{
    isVisible = vis;
}


//Baking
//using all rotations scales positions etc, generates the list of triangles that
//is represented by the rendered shapes.
//similar to instance baking.
//remember that this function needed to always reflect what render() does but inverted.
//returns number of triangles added
unsigned int B9SupportStructure::BakeToInstanceGeometry()
{
    unsigned int t;
    unsigned short int v;
    unsigned int trisAdded = 0;
    Triangle3D* pNewTri;
    QVector3D topScale;
    QVector3D topPos;
    QVector3D midScale;
    QVector3D midPos;
    QVector3D bottomScale;
    QVector3D bottomPos;


    //first bake the top
    topScale = QVector3D(topRadius*2.0,topRadius*2.0,topLength + topPenetration);
    topPos = QVector3D((topPenetrationPoint.x() + topPivot.x())*0.5
                          ,(topPenetrationPoint.y() + topPivot.y())*0.5
                          ,(topPenetrationPoint.z() + topPivot.z())*0.5);
    if(topAttachShape != NULL)
    for(t = 0; t < topAttachShape->GetTriangles()->size(); t++)
    {
        pNewTri = new Triangle3D(topAttachShape->GetTriangles()->at(t));


        for(v=0;v<3;v++)
        {
            //scale triangle vertices 1st
            pNewTri->vertex[v] *= topScale;

            //Rotate 2nd
            //dont deal with y - not needed in rendering or baking
            RotateVector(pNewTri->vertex[v], topThetaX, QVector3D(1,0,0));//x
            RotateVector(pNewTri->vertex[v], topThetaZ, QVector3D(0,0,1));//z

            //Translate 3rd into global space
            pNewTri->vertex[v] += topPos;
            pNewTri->vertex[v] += instanceParent->GetPos();
        }

        //Update the triangle bounds and normal
        pNewTri->UpdateBounds();
        pNewTri->UpdateNormalFromGeom();

        //Add To List
        instanceParent->triList.push_back(pNewTri);
        trisAdded++;
    }

    //second bake the middle
    midScale = QVector3D(midRadius*2.0,midRadius*2.0,midLength);
    midPos = QVector3D((topMidExtensionPoint.x() + bottomMidExtensionPoint.x())*0.5
                      ,(topMidExtensionPoint.y() + bottomMidExtensionPoint.y())*0.5
                      ,(topMidExtensionPoint.z() + bottomMidExtensionPoint.z())*0.5);

    if(midAttachShape != NULL)
    for(t = 0; t < midAttachShape->GetTriangles()->size(); t++)
    {
        pNewTri = new Triangle3D(midAttachShape->GetTriangles()->at(t));



        for(v=0;v<3;v++)
        {
            //scale triangle vertices 1st
            pNewTri->vertex[v] *= midScale;

            //Rotate 2nd
            //dont deal with y - not needed in rendering or baking
            RotateVector(pNewTri->vertex[v], midThetaX, QVector3D(1,0,0));//x
            RotateVector(pNewTri->vertex[v], midThetaZ, QVector3D(0,0,1));//z

            //Translate 3rd into global space
            pNewTri->vertex[v] += midPos;
            pNewTri->vertex[v] += instanceParent->GetPos();
        }

        //Update the triangle bounds and normal
        pNewTri->UpdateBounds();
        pNewTri->UpdateNormalFromGeom();

        //Add To List
        instanceParent->triList.push_back(pNewTri);
        trisAdded++;
    }

    //third bake the bottom
    bottomScale = QVector3D(bottomRadius*2.0,bottomRadius*2.0,bottomLength + bottomPenetration);
    bottomPos = QVector3D((bottomPenetrationPoint.x() + bottomPivot.x())*0.5
                          ,(bottomPenetrationPoint.y() + bottomPivot.y())*0.5
                          ,(bottomPenetrationPoint.z() + bottomPivot.z())*0.5);

    if(bottomAttachShape != NULL)
    for(t = 0; t < bottomAttachShape->GetTriangles()->size(); t++)
    {
        pNewTri = new Triangle3D(bottomAttachShape->GetTriangles()->at(t));



        for(v=0;v<3;v++)
        {
            //scale triangle vertices 1st
            pNewTri->vertex[v] *= bottomScale;

            //Rotate 2nd
            //dont deal with y - not needed in rendering or baking
            RotateVector(pNewTri->vertex[v], bottomThetaX+180, QVector3D(1,0,0));//x
            RotateVector(pNewTri->vertex[v], bottomThetaZ, QVector3D(0,0,1));//z

            //Translate 3rd into global space
            pNewTri->vertex[v] += bottomPos;
            pNewTri->vertex[v] += instanceParent->GetPos();
        }

        //Update the triangle bounds and normal
        pNewTri->UpdateBounds();
        pNewTri->UpdateNormalFromGeom();

        //Add To List
        instanceParent->triList.push_back(pNewTri);
        trisAdded++;
    }

    return trisAdded;
}





//Private
//////////////////////////////////////////////////////////
void B9SupportStructure::ReBuildGeometry()
{


}
void B9SupportStructure::ForceUpdate()
{
    QVector3D topFinalNormal;//result of combinging angle factor with source normal.
    QVector3D bottomFinalNormal;
    QVector3D alongMidDownNormal;
    QVector3D alongMidUpNormal;
    QVector3D topPivotCross;
    QVector3D bottomPivotCross;


    if(isGrounded)
    {
        bottomPoint.setZ(-instanceParent->GetPos().z());
    }

    length = Distance3D(GetTopPoint(),GetBottomPoint());

    //Compute real normal using angle factors
    topFinalNormal = topNormal;
    topFinalNormal.setX(topNormal.x()*topAngleFactor);
    topFinalNormal.setY(topNormal.y()*topAngleFactor);
    topFinalNormal.normalize();

    bottomFinalNormal = bottomNormal;
    bottomFinalNormal.setX(bottomNormal.x()*bottomAngleFactor);
    bottomFinalNormal.setY(bottomNormal.y()*bottomAngleFactor);
    bottomFinalNormal.normalize();


    //Compute pivot positions
    topPivot = topPoint - topFinalNormal*topLength;
    bottomPivot = bottomPoint - bottomFinalNormal*bottomLength;


    //Compute midExtensions based on normal angles,
    //these extensions are to cover up top and botton shape gaps.
    alongMidDownNormal = (bottomPivot - topPivot);
    alongMidDownNormal.normalize();
    topPivotCross = QVector3D::crossProduct(topFinalNormal,alongMidDownNormal);
    topMidExtension = topPivotCross.length()*topRadius;
    topMidExtensionPoint = topPivot - alongMidDownNormal*topMidExtension;

    alongMidUpNormal = (topPivot - bottomPivot);
    alongMidUpNormal.normalize();
    bottomPivotCross = QVector3D::crossProduct(bottomFinalNormal,alongMidUpNormal);
    if(isGrounded)
    {
        bottomMidExtension = midRadius*2*bottomPivotCross.length();
    }
    else
        bottomMidExtension = bottomPivotCross.length()*bottomRadius;


    bottomMidExtensionPoint = bottomPivot - alongMidUpNormal*bottomMidExtension;



    //True rendered mid length
    midLength = Distance3D(topMidExtensionPoint,bottomMidExtensionPoint);



    //Compute PenetrationPoints (the true end points that are rendered/baked)
    topPenetrationPoint = topPoint + topFinalNormal*topPenetration;
    bottomPenetrationPoint = bottomPoint + bottomFinalNormal*bottomPenetration;


    //Compute Euler rotations for top, mid, and bottom shapes
    topThetaX = qAcos(topFinalNormal.z())*(180/M_PI);
    topThetaZ = qAtan2(topFinalNormal.y(),topFinalNormal.x())*(180/M_PI)-90;

    if(topThetaZ <= 90)
        topThetaZ += 180;

    midThetaX = qAcos((topMidExtensionPoint.z() - bottomMidExtensionPoint.z())/midLength)*(180/M_PI);
    midThetaZ = qAtan2((topMidExtensionPoint.y() - bottomMidExtensionPoint.y()),
                       (topMidExtensionPoint.x() - bottomMidExtensionPoint.x()))*(180/M_PI)-90;

    if(midThetaZ <= 90)
        midThetaZ += 180;


    bottomThetaX = qAcos(bottomFinalNormal.z())*(180/M_PI)+180;
    bottomThetaZ = qAtan2(bottomFinalNormal.y(),bottomFinalNormal.x())*(180/M_PI)-90;

    if(bottomThetaZ <= 90)
        bottomThetaZ += 180;



}





