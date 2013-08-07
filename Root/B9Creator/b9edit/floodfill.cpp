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

#include "floodfill.h"


void floodFill(QImage* pImage, int x, int y, QColor fillColor)
{
	// If the pixel we are starting with is already the fill color, we're done
    if (pImage->pixel(x,y) == fillColor.rgb()) return;

	int startcolor = pImage->pixel(x,y);

    // Create the pixel queue.  Assume the worst case where every pixel in the
    // image may be in the queue.
	int pixelQueueSize = 0;
	int *pixelQueue= new int[pImage->width() * pImage->height()];
	
    // Add the start pixel to the queue (we created a single array of ints,
    // even though we are enqueuing two numbers.  We put the y value in the
    // upper 16 bits of the integer, and the x in the lower 16.  This gives
    // a limit of 65536x65536 pixels, that should be enough.)  
    pixelQueue[0] = (y << 16) + x;
    pixelQueueSize = 1;
  
    // Color the start pixel.
	pImage->setPixel(x,y,fillColor.rgb());

    // Keep going while there are pixels in the queue.
    while (pixelQueueSize > 0){
  
		// Get the x and y values of the next pixel in the queue
		x = pixelQueue[0] & 0xffff;
		y = (pixelQueue[0] >> 16) & 0xffff;
      
		// Remove the first pixel from the queue.  Rather than move all the
		// pixels in the queue, which would take forever, just take the one
		// off the end and move it to the beginning (order doesn't matter here).
		pixelQueueSize--;
		pixelQueue[0] = pixelQueue[pixelQueueSize];
    
		// If we aren't on the left side of the image, see if the pixel to the
		// left has been painted.  If not, paint it and add it to the queue.
		if (x > 0) {
		if ( pImage->pixel(x-1, y) == startcolor )
		{
			pImage->setPixel(x-1, y, fillColor.rgb());
			pixelQueue[pixelQueueSize] = (y << 16) + x-1;
			pixelQueueSize++;
		}
		}

		// If we aren't on the bottom of the image, see if the pixel below
		// this one has been painted.  If not, paint it and add it to the queue.
		if (y > 0) {
			if ( pImage->pixel(x, y-1) == startcolor )
			{
				pImage->setPixel(x, y-1, fillColor.rgb());
				pixelQueue[pixelQueueSize] = ((y-1) << 16) + x;
				pixelQueueSize++;
			}
		}
   
		// If we aren't on the right side of the image, see if the pixel to the
		// right has been painted.  If not, paint it and add it to the queue.
		if (x < pImage->width()-1) {
			if ( pImage->pixel(x+1, y) == startcolor )
			{
				pImage->setPixel(x+1, y, fillColor.rgb());
				pixelQueue[pixelQueueSize] = (y << 16) + x+1;
				pixelQueueSize++;
			}
		}
    
		// If we aren't on the top of the image, see if the pixel above
		// this one has been painted.  If not, paint it and add it to the queue.
		if (y < pImage->height()-1) {
			if ( pImage->pixel(x, y+1) == startcolor )
			{
				pImage->setPixel(x, y+1, fillColor.rgb());
				pixelQueue[pixelQueueSize] = ((y+1) << 16) + x;
				pixelQueueSize++;
			}
		}
	}
	delete [] pixelQueue;
}
