/*
 * Some filters for Potrace in Inkscape
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "filterset.h"

#include "imagemap-gdk.h"


/*#########################################################################
### C A N N Y
#########################################################################*/


static int sobelX[] =
{
    -1,  0,  1 ,
    -2,  0,  2 ,
    -1,  0,  1 
};

static int sobelY[] =
{
     1,  2,  1 ,
     0,  0,  0 ,
    -1, -2, -1 
};


static GrayMap *canny(GrayMap *gm, double dLowThreshold, double dHighThreshold)
{
    int width  = gm->width;
    int height = gm->height;
    int firstX = 1;
    int lastX  = width-2;
    int firstY = 1;
    int lastY  = height-2;

    GrayMap *newGm = GrayMapCreate(width, height);
    if (!newGm)
        return NULL;

    for (int y = 0 ; y<height ; y++)
        {
        for (int x = 0 ; x<width ; x++)
            {
            unsigned long sum = 0;
	    /* image boundaries */
            if (x<firstX || x>lastX || y<firstY || y>lastY)
                {
                sum = 0;
                }
            else
                {
                /* ### SOBEL FILTERING #### */
                long sumX = 0;
                long sumY = 0;
                int sobelIndex = 0;
                for (int i= y-1 ; i<=y+1 ; i++)
                    {
                    for (int j= x-1; j<=x+1 ; j++)
                        {
                        sumX += gm->getPixel(gm, j, i) * 
                             sobelX[sobelIndex++];
		        }
	            }

                sobelIndex = 0;
                for (int i= y-1 ; i<=y+1 ; i++)
                    {
                    for (int j= x-1; j<=x+1 ; j++)
                        {
                        sumY += gm->getPixel(gm, j, i) * 
                             sobelY[sobelIndex++];
		        }
	            }
                /*###  GET VALUE ### */
                sum = abs(sumX) + abs(sumY);

                if (sum > 765)
                    sum = 765;

                /*###  GET ORIENTATION ### */
                double orient = 0.0;
                if (sumX==0)
                    {
                    if (sumY==0)
                        orient = 0.0;
                    else if (sumY<0)
                        {
                        sumY = -sumY;
                        orient = 90.0;
                        }
                    else
                        orient = 90.0;
                    }
                else
                    {
                    orient = 57.295779515 * atan2( ((double)sumY),((double)sumX) );
                    if (orient < 0.0)
                        orient += 180.0;
                    }

                /*###  GET EDGE DIRECTION ### */
                int edgeDirection = 0;
                if (orient < 22.5)
                    edgeDirection = 0;
	        else if (orient < 67.5)
                    edgeDirection = 45;
	        else if (orient < 112.5)
                    edgeDirection = 90;
	        else if (orient < 157.5)
                    edgeDirection = 135;

                /* printf("%ld %ld %f %d\n", sumX, sumY, orient, edgeDirection); */

                /*### Get two adjacent pixels in edge direction ### */
                unsigned long leftPixel;
                unsigned long rightPixel;
                if (edgeDirection == 0)
                    {
                    leftPixel  = gm->getPixel(gm, x-1, y);
                    rightPixel = gm->getPixel(gm, x+1, y);
                    }
                else if (edgeDirection == 45)
                    {
                    leftPixel  = gm->getPixel(gm, x-1, y+1);
                    rightPixel = gm->getPixel(gm, x+1, y-1);
                    }
                else if (edgeDirection == 90)
                    {
                    leftPixel  = gm->getPixel(gm, x, y-1);
                    rightPixel = gm->getPixel(gm, x, y+1);
                    }
                else /*135 */
                    {
                    leftPixel  = gm->getPixel(gm, x-1, y-1);
                    rightPixel = gm->getPixel(gm, x+1, y+1);
                    }

                /*### Compare current value to adjacent pixels ### */
                /*### if less that either, suppress it ### */
                if (sum < leftPixel || sum < rightPixel)
                    sum = 0;
                else
                    {
                    unsigned long highThreshold = 
                          (unsigned long)(dHighThreshold * 765.0);
                    unsigned long lowThreshold = 
                          (unsigned long)(dLowThreshold * 765.0);
                    if (sum >= highThreshold)
                        sum = 765; /* EDGE.  3*255 this needs to be settable */
                    else if (sum < lowThreshold)
                        sum = 0; /* NONEDGE */
                    else
                        {
                        if ( gm->getPixel(gm, x-1, y-1)> highThreshold ||
                             gm->getPixel(gm, x  , y-1)> highThreshold ||
                             gm->getPixel(gm, x+1, y-1)> highThreshold ||
                             gm->getPixel(gm, x-1, y  )> highThreshold ||
                             gm->getPixel(gm, x+1, y  )> highThreshold ||
                             gm->getPixel(gm, x-1, y+1)> highThreshold ||
                             gm->getPixel(gm, x  , y+1)> highThreshold ||
                             gm->getPixel(gm, x+1, y+1)> highThreshold)
                            sum = 765; /* EDGE fix me too */
                        else
                            sum = 0; /* NONEDGE */
                        }
                    }


                }/* else */
            if (sum==0) /* invert light & dark */
                sum = 765;
            else
                sum = 0;
            newGm->setPixel(newGm, x, y, sum);
	    }/* for (x) */
	}/* for (y) */

    return newGm;
}




GrayMap *
grayMapCanny(GrayMap *gm, double lowThreshold, double highThreshold)
{
    if (!gm)
        return NULL;
    GrayMap *gaussGm = gm->getGaussian(gm);
    if (!gaussGm)
        return NULL;
    /*gaussGm->writePPM(gaussGm, "gauss.ppm");*/

    GrayMap *cannyGm = canny(gaussGm, lowThreshold, highThreshold);
    if (!cannyGm)
        return NULL;
    /*cannyGm->writePPM(cannyGm, "canny.ppm");*/

    gaussGm->destroy(gaussGm);

    return cannyGm;
}


GdkPixbuf *gdkCanny(GdkPixbuf *img, double lowThreshold, double highThreshold)
{
    if (!img)
        return NULL;

    GrayMap *grayMap = gdkPixbufToGrayMap(img);
    if (!grayMap)
        return NULL;

    /*grayMap->writePPM(grayMap, "gbefore.ppm");*/

    GrayMap *cannyGm = grayMapCanny(grayMap,lowThreshold, highThreshold);

    grayMap->destroy(grayMap);

    if (!cannyGm)
        return NULL;

    /*grayMap->writePPM(grayMap, "gafter.ppm");*/

    GdkPixbuf *newImg = grayMapToGdkPixbuf(cannyGm);


    return newImg;
}




/*#########################################################################
### Q U A N T I Z A T I O N
#########################################################################*/




GrayMap *quantizeBand(RgbMap *rgbmap, int nrColors)
{

    GrayMap *gm = GrayMapCreate(rgbmap->width, rgbmap->height);

    return gm;
}







/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/










