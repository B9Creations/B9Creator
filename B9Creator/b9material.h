#ifndef B9MATERIAL_H
#define B9MATERIAL_H

#include <QString>
#include <QVector>


struct XYData
{
    double size;
    int attachmentLayers;
    double attachmentLayersCureTime;
    QVector<double> cure_times;
    QVector<double> over_cure_times;
};


class B9Material
{
public:
    B9Material();
    ~B9Material();
    QString getLabel();
    void SetLabel(QString newLabel){m_sMaterialLabel = newLabel;}
    QString GetDescription(){return m_sMaterialDescription;}
    void SetDescription(QString newDesc){m_sMaterialDescription = newDesc;}
    void AddXYSize(double xySize);
    void SetXYAttachmentCureTime(double xySize, double time_s);
    void SetXYAttachmentLayers(double xySize,  int numberOfLayers);

    double GetXYAttachmentCureTime(double xySize);





    bool isFactoryEntry();



private:

    QString m_sMaterialLabel, m_sMaterialDescription;
    QVector<XYData> XYSizes;

    XYData* FindXYData(double xySize);



};


#endif // B9MATERIAL_H
