#include <stdio.h>
#include <stdlib.h>

#include "imagemap-gdk.h"

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

/*#########################################################################
## G R A Y M A P
#########################################################################*/

GrayMap *gdkPixbufToGrayMap(GdkPixbuf *buf)
{
    if (!buf)
        return NULL;

    int width       = gdk_pixbuf_get_width(buf);
    int height      = gdk_pixbuf_get_height(buf);
    guchar *pixdata = gdk_pixbuf_get_pixels(buf);
    int rowstride   = gdk_pixbuf_get_rowstride(buf);
    int n_channels  = gdk_pixbuf_get_n_channels(buf);

    GrayMap *grayMap = GrayMapCreate(width, height);
    if (!grayMap)
        return NULL;

    //### Fill in the odd cells with RGB values
    int x,y;
    int row  = 0;
    for (y=0 ; y<height ; y++)
        {
        guchar *p = pixdata + row;
        for (x=0 ; x<width ; x++)
            {
            unsigned long bright = (int)p[0] + (int)p[1] +(int)p[2];
            grayMap->setPixel(grayMap, x, y, bright);
            p += n_channels;
            }
        row += rowstride;
        }

    return grayMap;
}

GdkPixbuf *grayMapToGdkPixbuf(GrayMap *grayMap)
{
    if (!grayMap)
        return NULL;

    guchar *pixdata = (guchar *)
          malloc(sizeof(guchar) * grayMap->width * grayMap->height * 3);
    if (!pixdata)
        return NULL;

    int n_channels = 3;
    int rowstride  = grayMap->width * 3;

    GdkPixbuf *buf = gdk_pixbuf_new_from_data(pixdata, GDK_COLORSPACE_RGB,
                        0, 8, grayMap->width, grayMap->height,
                        rowstride, NULL, NULL);

    //### Fill in the odd cells with RGB values
    int x,y;
    int row  = 0;
    for (y=0 ; y<grayMap->height ; y++)
        {
        guchar *p = pixdata + row;
        for (x=0 ; x<grayMap->width ; x++)
            {
            unsigned long pix = grayMap->getPixel(grayMap, x, y) / 3;
            p[0] = p[1] = p[2] = (guchar)(pix & 0xff);
            p += n_channels;
            }
        row += rowstride;
        }

    return buf;
}



/*#########################################################################
## R G B M A P
#########################################################################*/

RgbMap *gdkPixbufToRgbMap(GdkPixbuf *buf)
{
    if (!buf)
        return NULL;

    int width       = gdk_pixbuf_get_width(buf);
    int height      = gdk_pixbuf_get_height(buf);
    guchar *pixdata = gdk_pixbuf_get_pixels(buf);
    int rowstride   = gdk_pixbuf_get_rowstride(buf);
    int n_channels  = gdk_pixbuf_get_n_channels(buf);

    RgbMap *rgbMap = RgbMapCreate(width, height);
    if (!rgbMap)
        return NULL;

    //### Fill in the cells with RGB values
    int x,y;
    int row  = 0;
    for (y=0 ; y<height ; y++)
        {
        guchar *p = pixdata + row;
        for (x=0 ; x<width ; x++)
            {
            rgbMap->setPixel(rgbMap, x, y, 
                 (int)p[0], (int)p[1], (int)p[2] );
            p += n_channels;
            }
        row += rowstride;
        }

    return rgbMap;
}

GdkPixbuf *rgbMapToGdkPixbuf(RgbMap *rgbMap)
{
    if (!rgbMap)
        return NULL;

    guchar *pixdata = (guchar *)
          malloc(sizeof(guchar) * rgbMap->width * rgbMap->height * 3);
    if (!pixdata)
        return NULL;

    int n_channels = 3;
    int rowstride  = rgbMap->width * 3;

    GdkPixbuf *buf = gdk_pixbuf_new_from_data(pixdata, GDK_COLORSPACE_RGB,
                        0, 8, rgbMap->width, rgbMap->height,
                        rowstride, NULL, NULL);

    //### Fill in the cells with RGB values
    int x,y;
    int row  = 0;
    for (y=0 ; y<rgbMap->height ; y++)
        {
        guchar *p = pixdata + row;
        for (x=0 ; x<rgbMap->width ; x++)
            {
            RGB rgb = rgbMap->getPixel(rgbMap, x, y);
            p[0] = rgb.r & 0xff;
            p[1] = rgb.g & 0xff;
            p[2] = rgb.b & 0xff;
            p += n_channels;
            }
        row += rowstride;
        }

    return buf;
}

