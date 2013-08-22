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

#include "sliceset.h"
#include "slice.h"
#include "math.h"
#include "slicecontext.h"
#include "b9layoutprojectdata.h"
#include "crushbitmap.h"
#include <QColor>

#include <QtDebug>
#include <QFutureSynchronizer>


SliceSet::SliceSet(B9ModelInstance* pParentInstance)
{
	if(!pParentInstance)
	{
		qDebug() << "SliceSet Constructor - No Parent Clone Specified!";
	}
	pInstance = pParentInstance;
    raster = NULL;
    SetSingleModelCompressHint(false);
}

SliceSet::~SliceSet()
{
    delete raster;
}
void SliceSet::SetupRasturizer()
{
    if(raster == NULL)
    {
        raster = new SliceContext(NULL,pInstance->pData->pMain->ProjectData());
        raster->makeCurrent();
    }
}

void SliceSet::SetupFutureWorkers()
{
    int threads = 0;
    if(workerThreads.size() == 0)
    {
        threads = QThread::idealThreadCount();
        qDebug() << "SliceSet: Ideal Thread Count: " << threads;
        if(threads == -1)
        {
            threads = 2;
            qDebug() << "SliceSet: Ideal Thread Count Reset to 2";
        }

        for(int i = 0; i < QThread::idealThreadCount(); i++)
        {
            workerThreads.push_back(QFuture<void>());
        }
    }
}

void SliceSet::SetSingleModelCompressHint(bool hint)
{
    singleModelCompression = hint;
}


//Tells the slice set to get ready to create new slices at the qued "positions".
void SliceSet::QueNewSlice(double realAltitude, int layerIndx)
{
    SliceRequest newReq;
    newReq.altitude = realAltitude;
    newReq.layerIndx = layerIndx;

    SliceRequests.push(newReq);
}

//Tells sliceset to UPDATE it's parrallel routine, using non-blocking
//specify job to tell the algorithm to also render and compress to a job.
//this function should not block
Slice* SliceSet::ParallelCreateSlices(bool &slicesInTransit, CrushedPrintJob* toJob)
{
    unsigned int i;
    Slice* pSlice = NULL;
    SliceRequest req;
    int buzythreads = 0;
    SetupFutureWorkers(); // make sure our thread count matches our cores (recomended core count)
    SetupRasturizer();

    //Deploy workers to smart jobs!
    for(i = 0; i < workerThreads.size(); i++)
    {
        if(workerThreads[i].isRunning()){
            buzythreads++;
            continue;
        }

        //COMPRESSION
        if(RasturizedSlices.size() && toJob)
        {

           pSlice = RasturizedSlices.front();
           if(!pSlice->inProccessing)
           {
               RasturizedSlices.pop();

               pSlice->inProccessing = true;
               CompressedSlices.push(pSlice);
               workerThreads[i] = QtConcurrent::run(this, &SliceSet::AddSliceToJob, pSlice, toJob);

               continue;
           }
        }

        //SLICING
        if(SliceRequests.size() && SlicedSlices.size() < 20)//19 slices max in buffer..
        {
            req = SliceRequests.front();
            SliceRequests.pop();

            pSlice = new Slice(req.altitude,req.layerIndx);
            pSlice->inProccessing = true;
            SlicedSlices.push(pSlice);
            workerThreads[i] = QtConcurrent::run(this, &SliceSet::ComputeSlice, pSlice);
            continue;
        }


    }
    //rasturizing
    if(SlicedSlices.size() && toJob)
    {
        pSlice = SlicedSlices.front();
        if(!pSlice->inProccessing)
        {
            SlicedSlices.pop();
            pSlice->inProccessing = true;
            RasterizeSlice(pSlice);
            RasturizedSlices.push(pSlice);
        }
    }


    if(toJob)
    {
        if(SliceRequests.size() || SlicedSlices.size() || RasturizedSlices.size() || CompressedSlices.size())
            slicesInTransit = true;
        else
            slicesInTransit = false;
        if(CompressedSlices.size() && !CompressedSlices.front()->inProccessing){
            pSlice = CompressedSlices.front();
            CompressedSlices.pop();
            return pSlice;}
        else
            return NULL;
    }
    else
    {
        if(SliceRequests.size() || SlicedSlices.size())
            slicesInTransit = true;
        else
            slicesInTransit = false;

        if(SlicedSlices.size() && !SlicedSlices.front()->inProccessing){
            pSlice = SlicedSlices.front();
            SlicedSlices.pop();
            return pSlice;}
        else
            return NULL;
    }


}


//Generates all geomtery for the given slice.
void SliceSet::ComputeSlice(Slice* slice)
{
    slice->GenerateSegments(pInstance);//actually generate the segments inside the slice

    slice->SortSegmentsByX();//sort segments in x direction

    slice->ConnectSegmentNeighbors();//connect adjacent segments

    slice->GenerateLoops();//generate loop structures

    slice->inProccessing = false;
}

//rasterizes the given slice to a void and fill images.
void SliceSet::RasterizeSlice(Slice* slice)
{
    B9LayoutProjectData* projectData = pInstance->pData->pMain->ProjectData();
    unsigned int xres = projectData->GetResolution().x();
    unsigned int yres = projectData->GetResolution().y();

    //render to 2 images, fill and void.
    raster->makeCurrent();
    raster->SetSlice(slice);
    slice->pImg = new QImage(raster->renderPixmap(xres,yres).toImage());
    slice->inProccessing = false;

}

//takes inflates from the master job, OR's with the pixmap and recompresses to the master job.
void SliceSet::AddSliceToJob(Slice* rasSlice, CrushedPrintJob* job)
{

    B9LayoutProjectData* projectData = pInstance->pData->pMain->ProjectData();
    unsigned int xres = projectData->GetResolution().x();
    unsigned int yres = projectData->GetResolution().y();
    QPainter painter;
    QImage img(xres,yres,QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::black);

    SubtractVoidFromFill(rasSlice->pImg);

    //we only need to inflate overlaying slices if there are multiple models
    if(!singleModelCompression)
    {
        job->inflateSlice(rasSlice->layerIndx, &img);
        if(img.size() == QSize(0,0))
        {
            img = QImage(xres,yres,QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::black);
        }

        //or images together
        painter.begin(rasSlice->pImg);
        painter.setRenderHint(QPainter::Antialiasing,false);
        painter.setCompositionMode(QPainter::CompositionMode_Plus);
        painter.drawImage(0,0,img);
        painter.end();
    }

    job->crushSlice(rasSlice->layerIndx, rasSlice->pImg);

    rasSlice->inProccessing = false;   
}



//subtracts void image from fill image.
//fillimage is returned as corrected.
void SliceSet::SubtractVoidFromFill(QImage *img)
{
    B9LayoutProjectData* projectData = pInstance->pData->pMain->ProjectData();
    unsigned int xres = projectData->GetResolution().x();
    unsigned int yres = projectData->GetResolution().y();
    QRgb pickedcolor;
    int x,y;

    for(x = 0; x < xres; x++)
    {
        for(y = 0; y < yres; y++)
        {
            pickedcolor = img->pixel(x,y);
            if(qRed(pickedcolor) || qGreen(pickedcolor))
            {
                int result = qRed(pickedcolor) - qGreen(pickedcolor);
                if(result > 0)
                {
                    result = 255;
                }else result = 0;
                img->setPixel(x,y,QColor(result,0,0).rgb());
            }
        }
    }
}




