#include "b9printermodelmanager.h"
#include "b9printermodeldata.h"

#include <QDebug>
#include <QSettings>

b9PrinterModelManager::b9PrinterModelManager(QObject *parent) :
    QObject(parent)
{
    this->SetCurrentOperatingPrinter(NULL);
}

b9PrinterModelManager::~b9PrinterModelManager()
{

}
b9PrinterModelData* b9PrinterModelManager::GetCurrentOperatingPrinter()
{
    return m_CurrentOperatingPrinter;
}

std::vector<b9PrinterModelData*> b9PrinterModelManager::GetPrinterModels()
{
    return m_PrinterDataList;
}

bool b9PrinterModelManager::SetCurrentOperatingPrinter(b9PrinterModelData* modelDataPtr)
{
    if(modelDataPtr == NULL) return false;

    m_CurrentOperatingPrinter = modelDataPtr;

    return true;
}

bool b9PrinterModelManager::SetCurrentOperatingPrinter(QString modelName)
{
    QSettings appSettings;

    if(FindPrinterDataByName(modelName) != NULL)
    {
        appSettings.beginGroup("PRINTER_DATA_MANAGER");
            appSettings.setValue("LATEST_OPERATING_PRINTER_NAME",modelName);
        appSettings.endGroup();
        return true;
    }
    else
    {
        qDebug() << "b9PrinterModelManager-SetCurrentOperatingPrinter: Unable To Find Printer By Name Of: " << modelName;
        return false;
    }
}

b9PrinterModelData* b9PrinterModelManager::FindPrinterDataByName(QString modelName)
{
    for(unsigned int i = 0; i < m_PrinterDataList.size(); i++)
    {
        if(m_PrinterDataList.at(i)->GetName() == modelName)
            return m_PrinterDataList[i];
    }

    return NULL;
}

b9PrinterModelData* b9PrinterModelManager::AddPrinterData(QString modelName)
{
   if(FindPrinterDataByName(modelName) != NULL)
        return NULL;

   b9PrinterModelData* newPrinterData = new b9PrinterModelData(modelName);
   m_PrinterDataList.push_back(newPrinterData);

   return m_PrinterDataList[m_PrinterDataList.size()-1];
}

void b9PrinterModelManager::ImportDefinitions(QString defFilePath)
{

}





