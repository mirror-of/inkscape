
#include "inkscape-potrace.h"


#include "main.h"
#include "greymap.h"
#include "curve.h"
#include "path.h"
#include "bitmap.h"

//## New stuff - bob
#include "canny.h"
#include "graymap-gdk.h"

#include <inkscape.h>
#include <desktop.h>
#include <document.h>
#include <selection.h>
#include <sp-image.h>
#include <sp-path.h>
#include <svg/stringstream.h>
#include <xml/repr.h>
#include <gdk-pixbuf/gdk-pixbuf.h>



//required by potrace
struct info info;

namespace Inkscape
{
namespace Potrace
{

/**
 *
 */
PotraceTracingEngine::PotraceTracingEngine()
{

    //##### Our defaults
    useBrightness       = true;
    brightnessThreshold = 0.5;

    useCanny            = false;
    cannyHighThreshold  = 0.65;
    cannyLowThreshold   = 0.1;


    //##### Potrace's defaults
    //backend_lookup("eps", &info.backend);
#define UNDEF ((double)(1e30))   /* a value to represent "undefined" */
    info.debug        = 0;
    info.width_d.x    = UNDEF;
    info.height_d.x   = UNDEF;
    info.rx           = UNDEF;
    info.ry           = UNDEF;
    info.sx           = UNDEF;
    info.sy           = UNDEF;
    info.stretch      = 1;
    info.lmar_d.x     = UNDEF;
    info.rmar_d.x     = UNDEF;
    info.tmar_d.x     = UNDEF;
    info.bmar_d.x     = UNDEF;
    info.angle        = 0;
    info.paperwidth   = DEFAULT_PAPERWIDTH;
    info.paperheight  = DEFAULT_PAPERHEIGHT;
    info.turdsize     = 2;
    info.unit         = 10;
    info.compress     = 0; //not applicable
    info.pslevel      = 2;
    info.color        = 0x000000;
    info.turnpolicy   = POLICY_MINORITY;
    info.gamma        = 2.2;
    info.opticurve    = 1;
    info.longcoding   = 0;
    info.alphamax     = 1.0;
    info.opttolerance = 0.2;
    info.outfile      = NULL;
    info.blacklevel   = 0.5;
    info.invert       = 0;
    info.opaque       = 0;
    info.group        = 0;
    info.fillcolor    = 0xffffff;
}




static void
writePaths(path_t *plist, Inkscape::SVGOStringStream& data)
{

    path_t *node;
    for (node=plist; node ; node=node->sibling)
        {
        //g_message("node->fm:%d\n", node->fm);
        if (!node->fm)
            continue;
        dpoint_t *pt = node->fcurve[node->fm -1].c;
        data << "M " << pt[2].x << " " << pt[2].y << " ";
        for (int i=0 ; i<node->fm ; i++)
            {
            pt = node->fcurve[i].c;
            switch (node->fcurve[i].tag)
                {
                case CORNER:
                    data << "L " << pt[1].x << " " << pt[1].y << " " ;
                    data << "L " << pt[2].x << " " << pt[2].y << " " ;
                break;
                case CURVETO:
                    data << "C " << pt[0].x << " " << pt[0].y << " "
                                 << pt[1].x << " " << pt[1].y << " "
                                 << pt[2].x << " " << pt[2].y << " ";

                break;
                default:
                break;
                }
            }
        data << "z";

        for (path_t *child=node->childlist; child ; child=child->sibling)
            {
            writePaths(child, data);
            }
        }
}

GrayMap *
PotraceTracingEngine::filter(GdkPixbuf * pixbuf)
{
    if (!pixbuf)
        return NULL;

    GrayMap *gm = gdkPixbufToGrayMap(pixbuf);

    GrayMap *newGm = NULL;

    if (useBrightness)
        {
        newGm = GrayMapCreate(gm->width, gm->height);
        double cutoff =  3.0 * ( brightnessThreshold * 256.0 );
        for (int y=0 ; y<gm->height ; y++)
            {
            for (int x=0 ; x<gm->width ; x++)
                {
                double brightness = (double)gm->getPixel(gm, x, y);
                if (brightness > cutoff)
                    newGm->setPixel(newGm, x, y, 765);
                else
                    newGm->setPixel(newGm, x, y, 0);
                }
            }

        //newGm->writePPM(newGm, "brightness.ppm");
        }
    else if (useCanny)
        {
        newGm = grayMapCanny(gm, cannyLowThreshold, cannyHighThreshold);
        //newGm->writePPM(newGm, "canny.ppm");
        }

    gm->destroy(gm);

    return newGm;
}

GdkPixbuf *
PotraceTracingEngine::preview(GdkPixbuf * pixbuf)
{
    GrayMap *gm = filter(pixbuf);
    if (!gm)
        return NULL;

    GdkPixbuf *newBuf = grayMapToGdkPixbuf(gm);

    gm->destroy(gm);

    return newBuf;

}


char *
PotraceTracingEngine::getPathDataFromPixbuf(GdkPixbuf * thePixbuf)
{
    if (!thePixbuf)
        return NULL;

    GrayMap *grayMap = filter(thePixbuf);

    bitmap_t *bm = bm_new(grayMap->width, grayMap->height);
    bm_clear(bm, 0);

    //##Read the data out of the GrayMap
    for (int y=0 ; y<grayMap->height ; y++)
        {
        for (int x=0 ; x<grayMap->width ; x++)
            {
            BM_UPUT(bm, x, y, grayMap->getPixel(grayMap, x, y) ? 0 : 1);
            }
        }

    grayMap->destroy(grayMap);

    //##Debug
    /*
    FILE *f = fopen("poimage.pbm", "wb");
    bm_writepbm(f, bm);
    fclose(f);
    */

    /* process the image */
    path_t *plist;
    int ret = bm_to_pathlist(bm, &plist);
    if (ret)
        {
        g_warning("Potrace::convertImageToPath: trouble tracing temp image\n");
        return false;
        }

    //## Free the Potrace bitmap
    bm_free(bm);

    ret = process_path(plist);
    if (ret)
        {
        g_warning("Potrace::convertImageToPath: trouble processing trace\n");
        return false;
        }


    Inkscape::SVGOStringStream data;

    data << "";

    //## copy the path information into our d="" attribute string
    writePaths(plist, data);

    //# we are done with the pathlist
    pathlist_free(plist);

    //# get the svg <path> 'd' attribute
    char *d = strdup(data.str().c_str());
    //g_message("### GOT '%s' \n", d);


    return d;
}





};//namespace Potrace
};//namespace Inkscape

