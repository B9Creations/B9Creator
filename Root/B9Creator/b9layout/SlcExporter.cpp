/*************************************************************************************
//
//  LICENSE INFORMATION
//
//  BCreator(tm)
//  Software for the control of the 3D Printer, "B9Creator"(tm)
//
//  Copyright 2011-2012 B9Creations, LLC
//  B9Creations(tm) and B9Creator(tm) are trademarks of B9Creations, LLC
//
//  This file is part of B9Creator
//
//    B9Creator is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    B9Creator is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with B9Creator .  If not, see <http://www.gnu.org/licenses/>.
//
//  The above copyright notice and this permission notice shall be
//    included in all copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
*************************************************************************************/

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
