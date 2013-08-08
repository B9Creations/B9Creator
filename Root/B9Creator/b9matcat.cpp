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

#include <QApplication>
#include <QFile>
#include "b9matcat.h"
#include "OS_Wrapper_Functions.h"
#include <QSettings>

B9MatCatItem::B9MatCatItem()
{
    m_sMaterialLabel = "Name";
    m_sMaterialDescription = "Description";
    initDefaults();
}

void B9MatCatItem::initDefaults()
{
    for(int xy = 0; xy < XYCOUNT; xy ++){
        m_aAttachTimes[xy] = 1.0;
        m_aAttachNumber[xy] = 1;
        for(int z = 0; z < ZCOUNT; z++)
            for(int t = 0; t < TCOUNT; t++)
                m_aTimes[xy][z][t] = 0.0; // Set all to unused
    }

}

bool B9MatCatItem::isFactoryEntry()
{
    return m_sMaterialLabel.left(2)=="!@";
}

QString B9MatCatItem::getMaterialLabel()
{
    if(isFactoryEntry())return m_sMaterialLabel.right(m_sMaterialLabel.count()-2);
    return m_sMaterialLabel;
}

bool MatLessThan::operator()(const B9MatCatItem *left, const B9MatCatItem *right ) const
{
    QString sL = left->m_sMaterialLabel;
    QString sR = right->m_sMaterialLabel;
    if(sL.left(2)=="!@") sL.right(sL.count()-2);
    if(sR.left(2)=="!@") sR.right(sL.count()-2);
    return sL.toLower() < sR.toLower();
}

//////////////////////////////
B9MatCat::B9MatCat(QObject *parent) :
    QObject(parent)
{
    clear();
}

bool B9MatCat::load(QString sModelName)
{
    clear();//clear out all material int the class first!
    m_Materials.clear();
    m_sModelName = sModelName;
    QString sPath;

    #ifndef Q_OS_LINUX
    //See if there is an old matfile with user defined materials in
    //it and stuff them in the register. THIS IF STATEMENT SHOULD HAPPEN ONLY ONCE EVER
    if(QFile::exists(CROSS_OS_GetDirectoryFromLocationTag("EXECUTABLE_DIR")+"/"+m_sModelName+".b9m"))
    {
        sPath = CROSS_OS_GetDirectoryFromLocationTag("EXECUTABLE_DIR")+"/"+m_sModelName+".b9m";
        QFile inFile(sPath);
        inFile.open(QIODevice::ReadOnly);
        if(!inFile.isOpen()) return false;
        QDataStream inStream(&inFile);
        streamIn(&inStream);

        //write the user ones to registry
        SaveUserMaterialsToRegister();

        inFile.close();
        inFile.remove();

        load(sModelName);//Now do the standard load. (clears out any old official stuff)
    }
    #endif

    sPath = CROSS_OS_GetDirectoryFromLocationTag("APPLICATION_DIR")+"/"+m_sModelName+".b9m";



    QFile inFile(sPath);
    inFile.open(QIODevice::ReadOnly);
    if(!inFile.isOpen()) return false;
    QDataStream inStream(&inFile);
    streamIn(&inStream);
    inFile.close();

    LoadUserMaterialsFromRegister();

    return true;
}

bool B9MatCat::save()
{
    QString sPath = CROSS_OS_GetDirectoryFromLocationTag("APPLICATION_DIR")+"/"+m_sModelName+".b9m";
    QFile outFile(sPath);
    outFile.open(QIODevice::WriteOnly);
    if(!outFile.isOpen()) return false;
    QDataStream outStream(&outFile);
    streamOut(&outStream);
    //at this point ffactory mats have been saved to bin file


    //write the user ones to registry
    SaveUserMaterialsToRegister();

    return true;
}

//stream out to the binary mat catalog file,
//only stream out NON user created mats
//Added in v15
void B9MatCat::streamOut(QDataStream* pOut)
{
    unsigned int numFactoryMats = 0;
    qSort(m_Materials.begin(), m_Materials.end(), MatLessThan());

    for(int i=0; i<m_Materials.count(); i++){
        if(m_Materials[i]->isFactoryEntry())
            numFactoryMats++;
    }

    //header
    *pOut << (quint32)numFactoryMats;
    //body
    for(int i=0; i<m_Materials.count(); i++)
    {
        if(m_Materials[i]->isFactoryEntry())
        {
            *pOut << m_Materials[i]->getFactoryMaterialLabel() << m_Materials[i]->getMaterialDescription();
            for(int xy = 0; xy < XYCOUNT; xy++)
            {
                *pOut << m_Materials[i]->getTattach(xy);
                *pOut << m_Materials[i]->getNumberAttach(xy);
                for(int z = 0; z < ZCOUNT; z++)
                    *pOut << m_Materials[i]->getTbase(xy,z) << m_Materials[i]->getTover(xy,z);
            }
        }
    }
}

void B9MatCat::streamIn(QDataStream* pIn)
{
    m_Materials.clear();
    QString s1, s2;
    double d0, d1, d2;
    int iNum;
    int iCount = 0;
    *pIn >> iCount;
    for(int i=0; i<iCount; i++){
        m_Materials.append(new B9MatCatItem);
        *pIn >> s1 >> s2;
        m_Materials[i]->setMaterialLabel(s1);
        m_Materials[i]->setMaterialDescription(s2);
        for(int xy = 0; xy < XYCOUNT; xy++){
            *pIn >> d0 >> iNum;
            m_Materials[i]->setTattach(xy,d0);
            m_Materials[i]->setNumberAttach(xy,iNum);
            for(int z = 0; z < ZCOUNT; z++){
                *pIn >> d1 >> d2;
                m_Materials[i]->setTbase(xy,z,d1);
                m_Materials[i]->setTover(xy,z,d2);
            }
        }
    }

}

void B9MatCat::clear()
{
    m_iCurMatIndex=0;
    m_iCurXYIndex=0;
    m_iCurZIndex=0;
    m_sModelName = "Untitled";
    m_Materials.clear();
    m_Materials.append(new B9MatCatItem);
    m_Materials[0]->setMaterialLabel("Untitled Material");
    m_Materials[0]->setMaterialDescription("New Material - please enter data");
    for(int xy = 0; xy < XYCOUNT; xy++)
    {
        m_Materials[0]->setTattach(xy,1.0);
        m_Materials[0]->setNumberAttach(xy,1);
        for(int z = 0; z < ZCOUNT; z++){
            m_Materials[0]->setTbase(xy,z,0.0);
            m_Materials[0]->setTover(xy,z,0.0);
        }
    }
}
QString B9MatCat::getXYLabel(int iXY)
{
    QString sLabel = "??";
    switch (iXY){
    case 0:
        sLabel = "50 (µm)";
        break;
    case 1:
        sLabel = "75 (µm)";
        break;
    case 2:
        sLabel = "100 (µm)";
        break;
    default:
        break;
    }
    return sLabel;
}

double B9MatCat::getXYinMM(int iXY)
{
    double dValue = -1.0;
    switch (iXY){
    case 0:
        dValue = 0.050;
        break;
    case 1:
        dValue = 0.075;
        break;
    case 2:
        dValue = 0.100;
        break;
    default:
        break;
    }
    return dValue;
}

QString B9MatCat::getZLabel(int iZ)
{
    QString sLabel = "??";
    double dZ = getZinMM(iZ)*1000;
    if(dZ<0)return sLabel;
    return QString::number(dZ,'f',2)+" (µm)";
}

double B9MatCat::getZinMM(int iZ)
{
    // return valid increments for iZ values of 0-15
    if(iZ<0||iZ>15)return -1.0;
    return (iZ+1) * 0.00635;
}

void B9MatCat::addMaterial(QString sName, QString sDescription){
    B9MatCatItem* pNew = new B9MatCatItem;
    pNew->setMaterialLabel(sName);
    pNew->setMaterialDescription(sDescription);
    m_Materials.append(pNew);
}

void B9MatCat::addDupMat(QString sName, QString sDescription, int iOrigin){
    B9MatCatItem* pNew = new B9MatCatItem;
    pNew->setMaterialLabel(sName);
    pNew->setMaterialDescription(sDescription);
    for(int xy = 0; xy < XYCOUNT; xy ++){
        pNew->setTattach(xy,m_Materials[iOrigin]->getTattach(xy));
        pNew->setNumberAttach(xy,m_Materials[iOrigin]->getNumberAttach(xy));
        for(int z = 0; z < ZCOUNT; z++){
            pNew->setTbase(xy, z, m_Materials[iOrigin]->getTbase(xy,z));
            pNew->setTover(xy, z, m_Materials[iOrigin]->getTover(xy,z));
        }
    }
    m_Materials.append(pNew);
}

int B9MatCat::getCurTbaseAtZinMS(double zMM){
    double dLowTime, dHighTime;
    double lowMatch = getZinMM(0);
    double highMatch = getZinMM(15);
    for(int i=0;i <16; i++){
        if(getZinMM(i)<=zMM){lowMatch = getZinMM(i);dLowTime = getTbase(m_iCurMatIndex, m_iCurXYIndex, i).toDouble();}
        if(getZinMM(i)>=zMM){highMatch = getZinMM(i);dHighTime = getTbase(m_iCurMatIndex, m_iCurXYIndex, i).toDouble();break;}
    }
    if(dHighTime==dLowTime)return 1000*dHighTime;
    return 1000.0*((((zMM-lowMatch)/(highMatch-lowMatch))*(dHighTime - dLowTime))+ dLowTime);
}

int B9MatCat::getCurToverAtZinMS(double zMM){
    double dLowTime, dHighTime;
    double lowMatch = getZinMM(0);
    double highMatch = getZinMM(15);
    for(int i=0;i <16; i++){
        if(getZinMM(i)<=zMM){lowMatch = getZinMM(i);dLowTime = getTover(m_iCurMatIndex, m_iCurXYIndex, i).toDouble();}
        if(getZinMM(i)>=zMM){highMatch = getZinMM(i);dHighTime = getTover(m_iCurMatIndex, m_iCurXYIndex, i).toDouble();break;}
    }
    if(dHighTime==dLowTime)return dHighTime*1000.0;
    return 1000.0*((((zMM-lowMatch)/(highMatch-lowMatch))*(dHighTime - dLowTime))+ dLowTime);
}



void B9MatCat::SaveUserMaterialsToRegister()
{
    QSettings appSettings;

    int m;
    int xy;
    int l;
    int validMatCount = 0;
    appSettings.beginGroup("USERMATS");

        //first sort the materials list.
        qSort(m_Materials.begin(), m_Materials.end(), MatLessThan());


        for(m = 0; m < m_Materials.size(); m++)
        {
            //we dont want to save factory entrees
            if(!m_Materials[m]->isFactoryEntry())
                validMatCount++;
            else
            {
                continue;
            }



            //write label.
            appSettings.setValue("M_" + QString::number(validMatCount-1) + "_LABEL",m_Materials[m]->getMaterialLabel());
            //write description
            appSettings.setValue("M_" + QString::number(validMatCount-1) + "_DESC",m_Materials[m]->getMaterialDescription());

            //write xy sizes count
            appSettings.setValue("M_" + QString::number(validMatCount-1) + "_XYCOUNT",XYCOUNT);
            //for flexibility.

            for(xy = 0; xy < XYCOUNT; xy++)
            {
                //write time Attach
                appSettings.setValue("M_" + QString::number(validMatCount-1) + "_" + QString::number(xy) + "_TATTACH",m_Materials[m]->getTattach(xy));
                //write number of attach
                appSettings.setValue("M_" + QString::number(validMatCount-1) + "_" + QString::number(xy) + "_NUMATTACH",m_Materials[m]->getNumberAttach(xy));

                appSettings.setValue("M_" + QString::number(validMatCount-1) + "_" + QString::number(xy) + "_LAYCOUNT",ZCOUNT);//TODO
                //ZCOUNT should be member of m_Materials[m]
                //for flexibility.
                for(l = 0; l < ZCOUNT; l++)
                {
                    //write time base
                    appSettings.setValue("M_" + QString::number(validMatCount-1) + "_" + QString::number(xy) + "_" +
                                         QString::number(l) + "_TBASE",m_Materials[m]->getTbase(xy,l));

                    //write time overcure
                    appSettings.setValue("M_" + QString::number(validMatCount-1) + "_" + QString::number(xy) + "_" +
                                         QString::number(l) + "_TOVER",m_Materials[m]->getTover(xy,l));
                }
            }
        }

        appSettings.setValue("NUMMAT",validMatCount);

    appSettings.endGroup();
}




int B9MatCat::LoadUserMaterialsFromRegister()
{
    QSettings appSettings;
    int MaterialCount;
    int XYCount;
    int LAYCount;
    int m;
    int xy;
    int l;
    double TBase;
    double TOver;
    double AttachTime;
    int AttachNum;
    QString MatLabel;
    QString MatDesc;


    const int invalidKey = -1;

    appSettings.beginGroup("USERMATS");

    MaterialCount = appSettings.value("NUMMAT",invalidKey).toInt();
    if(MaterialCount <= 0)
        return MaterialCount;//nothing to read in from registery


    for(m = 0; m < MaterialCount; m++)
    {
        MatLabel = appSettings.value("M_" + QString::number(m) + "_LABEL",invalidKey).toString();
        MatDesc = appSettings.value("M_" + QString::number(m) + "_DESC",invalidKey).toString();

        m_Materials.append(new B9MatCatItem);
        m_Materials.back()->setMaterialDescription(MatDesc);
        m_Materials.back()->setMaterialLabel(MatLabel);


        XYCount = appSettings.value("M_" + QString::number(m) + "_XYCOUNT",invalidKey).toInt();

        for(xy = 0; xy < XYCount; xy++)
        {
            AttachTime = appSettings.value("M_" + QString::number(m) + "_" + QString::number(xy) +
                                           "_TATTACH",invalidKey).toDouble();
            AttachNum = appSettings.value("M_" + QString::number(m) + "_" + QString::number(xy) +
                                          "_NUMATTACH",invalidKey).toInt();
            m_Materials.back()->setTattach(xy,AttachTime);
            m_Materials.back()->setNumberAttach(xy,AttachNum);

            LAYCount = appSettings.value("M_" + QString::number(m) + "_" + QString::number(xy) + "_LAYCOUNT",invalidKey).toInt();
            for(l = 0; l < LAYCount; l++)
            {

                TBase = appSettings.value("M_" + QString::number(m) + "_" + QString::number(xy) + "_" +
                                          QString::number(l) + "_TBASE",invalidKey).toDouble();
                TOver = appSettings.value("M_" + QString::number(m) + "_" + QString::number(xy) + "_" +
                                          QString::number(l) + "_TOVER",invalidKey).toDouble();
                m_Materials.back()->setTbase(xy,l,TBase);
                m_Materials.back()->setTover(xy,l,TOver);
            }
        }
    }
    appSettings.endGroup();

    return MaterialCount;
}
