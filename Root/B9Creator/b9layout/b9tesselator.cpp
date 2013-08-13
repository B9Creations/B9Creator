#include "b9tesselator.h"
#include <QtOpenGL>
#include <OS_GL_Wrapper.h>


B9Tesselator::B9Tesselator()
{
    memoryFull = false;
    CombineSize = 2048;
}
B9Tesselator::~B9Tesselator()
{
    unsigned long int i;

    for(i = 0; i < numPolyVerts; i++)
    {
        delete[] polyverts[i];
    }
    delete[] polyverts;
}


//input plygonList must be in order - depicting a fill or void
int B9Tesselator::Triangulate( const std::vector<QVector2D>* polygonList, std::vector<QVector2D> *triangleStrip)
{

        unsigned long int i;
        GLUtesselator *tess = gluNewTess(); // create a tessellator
        if(!tess) return 0;  // failed to create tessellation object, return 0


        this->triStrip = triangleStrip;
        this->CombineVertexIndex = 0;

        numPolyVerts = polygonList->size();
        polyverts = new GLdouble*[polygonList->size()];

        for(i = 0; i < polygonList->size(); i++)
        {
            polyverts[i] = new GLdouble[3];
            polyverts[i][0] = GLdouble(polygonList->at(i).x());
            polyverts[i][1] = GLdouble(polygonList->at(i).y());
            polyverts[i][2] = GLdouble(0.0);//zero in the z
        }

        gluTessProperty(tess, GLU_TESS_WINDING_RULE,
                           GLU_TESS_WINDING_NONZERO);
        gluTessNormal(tess, 0, 0, 1);

        // register callback functions
        gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (void (CALLBACK *)())tessBeginCB);
        gluTessCallback(tess, GLU_TESS_END, (void (CALLBACK *)())tessEndCB);
        gluTessCallback(tess, GLU_TESS_ERROR_DATA, (void (CALLBACK *)())tessErrorCB);
        gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (void (CALLBACK *)())tessVertexCB);
        gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (void (CALLBACK *)())tessCombineCB);


        gluTessBeginPolygon(tess, (void*) this);
            gluTessBeginContour(tess);
            for(i = 0; i < numPolyVerts; i++)
            {
                gluTessVertex(tess, polyverts[i], polyverts[i]);
            }
            gluTessEndContour(tess);
        gluTessEndPolygon(tess);


        gluDeleteTess(tess);        // delete after tessellation

        return !(int)memoryFull;
}
std::vector<QVector2D>* B9Tesselator::GetTrangleStrip()
{
    return triStrip;
}

//Tesselation callbacks
void CALLBACK tessBeginCB(GLenum which, void* user_data)
{
    B9Tesselator* tess = (B9Tesselator*)user_data;
    //the tesselator could be using either triangls strips, fans, or simple triangles.
    tess->currentEnumType = which;

    if(which == GL_TRIANGLE_FAN)
    {
        tess->fanFirstTri = true;
        tess->fanSecondTri = false;
    }
    else if(which == GL_TRIANGLE_STRIP)
    {
        tess->stripFirstTri = true;
        tess->stripSecondTri = false;
        tess->stripCount = 0;
    }


}






void CALLBACK tessVertexCB(const GLvoid *data, void *user_data)
{

    GLdouble* vertData = (GLdouble*)data;
    QVector2D vert;
    vert.setX(vertData[0]);
    vert.setY(vertData[1]);
    B9Tesselator* tess = (B9Tesselator*)user_data;


    if(tess->currentEnumType == GL_TRIANGLES)
    {
         tess->GetTrangleStrip()->push_back(vert);
    }
    else if(tess->currentEnumType == GL_TRIANGLE_FAN)
    {

        if(tess->fanFirstTri)
        {
            tess->fanOriginVertex = vert;
            tess->fanFirstTri = false;
            tess->fanSecondTri = true;

            return;
        }
        else if(tess->fanSecondTri)
        {
            tess->fanFirstTri = false;
            tess->fanSecondTri = false;

            tess->prevVertex = vert;
        }
        else
        {
            tess->GetTrangleStrip()->push_back(tess->fanOriginVertex);
            tess->GetTrangleStrip()->push_back(tess->prevVertex);
            tess->GetTrangleStrip()->push_back(vert);
            tess->prevVertex = vert;
        }

    }
    else if(tess->currentEnumType == GL_TRIANGLE_STRIP)
    {
        if(tess->stripFirstTri)
        {

            tess->prevPrevVertex = vert;
            tess->stripFirstTri = false;
            tess->stripSecondTri = true;
            return;
        }
        else if(tess->stripSecondTri)
        {
            tess->prevVertex = vert;
            tess->stripFirstTri = false;
            tess->stripSecondTri = false;
        }
        else
        {
            if(tess->stripCount%2)//odd
            {
                tess->GetTrangleStrip()->push_back(tess->prevVertex);
                tess->GetTrangleStrip()->push_back(tess->prevPrevVertex);
                tess->GetTrangleStrip()->push_back(vert);

            }
            else
            {
                tess->GetTrangleStrip()->push_back(tess->prevPrevVertex);
                tess->GetTrangleStrip()->push_back(tess->prevVertex);
                tess->GetTrangleStrip()->push_back(vert);

            }
            tess->prevPrevVertex = tess->prevVertex;
            tess->prevVertex = vert;
        }
        tess->stripCount++;
    }
    else
    {
        qDebug() << "B9Tesselator: WARNING! un-implemented OpenGl Triangle Enum!";
    }




}

void CALLBACK tessCombineCB(const GLdouble newVertex[3], const GLdouble *neighborVertex[4],
                            const GLfloat neighborWeight[4], GLdouble **outData, void* user_data)
{
    B9Tesselator* tess = (B9Tesselator*)user_data;


    if(tess->CombineVertexIndex >= tess->CombineSize)
    {
        tess->memoryFull = true;
        return;
    }


    // copy new intersect vertex to local array
    // Because newVertex is temporal and cannot be hold by tessellator until next
    // vertex callback called, it must be copied to the safe place in the app.
    // Once gluTessEndPolygon() called, then you can safly deallocate the array.
    tess->Combinevertices[tess->CombineVertexIndex][0] = newVertex[0];
    tess->Combinevertices[tess->CombineVertexIndex][1] = newVertex[1];
    tess->Combinevertices[tess->CombineVertexIndex][2] = newVertex[2];



    // return output data (vertex coords and others)
    *outData = tess->Combinevertices[tess->CombineVertexIndex];   // assign the address of new intersect vertex

    tess->CombineVertexIndex++;  // increase index for next vertex

}

void CALLBACK tessEndCB()
{

}


void CALLBACK tessErrorCB(GLenum errorCode, void* user_data)
{
    B9Tesselator* tess = (B9Tesselator*)user_data;
    qDebug() << "onTessError";
    tess->errorAcumulations++;
}









