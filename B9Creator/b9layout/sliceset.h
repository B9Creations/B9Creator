#ifndef SLICESET_H
#define SLICESET_H
#include <vector>
class Slice;
class ModelInstance;

class SliceSet
{
public:
	SliceSet(ModelInstance* pParentInstance);
	~SliceSet();

	//pointer to the "parent" modelclone
	ModelInstance* pInstance;

	Slice* pSliceData;
	bool GenerateSlice(double realAltitude); //generates 1 slice from a model at the real altitude.
};

#endif