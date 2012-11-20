#ifndef B9MATCAT_H
#define B9MATCAT_H

#include <QObject>
#include <QDataStream>

#define XYCOUNT 3
#define ZCOUNT 16
#define TCOUNT 2

class B9MatCatItem
{
public:
    B9MatCatItem();

    QString getMaterialLabel();
    bool isFactoryEntry();
    QString getFactoryMaterialLabel(){return m_sMaterialLabel;}
    QString getMaterialDescription(){return m_sMaterialDescription;}
    void setMaterialLabel(QString sLabel){m_sMaterialLabel = sLabel;}
    void setMaterialDescription(QString sLabel){m_sMaterialDescription = sLabel;}

    double getTattach(int iXY){return m_aAttachTimes[iXY];}
    double getTbase(int iXY, int iZ){return m_aTimes[iXY][iZ][0];}
    double getTover(int iXY, int iZ){return m_aTimes[iXY][iZ][1];}
    void setTattach(int iXY, double dSeconds){m_aAttachTimes[iXY]=dSeconds;}
    void setTbase(int iXY, int iZ, double dSeconds){m_aTimes[iXY][iZ][0]=dSeconds;}
    void setTover(int iXY, int iZ, double dSeconds){m_aTimes[iXY][iZ][1]=dSeconds;}

private:
    void initDefaults();
    double m_aTimes [XYCOUNT][ZCOUNT][TCOUNT]; // Cure times in seconds
    double m_aAttachTimes [XYCOUNT]; // Cure times in seconds for first layer
    QString m_sMaterialLabel, m_sMaterialDescription;

public: // sorting friends
    friend class MatLessThan;
};
class MatLessThan
{
public:
    MatLessThan(  ){}
    bool operator()(const B9MatCatItem *left, const B9MatCatItem *right ) const;
};

///////////////////////////////////////////////////////////////////////////
class B9MatCat : public QObject
{
    Q_OBJECT
public:
    explicit B9MatCat(QObject *parent = 0);

    int getMaterialCount(){return m_Materials.count();}
    int getXYCount(){return XYCOUNT;}
    int getZCount(){return ZCOUNT;}

    QString getFactoryMaterialLabel(int index){return m_Materials[index]->getFactoryMaterialLabel();}
    QString getMaterialLabel(int index){return m_Materials[index]->getMaterialLabel();}
    bool isFactoryEntry(int iMat){return m_Materials[iMat]->isFactoryEntry();}
    QString getMaterialDescription(int index){return m_Materials[index]->getMaterialDescription();}

    QString getTattach(int iMat, int iXY) {return QString::number(m_Materials[iMat]->getTattach(iXY));}
    QString getTbase(int iMat, int iXY, int iZ){return QString::number(m_Materials[iMat]->getTbase(iXY, iZ));}
    QString getTover(int iMat, int iXY, int iZ){return QString::number(m_Materials[iMat]->getTover(iXY, iZ));}

    void setTattach(int iMat, int iXY,double dT){m_Materials[iMat]->setTattach(iXY, dT);}
    void setTbase(int iMat, int iXY, int iZ, double dT){m_Materials[iMat]->setTbase(iXY, iZ, dT);}
    void setTover(int iMat, int iXY, int iZ, double dT){m_Materials[iMat]->setTover(iXY, iZ, dT);}

    QString getCurTattach(){return QString::number(m_Materials[m_iCurMatIndex]->getTattach(m_iCurXYIndex));}
    QString getCurTbaseAtZ(int iZ){return QString::number(m_Materials[m_iCurMatIndex]->getTbase(m_iCurXYIndex, iZ));}
    QString getCurToverAtZ(int iZ){return QString::number(m_Materials[m_iCurMatIndex]->getTover(m_iCurXYIndex, iZ));}

    QString getModelName(){return m_sModelName;}

    QString getXYLabel(int iXY);
    double getXYinMM(int iXY);
    QString getZLabel(int iZ);
    double getZinMM(int iZ);

    void deleteMaterial(int iMat){m_Materials.removeAt(iMat);}

    void setCurMatIndex(int iM){m_iCurMatIndex=iM;}
    void setCurXYIndex(int iXY){m_iCurXYIndex=iXY;}
    void setCurZIndex(int iZ){m_iCurZIndex=iZ;}
    int getCurMatIndex(){return m_iCurMatIndex;}
    int getCurXYIndex(){return m_iCurXYIndex;}
    int getCurZIndex(){return m_iCurZIndex;}

    int getCurTbaseAtZinMS(double zMM);
    int getCurToverAtZinMS(double zMM);

signals:

public slots:
    void addMaterial(QString sName, QString sDescription);
    void addDupMat(QString sName, QString sDescription,int iOriginIndex);
    void clear();
    bool load(QString sModelName);
    bool save();

private:
    QList<B9MatCatItem*> m_Materials;
    QString m_sModelName;
    void streamOut(QDataStream* pOut);
    void streamIn(QDataStream* pIn);

    int m_iCurMatIndex;
    int m_iCurXYIndex;
    int m_iCurZIndex;

};

#endif // B9MATCAT_H
