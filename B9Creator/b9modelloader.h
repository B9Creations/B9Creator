#ifndef B9MODELLOADER_H
#define B9MODELLOADER_H

#include <QObject>
#include <QFile>
#include <QTextStream>


//simple structure to read triangles into
//TODO assuming floats are 32bits on the compiling platform
struct STLTri
{
    float nx,ny,nz;
    //by stl definition,
    //the model is supposed to be in positive cordinates only
    //but were using unsigned anyway...
    float x0,y0,z0;
    float x1,y1,z1;
    float x2,y2,z2;
};


class B9ModelLoader : public QObject
{
    Q_OBJECT
public:
    explicit B9ModelLoader( QString filename, bool &readyRead, QObject *parent = 0);
    ~B9ModelLoader();

    bool LoadNextTri(STLTri *&tri, bool &errorFlag);
    float GetPercentDone();
    QString GetError(){return lastError;}//returns any errors that happened in the loading process.


signals:
    void PercentCompletedUpdate(qint64 frac, qint64 total);
    
public slots:

private:

    QString fileType;// "BIN_STL", "ASCII_STL", "UNCOMPRESSED_AMF"......
    QFile asciifile;
    QTextStream asciiStream;
    QFile binfile;

    quint32 triCount;
    unsigned long int byteCount;

    QString lastError;

    float lastpercent;
    QString DetermineFileType(QString filename, bool &readyRead);
    void PrepareSTLForReading(QString filename, bool &readyRead);
    void PrepareAMFForReading(){}
    bool ReadBinHeader();
    bool ReadAsciiHeader();
    bool CheckBinFileValidity();
};

#endif // B9MODELLOADER_H
