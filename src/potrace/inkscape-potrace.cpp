
#include "inkscape-potrace.h"


#include "main.h"
#include "greymap.h"
#include "curve.h"
#include "path.h"
#include "bitmap.h"

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



char *
PotraceTracingEngine::getPathDataFromPixbuf(GdkPixbuf * pixbuf)
{
    if (!pixbuf)
        return NULL;

    //##Get the dimensions
    int width       = gdk_pixbuf_get_width(pixbuf);
    int height      = gdk_pixbuf_get_height(pixbuf);
    guchar *pixdata = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride   = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels  = gdk_pixbuf_get_n_channels(pixbuf);

    //g_message("w:%d h:%d rowstride:%d channels:%d\n",
    //       width, height, rowstride, n_channels);

    bitmap_t *bm = bm_new(width, height);
    bm_clear(bm, 0);

    double cutoff =  3.0 * ( brightnessThreshold * 256.0 );
    //g_message("threshold:%f  cutoff:%f\n", brightnessThreshold, cutoff);

    //##Read the data out of the Pixbuf
    int x,y;
    int row=0;
    for (y=0 ; y<height ; y++)
        {
        guchar *p = pixdata + row;
        for (x=0 ; x<width ; x++)
            {
            double brightness = (double)p[0]+(double)p[1]+(double)p[2];
            BM_UPUT(bm, x, y, brightness > cutoff ? 0 : 1);
            p += n_channels;
            }
        row += rowstride;
        }

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

