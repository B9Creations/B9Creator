#ifndef B9UPDATEENTRY_H
#define B9UPDATEENTRY_H
#include <QString>




struct B9UpdateEntry
{
    QString localLocationTag;
    QString fileName;
    int version;
    QString OSDir;//used flag to tell us to download from sub dir.
};


#endif // B9UPDATEENTRY_H
