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
    me->setPixel = setPixel;
    me->getPixel = getPixel;
    me->writePPM = writePPM;
    me->destroy  = destroy;

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
