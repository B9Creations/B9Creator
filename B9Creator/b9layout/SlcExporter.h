#ifndef SLCEXPORTER
#define SLCEXPORTER
#include <fstream>

#include <string>

//using namespace std;

class SlcExporter
{
public:
	SlcExporter(std::string filename);
	~SlcExporter(void);
	bool SuccessOpen(){return opened;}

	void WriteHeader(bool unitsINCH = false, bool unitsMM = true, std::string headerstring = "");
	void WriteReservedSpace();
	void WriteSampleTableSize(char ntables);
	void WriteSampleTable(float minz, float layerthick, float linewidthcomp, float reserved = 0.0);
	void WriteNewSlice(float zaltitude, unsigned int numboundries);
	void WriteBoundryHeader(unsigned int numvertices, unsigned int numgaps);
	void WriteBoundryVert(float xcord, float ycord);

private:

	std::string filename;
	std::ofstream outfile;
	bool opened;

};

#endif