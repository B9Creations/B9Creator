#include "SlcExporter.h"


SlcExporter::SlcExporter(std::string filename)
{
	opened = false;
	this->filename = filename;
    outfile.open(filename.c_str(),std::ios_base::binary | std::ios::trunc);
	opened = outfile.is_open();
}

SlcExporter::~SlcExporter(void)
{
	outfile.close();
}

void SlcExporter::WriteHeader(bool unitsINCH, bool unitsMM, std::string headerstring)
{
    //outfile.flags(std::ios_base::trunc);

	outfile << "-SLCVER 2.0\n";
	if(unitsMM)
	{
		outfile << "-UNITS MM";
	}
	else if(unitsINCH)
	{
		outfile << "-UNITS INCH";
	}
    //outfile.flags(std::ios_base::binary);


	outfile.write(headerstring.c_str(),headerstring.size());

	char* terminationsequence = new char[3];
	terminationsequence[0] = 0x0d;
	terminationsequence[1] = 0x0a;
	terminationsequence[2] = 0x1a;

	outfile.write(terminationsequence,3);
	delete[] terminationsequence;
}

void SlcExporter::WriteReservedSpace()
{
	int i;
	char n = 0;
	for(i = 0; i < 256; i++)
	{
		outfile.write(&n,1);
	}
}
void SlcExporter::WriteSampleTableSize(char ntables)
{
	outfile.write(&ntables,1);
}

void SlcExporter::WriteSampleTable(float minz, float layerthick, float linewidthcomp, float reserved)
{
	char* p = (char*)&minz;
	outfile.write(p,4);
	p = (char*)&layerthick;
	outfile.write(p,4);
	p = (char*)&linewidthcomp;
	outfile.write(p,4);
	p = (char*)&reserved;
	outfile.write(p,4);
}

void SlcExporter::WriteNewSlice(float zaltitude, unsigned int numboundries)
{
	char* p = (char*)&zaltitude;
	outfile.write(p,4);
	p = (char*)&numboundries;
	outfile.write(p,4);
}

void SlcExporter::WriteBoundryHeader(unsigned int numvertices, unsigned int numgaps)
{
	char* p = (char*)&numvertices;
	outfile.write(p,4);
	p = (char*)&numgaps;
	outfile.write(p,4);
}

void SlcExporter::WriteBoundryVert(float xcord, float ycord)
{
	char* p = (char*)&xcord;
	outfile.write(p,4);
	p = (char*)&ycord;
	outfile.write(p,4);
}
