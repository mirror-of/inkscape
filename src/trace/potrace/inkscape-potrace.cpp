/*
 * This is the C++ glue between Inkscape and Potrace
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 * 
 * Potrace, the wonderful tracer located at http://potrace.sourceforge.net,
 * is provided by the generosity of Peter Selinger, to whom we are grateful.
 *
 */

#include "inkscape-potrace.h"


#include "main.h"
#include "greymap.h"
#include "curve.h"
#include "path.h"
#include "bitmap.h"

#include "trace/filterset.h"
#include "trace/imagemap-gdk.h"

#include <inkscape.h>
#include <desktop.h>
#include <document.h>
#include <selection.h>
#include <sp-image.h>
#include <sp-path.h>
#include <svg/stringstream.h>
#include <xml/repr.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <gtkmm.h>

/** BOB added these two small items **/
PotraceStatusFunc potraceStatusFunc = NULL;
void   *potraceStatusUserData       = NULL;

static void updateGui()
{
   //## Allow the GUI to update
   Gtk::Main::iteration(false); //at least once, non-blocking
   while( Gtk::Main::events_pending() )
       Gtk::Main::iteration();

}


static int potraceStatus(char *msg, void *userData)
{
    updateGui();

    if (!msg || !userData)
        return 0;

    //g_message("msg:%s\n", msg);

    Inkscape::Trace::Potrace::PotraceTracingEngine *engine =
          (Inkscape::Trace::Potrace::PotraceTracingEngine *)userData;
    
    return engine->keepGoing;

}




//required by potrace
struct info info;

namespace Inkscape {

namespace Trace {

namespace Potrace {


/**
 *
 */
PotraceTracingEngine::PotraceTracingEngine()
{

    //##### Our defaults
    invert    = false;
    traceType = TRACE_BRIGHTNESS;

    quantizationNrColors = 8;

    brightnessThreshold  = 0.45;

    cannyHighThreshold   = 0.65;


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




typedef struct
{
    double x;
    double y;
} Point;


/**
 * Check a point against a list of points to see if it
 * has already occurred.
 */
static bool
hasPoint(std::vector<Point> &points, double x, double y)
{
    for (unsigned int i=0; i<points.size() ; i++)
        {
        Point p = points[i];
        if (p.x == x && p.y == y)
            return true;
        }
    return false;
}


/**
 *  Recursively descend the path_t node tree, writing paths in SVG
 *  format into the output stream.  The Point vector is used to prevent
 *  redundant paths.
 */
static void
writePaths(PotraceTracingEngine *engine, path_t *plist,
            Inkscape::SVGOStringStream& data, std::vector<Point> &points)
{

    path_t *node;
    for (node=plist; node ; node=node->sibling)
        {
        //g_message("node->fm:%d\n", node->fm);
        if (!node->fm)
            continue;
        dpoint_t *pt = node->fcurve[node->fm -1].c;
        double x0 = 0.0;
        double y0 = 0.0;
        double x1 = 0.0;
        double y1 = 0.0;
        double x2 = pt[2].x;
        double y2 = pt[2].y;
        //Have we been here already?
        if (hasPoint(points, x2, y2))
            {
            //g_message("duplicate point: (%f,%f)\n", x2, y2);
            continue;
            }
        else
            {
            Point p;
            p.x = x2; p.y = y2;
            points.push_back(p);
            }
        data << "M " << x2 << " " << y2 << " ";
        for (int i=0 ; i<node->fm ; i++)
            {
            if (!potraceStatus("wp", (void *)engine))
                return;
            pt = node->fcurve[i].c;
            x0 = pt[0].x;
            y0 = pt[0].y;
            x1 = pt[1].x;
            y1 = pt[1].y;
            x2 = pt[2].x;
            y2 = pt[2].y;
            switch (node->fcurve[i].tag)
                {
                case CORNER:
                    data << "L " << x1 << " " << y1 << " " ;
                    data << "L " << x2 << " " << y2 << " " ;
                break;
                case CURVETO:
                    data << "C " << x0 << " " << y0 << " "
                                 << x1 << " " << y1 << " "
                                 << x2 << " " << y2 << " ";

                break;
                default:
                break;
                }
            }
        data << "z";

        for (path_t *child=node->childlist; child ; child=child->sibling)
            {
            writePaths(engine, child, data, points);
            }
        }
}





static GrayMap *
filter(PotraceTracingEngine &engine, GdkPixbuf * pixbuf)
{
    if (!pixbuf)
        return NULL;

    GrayMap *newGm = NULL;

    /*### Color quantization -- banding ###*/
    if (engine.getTraceType() == TRACE_QUANT)
        {
        RgbMap *rgbmap = gdkPixbufToRgbMap(pixbuf);
        //rgbMap->writePPM(rgbMap, "rgb.ppm");
        newGm = quantizeBand(rgbmap,
                            engine.getQuantizationNrColors());
        rgbmap->destroy(rgbmap);
        //return newGm;
        }

    /*### Brightness threshold ###*/
    else if (engine.getTraceType() == TRACE_BRIGHTNESS)
        {
        GrayMap *gm = gdkPixbufToGrayMap(pixbuf);

        newGm = GrayMapCreate(gm->width, gm->height);
        double cutoff =  3.0 *
               ( engine.getBrightnessThreshold() * 256.0 );
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

        gm->destroy(gm);
        //newGm->writePPM(newGm, "brightness.ppm");
        //return newGm;
        }

    /*### Canny edge detection ###*/
    else if (engine.getTraceType() == TRACE_CANNY)
        {
        GrayMap *gm = gdkPixbufToGrayMap(pixbuf);
        newGm = grayMapCanny(gm, 0.1, engine.getCannyHighThreshold());
        gm->destroy(gm);
        //newGm->writePPM(newGm, "canny.ppm");
        //return newGm;
        }

    /*### Do I invert the image? ###*/
    if (newGm && engine.getInvert())
        {
        for (int y=0 ; y<newGm->height ; y++)
            {
            for (int x=0 ; x<newGm->width ; x++)
                {
                unsigned long brightness = newGm->getPixel(newGm, x, y);
                brightness = 765 - brightness;
                newGm->setPixel(newGm, x, y, brightness);
                }
            }
        }

    return newGm;//none of the above
}


static IndexedMap *
filterIndexed(PotraceTracingEngine &engine, GdkPixbuf * pixbuf)
{
    if (!pixbuf)
        return NULL;

    IndexedMap *newGm = NULL;

    /*### Color quant multiscan ###*/
    if (engine.getTraceType() == TRACE_QUANT_COLOR)
        {
        RgbMap *gm = gdkPixbufToRgbMap(pixbuf);
        RgbMap *gaussMap = rgbMapGaussian(gm);
        newGm = rgbMapQuantize(gaussMap, 4, engine.getQuantScanNrColors());
        gaussMap->destroy(gaussMap);
        gm->destroy(gm);
        }

    /*### Quant multiscan ###*/
    else if (engine.getTraceType() == TRACE_QUANT_MONO)
        {
        RgbMap *gm = gdkPixbufToRgbMap(pixbuf);
        RgbMap *gaussMap = rgbMapGaussian(gm);
        newGm = rgbMapQuantize(gaussMap, 4, engine.getQuantScanNrColors());
        gaussMap->destroy(gaussMap);
        gm->destroy(gm);

        //Turn to grays
        for (int i=0 ; i<newGm->nrColors ; i++)
            {
            RGB rgb = newGm->clut[i];
            int grayVal = (rgb.r + rgb.g + rgb.b) / 3;
            rgb.r = rgb.g = rgb.b = grayVal;
            newGm->clut[i] = rgb;
            }
        }

    return newGm;
}




GdkPixbuf *
PotraceTracingEngine::preview(GdkPixbuf * pixbuf)
{
    if ( traceType == TRACE_QUANT_COLOR ||
         traceType == TRACE_QUANT_MONO   )
        {
        IndexedMap *gm = filterIndexed(*this, pixbuf);
        if (!gm)
            return NULL;

        GdkPixbuf *newBuf = indexedMapToGdkPixbuf(gm);

        gm->destroy(gm);

        return newBuf;
        }
    else
        {
        GrayMap *gm = filter(*this, pixbuf);
        if (!gm)
            return NULL;

        GdkPixbuf *newBuf = grayMapToGdkPixbuf(gm);

        gm->destroy(gm);

        return newBuf;
        }
}


//*This is the core inkscape-to-potrace binding
char *PotraceTracingEngine::grayMapToPath(GrayMap *grayMap)
{

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

    //##Debug
    /*
    FILE *f = fopen("poimage.pbm", "wb");
    bm_writepbm(f, bm);
    fclose(f);
    */

    if (!keepGoing)
        {
        g_warning("aborted");
        return NULL;
        }

    /* process the image */
    path_t *plist;
    int ret = bm_to_pathlist(bm, &plist);
    if (ret)
        {
        g_warning("Potrace::convertImageToPath: trouble tracing temp image\n");
        return NULL;
        }

    //## Free the Potrace bitmap
    bm_free(bm);

    ret = process_path(plist);
    if (ret)
        {
        g_warning("Potrace::convertImageToPath: trouble processing trace\n");
        return NULL;
        }


    if (!keepGoing)
        {
        g_warning("aborted");
        pathlist_free(plist);
        return NULL;
        }

    Inkscape::SVGOStringStream data;

    data << "";

    //## copy the path information into our d="" attribute string
    std::vector<Point> points;
    writePaths(this, plist, data, points);

    //# we are done with the pathlist
    pathlist_free(plist);

    if (!keepGoing)
        return NULL;

    char *d     = strdup((char *)data.str().c_str());
    
    return d;

}



/**
 *  This is called for a single scan
 */
TracingEngineResult *
PotraceTracingEngine::traceSingle(GdkPixbuf * thePixbuf, int *nrPaths)
{

    if (!thePixbuf)
        return NULL;

    GrayMap *grayMap = filter(*this, thePixbuf);
    if (!grayMap)
        return NULL;

    char *d = grayMapToPath(grayMap);
    
    grayMap->destroy(grayMap);
    
    if (!d)
        {
        *nrPaths = 0;
        return NULL;
        }
    char *style = "fill:#000000";
    
    //g_message("### GOT '%s' \n", d);
    TracingEngineResult *result = new TracingEngineResult(style, d);
    *nrPaths = 1;
    
    return result;
}


/**
 *  Called for multiple-scanning algorithms
 */
TracingEngineResult *
PotraceTracingEngine::traceMultiple(GdkPixbuf * thePixbuf, int *nrPaths)
{

    if (!thePixbuf)
        return NULL;

    IndexedMap *iMap = filterIndexed(*this, thePixbuf);
    if (!iMap)
        return NULL;

    TracingEngineResult *results = NULL;
    
    GrayMap *gm = GrayMapCreate(iMap->width, iMap->height);
    
    for (int colorIndex=0 ; colorIndex<iMap->nrColors ; colorIndex++)
        {
        
        /*Make a gray map for each color index */
        for (int row=0 ; row<iMap->height ; row++)
            {
            for (int col=0 ; col<iMap->width ; col++)
                {
                int indx = (int) iMap->getPixel(iMap, col, row);
                if (indx == colorIndex)
                    gm->setPixel(gm, col, row, 0);
                else
                    gm->setPixel(gm, col, row, 765);
                }
            }

        //## Now we have a traceable graymap
        char *d = grayMapToPath(gm);
    
        if (!d)
            {
            *nrPaths = 0;
            return NULL;
            }
        
        //### get style info
        char style[13];
        RGB rgb = iMap->clut[colorIndex];
        sprintf(style, "fill:#%02x%02x%02x", rgb.r, rgb.g, rgb.b);
    
        //g_message("### GOT '%s' \n", d);
        TracingEngineResult *result = new TracingEngineResult(style, d);
        if (!results)
            {
            results = result; //first one
            }
        else
            {
            //walk to end of list
            TracingEngineResult *r;
            for (r=results ; r->next ; r=r->next)
                {}
            r->next = result;
            }



        }
    
    //report the count of paths processed
    *nrPaths = iMap->nrColors;

    gm->destroy(gm);
    iMap->destroy(iMap);  
    
    return results;
}



/**
 *  This is the working method of this interface, and all
 *  implementing classes.  Take a GdkPixbuf, trace it, and
 *  return the path data that is compatible with the d="" attribute
 *  of an SVG <path> element.
 */
TracingEngineResult *
PotraceTracingEngine::trace(GdkPixbuf * thePixbuf, int *nrPaths)
{

    //Set up for messages
    keepGoing             = 1;
    potraceStatusFunc     = potraceStatus;
    potraceStatusUserData = (void *)this;

    if ( traceType == TRACE_QUANT_COLOR ||
         traceType == TRACE_QUANT_MONO   )
        {
        return traceMultiple(thePixbuf, nrPaths);
        }
    else
        {
        return traceSingle(thePixbuf, nrPaths);
        }
}





/**
 *  Abort the thread that is executing getPathDataFromPixbuf()
 */
void
PotraceTracingEngine::abort()
{
    //g_message("PotraceTracingEngine::abort()\n");
    keepGoing = 0;
}




}  // namespace Potrace
}  // namespace Trace
}  // namespace Inkscape

