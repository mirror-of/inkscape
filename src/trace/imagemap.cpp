#include <stdio.h>
#include <stdlib.h>

#include "imagemap.h"

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


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
