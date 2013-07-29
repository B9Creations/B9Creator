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

#include "b9modelwriter.h"
#include "b9layout/triangle3d.h"
#include <QDebug>

B9ModelWriter::B9ModelWriter(QString filename, bool &readyWrite, QObject *parent) :
    QObject(parent)
{
    triCount = 0;


    binOUT.setFileName(filename);
    readyWrite = binOUT.open(QIODevice::WriteOnly | QIODevice::Truncate);

    if(readyWrite)
    {
        binStream.setDevice(&binOUT);
        binStream.setByteOrder(QDataStream::LittleEndian);
        WriteHeader(0);//initially but zero triangles in the header.
    }
}


B9ModelWriter::~B9ModelWriter()
{
    if(binOUT.isOpen())
        binOUT.close();
}



void B9ModelWriter::WriteHeader(quint32 triCount)
{
    unsigned int i;
    quint8 emptyByte1 = 'B';
    quint8 emptyByte2 = '9';

    //write 80byte header
    for(i = 0; i < 80; i++)
    {
        if(i%2)
            binStream << emptyByte1;
        else
            binStream << emptyByte2;
    }

    binStream << quint32(triCount);
}

void B9ModelWriter::WriteNextTri(Triangle3D* pTri)
{
    float nx = pTri->normal.x();
    float ny = pTri->normal.y();
    float nz = pTri->normal.z();

    float x0 = pTri->vertex[0].x();
    float y0 = pTri->vertex[0].y();
    float z0 = pTri->vertex[0].z();

    float x1 = pTri->vertex[1].x();
    float y1 = pTri->vertex[1].y();
    float z1 = pTri->vertex[1].z();

    float x2 = pTri->vertex[2].x();
    float y2 = pTri->vertex[2].y();
    float z2 = pTri->vertex[2].z();


    binOUT.write((char*)&nx,4);
    binOUT.write((char*)&ny,4);
    binOUT.write((char*)&nz,4);
    binOUT.write((char*)&x0,4);
    binOUT.write((char*)&y0,4);
    binOUT.write((char*)&z0,4);
    binOUT.write((char*)&x1,4);
    binOUT.write((char*)&y1,4);
    binOUT.write((char*)&z1,4);
    binOUT.write((char*)&x2,4);
    binOUT.write((char*)&y2,4);
    binOUT.write((char*)&z2,4);

    //atribute byte count (not used by most software)
    binStream << quint16(0);

    triCount++;
}


void B9ModelWriter::Finalize()
{
    binOUT.seek(80);
    binStream << quint32(triCount);

}




















