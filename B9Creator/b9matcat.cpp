#include <QFile>
#include "b9matcat.h"

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
    clear();
    m_Materials.clear();
    m_sModelName = sModelName;
    QString sPath = m_sModelName+".b9m";

    QFile inFile(sPath);
    inFile.open(QIODevice::ReadOnly);
    if(!inFile.isOpen()) return false;
    QDataStream inStream(&inFile);
    streamIn(&inStream);
    return true;
}

bool B9MatCat::save()
{
    QString sPath = m_sModelName+".b9m";
    QFile outFile(sPath);
    outFile.open(QIODevice::WriteOnly);
    if(!outFile.isOpen()) return false;
    QDataStream outStream(&outFile);
    streamOut(&outStream);
    return true;
}

void B9MatCat::streamOut(QDataStream* pOut)
{
    qSort(m_Materials.begin(), m_Materials.end(), MatLessThan());
    *pOut << (quint32)m_Materials.count();
    for(int i=0; i<m_Materials.count(); i++){
        *pOut << m_Materials[i]->getFactoryMaterialLabel() << m_Materials[i]->getMaterialDescription();
        for(int xy = 0; xy < XYCOUNT; xy++){
            *pOut << m_Materials[i]->getTattach(xy);
            *pOut << m_Materials[i]->getNumberAttach(xy);
            for(int z = 0; z < ZCOUNT; z++)
                *pOut << m_Materials[i]->getTbase(xy,z) << m_Materials[i]->getTover(xy,z);
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
    int iLowTime, iHighTime;
    double lowMatch = getZinMM(0);
    double highMatch = getZinMM(15);
    for(int i=0;i <16; i++){
        if(getZinMM(i)<=zMM){lowMatch = getZinMM(i);iLowTime = getTbase(m_iCurMatIndex, m_iCurXYIndex, i).toInt();}
        if(getZinMM(i)>=zMM){highMatch = getZinMM(i);iHighTime = getTbase(m_iCurMatIndex, m_iCurXYIndex, i).toInt();break;}
    }
    if(iHighTime==iLowTime)return (double)iHighTime*1000.0;
    return 1000.0*((((zMM-lowMatch)/(highMatch-lowMatch))*(double)(iHighTime - iLowTime))+ (double)iLowTime);
}

int B9MatCat::getCurToverAtZinMS(double zMM){
    int iLowTime, iHighTime;
    double lowMatch = getZinMM(0);
    double highMatch = getZinMM(15);
    for(int i=0;i <16; i++){
        if(getZinMM(i)<=zMM){lowMatch = getZinMM(i);iLowTime = getTover(m_iCurMatIndex, m_iCurXYIndex, i).toInt();}
        if(getZinMM(i)>=zMM){highMatch = getZinMM(i);iHighTime = getTover(m_iCurMatIndex, m_iCurXYIndex, i).toInt();break;}
    }
    if(iHighTime==iLowTime)return (double)iHighTime*1000.0;
    return 1000.0*((((zMM-lowMatch)/(highMatch-lowMatch))*(double)(iHighTime - iLowTime))+ (double)iLowTime);
}
