#include <stdio.h>
#include <stdlib.h>
#include "graymap.h"

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


/*#########################################################################
### G R A Y M A P
#########################################################################*/

static void setPixel(GrayMap *me, int x, int y, unsigned long val)
{
    long offset = y * me->width + x;
    if (val>765)
        val = 765;
    me->pixels[offset] = val;
}

static unsigned long getPixel(GrayMap *me, int x, int y)
{
    long offset = y * me->width + x;
    return me->pixels[offset];
}





static int gaussMatrix[] =
{
     2,  4,  5,  4, 2,
     4,  9, 12,  9, 4,
     5, 12, 15, 12, 5,
     4,  9, 12,  9, 4,
     2,  4,  5,  4, 2
};

static GrayMap *getGaussian(GrayMap *me)
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
            unsigned long sum = 0;

	    /* image boundaries */
            if (x<firstX || x>lastX || y<firstY || y>lastY)
                {
                newGm->setPixel(newGm, x, y, me->getPixel(me, x, y));
                continue;
                }
            int gaussIndex = 0;
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


static int writePPM(GrayMap *me, char *fileName)
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


static void destroy(GrayMap *me)
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
    me->setPixel    = setPixel;
    me->getPixel    = getPixel;
    me->getGaussian = getGaussian;
    me->writePPM    = writePPM;
    me->destroy     = destroy;

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
### E N D    O F    F I L E
#########################################################################*/
