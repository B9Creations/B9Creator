#include "b9printermodeldata.h"
#include "b9material.h"
#include <QString>


//////////////////////////////////////////////////////////////////////////
//PrinterModelData Class
b9PrinterModelData::b9PrinterModelData()
{
    m_sModelName = "";

    m_dStepSizeMicrons = 0;
    m_iMaxSteps = 0;

    B9Material startMat;
    AddMaterial(startMat);
}
b9PrinterModelData::b9PrinterModelData(QString modelName)
{
    m_sModelName = modelName;

    m_dStepSizeMicrons = 0;
    m_iMaxSteps = 0;

    B9Material startMat;
    AddMaterial(startMat);
}
b9PrinterModelData::~b9PrinterModelData()
{
}


QString b9PrinterModelData::GetName() const
{
    return m_sModelName;
}


void b9PrinterModelData::ClearMaterials()
{
    m_Materials.clear();
}


void b9PrinterModelData::AddMaterial(B9Material mat)
{
    m_Materials.push_back(mat);
}


QVector<B9Material> *b9PrinterModelData::GetMaterials()
{
    return &m_Materials;
}

B9Material* b9PrinterModelData::FindMaterialByLabel(QString label)
{
    for(int i = 0; i < this->m_Materials.size(); i++)
    {
        if(m_Materials[i].getLabel() == label)
        {
            return &m_Materials[i];
        }
    }
    return NULL;
}

double b9PrinterModelData::GetXYSizeByIndex(int index)
{
    if((m_dXYPixelSizes.size() - 1) < index)
        return -1;
    else
        return m_dXYPixelSizes.at(index);
}


