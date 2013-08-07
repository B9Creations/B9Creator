#ifndef B9PRINTERMODELMANAGER_H
#define B9PRINTERMODELMANAGER_H

#include <QObject>
#include <QVector>
#include "b9printermodeldata.h"

class QString;


class b9PrinterModelManager : public QObject
{
    Q_OBJECT
public:
    explicit b9PrinterModelManager(QObject *parent = 0);
    ~b9PrinterModelManager();

    void ImportDefinitions(QString defFilePath);//imports from given definition file

    b9PrinterModelData* GetCurrentOperatingPrinter();
    std::vector<b9PrinterModelData*> GetPrinterModels();
    bool SetCurrentOperatingPrinter(b9PrinterModelData* modelDataPtr);
    bool SetCurrentOperatingPrinter(QString modelName);
    b9PrinterModelData* FindPrinterDataByName(QString modelName);





signals:
    void FilesUpdated();
    
public slots:


private:
    //members
    std::vector<b9PrinterModelData*> m_PrinterDataList;
    b9PrinterModelData* m_CurrentOperatingPrinter;

    //functions
    b9PrinterModelData* AddPrinterData(QString modelName);





};

#endif // B9PRINTERMODELMANAGER_H
