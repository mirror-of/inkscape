#include <stdio.h>
#include <stdlib.h>

#include "imagemap.h"

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


/*## for any gaussian filtering below ##*/
static int gaussMatrix[] =
{
     2,  4,  5,  4, 2,
     4,  9, 12,  9, 4,
     5, 12, 15, 12, 5,
     4,  9, 12,  9, 4,
     2,  4,  5,  4, 2
};


/*#########################################################################
### G R A Y M A P
#########################################################################*/


static void gSetPixel(GrayMap *me, int x, int y, unsigned long val)
{
    long offset = y * me->width + x;
    if (val>765)
        val = 765;
    me->pixels[offset] = val;
}

static unsigned long gGetPixel(GrayMap *me, int x, int y)
{
    long offset = y * me->width + x;
    return me->pixels[offset];
}





static GrayMap *gGetGaussian(GrayMap *me)
{
    int width  = me->width;
    int height = me->height;
    int firstX = 2;
    int lastX  = width-3;
    int firstY = 2;
    int lastY  = height-3;

    GrayMap *newGm = GrayMapCreate(width, height);
    if (!newGm)
        return NULL;

    for (int y = 0 ; y<height ; y++)
        {
        for (int x = 0 ; x<width ; x++)
            {
	    /* image boundaries */
            if (x<firstX || x>lastX || y<firstY || y>lastY)
                {
                newGm->setPixel(newGm, x, y, me->getPixel(me, x, y));
                continue;
                }

            /* all other pixels */
            int gaussIndex = 0;
            unsigned long sum = 0;
            for (int i= y-2 ; i<=y+2 ; i++)
                {
                for (int j= x-2; j<=x+2 ; j++)
                    {
                    int weight = gaussMatrix[gaussIndex++];
                    sum += me->getPixel(me, j, i) * weight;
		    }
	        }
            sum /= 115;
	    newGm->setPixel(newGm, x, y, sum);
	    }
	}

    return newGm;



}


static int gWritePPM(GrayMap *me, char *fileName)
{
    if (!fileName)
        return FALSE;

    FILE *f = fopen(fileName, "wb");
    if (!f)
        return FALSE;

    fprintf(f, "P6 %d %d 255\n", me->width, me->height);

    for (int y=0 ; y<me->height; y++)
        {
        for (int x=0 ; x<me->width ; x++)
            {
            unsigned long pix  = me->getPixel(me, x, y) / 3;
            unsigned char pixb = (unsigned char) (pix & 0xff);
            fwrite(&pixb, 1, 1, f);
            fwrite(&pixb, 1, 1, f);
            fwrite(&pixb, 1, 1, f);
            }
        }
    fclose(f);
    return TRUE;
}


static void gDestroy(GrayMap *me)
{
    if (me->pixels)
        free(me->pixels);
    free(me);
}

GrayMap *GrayMapCreate(int width, int height)
{

    GrayMap *me = (GrayMap *)malloc(sizeof(GrayMap));
    if (!me)
        return NULL;

    /** methods **/
    me->setPixel    = gSetPixel;
    me->getPixel    = gGetPixel;
    me->getGaussian = gGetGaussian;
    me->writePPM    = gWritePPM;
    me->destroy     = gDestroy;

    /** fields **/
    me->width  = width;
    me->height = height;
    me->pixels = (unsigned long *) 
              malloc(sizeof(unsigned long) * width * height);
    if (!me->pixels)
        {
        free(me);
        return NULL;
        }

    return me;
}





/*#########################################################################
### I M A G E  M A P
#########################################################################*/



static void rSetPixel(RgbMap *me, int x, int y, int r, int g, int b)
{
    long offset = y * me->width + x;
    RGB rgb;
    rgb.r = r;
    rgb.g = g;
    rgb.b = b;
    me->pixels[offset] = rgb;
}

static void rSetPixelRGB(RgbMap *me, int x, int y, RGB rgb)
{
    long offset = y * me->width + x;
    me->pixels[offset] = rgb;
}

static RGB rGetPixel(RgbMap *me, int x, int y)
{
    long offset = y * me->width + x;
    return me->pixels[offset];
}


static RgbMap *rGetGaussian(RgbMap *me)
{
    int width  = me->width;
    int height = me->height;
    int firstX = 2;
    int lastX  = width-3;
    int firstY = 2;
    int lastY  = height-3;

    RgbMap *newGm = RgbMapCreate(width, height);
    if (!newGm)
        return NULL;

    for (int y = 0 ; y<height ; y++)
        {
        for (int x = 0 ; x<width ; x++)
            {
	    /* image boundaries */
            if (x<firstX || x>lastX || y<firstY || y>lastY)
                {
                newGm->setPixelRGB(newGm, x, y, me->getPixel(me, x, y));
                continue;
                }

            /* all other pixels */
            int gaussIndex = 0;
            int sumR = 0;
            int sumG = 0;
            int sumB = 0;
            for (int i= y-2 ; i<=y+2 ; i++)
                {
                for (int j= x-2; j<=x+2 ; j++)
                    {
                    int weight = gaussMatrix[gaussIndex++];
                    RGB rgb = me->getPixel(me, j, i);
                    sumR += weight * (int)rgb.r;
                    sumG += weight * (int)rgb.g;
                    sumB += weight * (int)rgb.b;
		    }
	        }
            RGB rout;
            rout.r = ( sumR / 115 ) & 0xff;
            rout.g = ( sumG / 115 ) & 0xff;
            rout.b = ( sumB / 115 ) & 0xff;
	    newGm->setPixelRGB(newGm, x, y, rout);
	    }
	}

    return newGm;



}


static int rWritePPM(RgbMap *me, char *fileName)
{
    if (!fileName)
        return FALSE;

    FILE *f = fopen(fileName, "wb");
    if (!f)
        return FALSE;

    fprintf(f, "P6 %d %d 255\n", me->width, me->height);

    for (int y=0 ; y<me->height; y++)
        {
        for (int x=0 ; x<me->width ; x++)
            {
            RGB rgb = me->getPixel(me, x, y);
            fwrite(&(rgb.r), 1, 1, f);
            fwrite(&(rgb.g), 1, 1, f);
            fwrite(&(rgb.b), 1, 1, f);
            }
        }
    fclose(f);
    return TRUE;
}


static void rDestroy(RgbMap *me)
{
    if (me->pixels)
        free(me->pixels);
    free(me);
}



RgbMap *RgbMapCreate(int width, int height)
{

    RgbMap *me = (RgbMap *)malloc(sizeof(RgbMap));
    if (!me)
        return NULL;

    /** methods **/
    me->setPixel    = rSetPixel;
    me->setPixelRGB = rSetPixelRGB;
    me->getPixel    = rGetPixel;
    me->getGaussian = rGetGaussian;
    me->writePPM    = rWritePPM;
    me->destroy     = rDestroy;


    /** fields **/
    me->width  = width;
    me->height = height;
    me->pixels = (RGB *) 
              malloc(sizeof(RGB) * width * height);
    if (!me->pixels)
        {
        free(me);
        return NULL;
        }

    return me;
}






/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/
