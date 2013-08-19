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

#ifndef SLICESET_H
#define SLICESET_H
#include <vector>
#include <list>
#include <queue>
#include <QDebug>
#include <QFuture>
#include <QImage>

class Slice;
class B9ModelInstance;
class CrushedPrintJob;
class SliceContext;


struct SliceRequest
{
    double altitude;
    int layerIndx;
};


class SliceSet
{
public:
    SliceSet(B9ModelInstance* pParentInstance);
	~SliceSet();

	//pointer to the "parent" modelclone
    B9ModelInstance* pInstance;

    std::queue<SliceRequest> SliceRequests;
    std::queue<Slice*> SlicedSlices;
    std::queue<Slice*> RasturizedSlices;
    std::queue<Slice*> CompressedSlices;

    std::vector< QFuture<void> > workerThreads;

    SliceContext* raster;

    void QueNewSlice(double realAltitude, int layerIndx);
    void SetSingleModelCompressHint(bool hint);
    Slice* ParallelCreateSlices(bool &slicesInTransit,CrushedPrintJob* toJob);




    void ComputeSlice(Slice* slice);
    void RasterizeSlice(Slice* slice);
    void SubtractVoidFromFill(QImage *img);
    void AddSliceToJob(Slice *rasSlice, CrushedPrintJob* job);



private:
    void SetupFutureWorkers();
    void SetupRasturizer();

    QMutex mutex;
    bool singleModelCompression;


};

#endif
