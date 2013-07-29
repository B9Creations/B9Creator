#ifndef B9VERTICALTRICONTAINER_H
#define B9VERTICALTRICONTAINER_H

#include <vector>
#include "triangle3d.h"

#define TRICONTAINER_PLAY 0.001

class B9VerticalTriContainer
{
public:
    B9VerticalTriContainer();

    double maxZ;
    double minZ;

    B9VerticalTriContainer* upContainer;
    B9VerticalTriContainer* downContainer;

    std::vector<Triangle3D*> tris;


    bool TriangleFits(Triangle3D* tri)
    {
        //test if the triangle has any part in the container.
        if((tri->maxBound.z() >= (minZ - TRICONTAINER_PLAY))
          &&
          (tri->minBound.z() <= (maxZ + TRICONTAINER_PLAY)))

           return true;

        return false;
    }

};

#endif // B9VERTICALTRICONTAINER_H
