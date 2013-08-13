#ifndef B9TESSELATOR_H
#define B9TESSELATOR_H

#ifndef CALLBACK
#define CALLBACK
#endif

#include <QVector2D>
#include "loop.h"
#include <vector>
#include <QtOpenGL>
#include <OS_GL_Wrapper.h>


class B9Tesselator;
// CALLBACK functions for GLU_TESS ////////////////////////////////////////////
// NOTE: must be declared with CALLBACK
void CALLBACK tessBeginCB(GLenum which, void *user_data);
void CALLBACK tessEndCB();
void CALLBACK tessErrorCB(GLenum errorCode, void* user_data);
void CALLBACK tessVertexCB(const GLvoid *data, void *user_data);
void CALLBACK tessCombineCB(const GLdouble newVertex[3], const GLdouble *neighborVertex[4],
                            const GLfloat neighborWeight[4], GLdouble **outData, void* user_data);





class B9Tesselator
{
public:
    B9Tesselator();
    ~B9Tesselator();

    //input plygonList must be in order - depicting a fill or void
    int Triangulate( const std::vector<QVector2D> *polygonList, std::vector<QVector2D> *triangleStrip);
    std::vector<QVector2D>* GetTrangleStrip();


    GLenum currentEnumType;
    bool fanFirstTri;
    bool fanSecondTri;
    bool stripFirstTri;
    bool stripSecondTri;
    unsigned long int stripCount;
    QVector2D fanOriginVertex;
    QVector2D prevVertex;
    QVector2D prevPrevVertex;

    //48 megabytes of buffer for the triangulator
    unsigned int CombineSize;
    GLdouble Combinevertices[2048][6];               // arrary to store newly created vertices (x,y,z,r,g,b) by combine callback
    unsigned int CombineVertexIndex;
    bool memoryFull;

    int errorAcumulations;
private:
    unsigned long int numPolyVerts;
    GLdouble ** polyverts;//pointer to all vertex data
    std::vector<QVector2D>* triStrip;


};

#endif // B9TESSELATOR_H
