
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
#include <xml/repr.h>

#include <gdk-pixbuf/gdk-pixbuf.h>


//required by potrace
struct info info;

namespace Inkscape
{
namespace Potrace
{


class PotraceImpl : public Potrace
{

    public:

    PotraceImpl()
        {}

    virtual ~PotraceImpl()
        {}

    //## Cutoff brightness for black/white
    double threshold;


};//class PotraceImpl


static void
writePaths(SPCurve *curve, path_t *plist)
{





}







gboolean
Potrace::convertImageToPath()
{
    if (!SP_ACTIVE_DESKTOP)
        {
        g_warning("Potrace::convertImageToPath: no active desktop\n");
        return false;
        }
    SPSelection *sel = SP_ACTIVE_DESKTOP->selection;
    if (!sel)
        {
        g_warning("Potrace::convertImageToPath: nothing selected\n");
        return false;
        }

    if (!SP_ACTIVE_DOCUMENT)
        {
        g_warning("Potrace::convertImageToPath: no active document\n");
        return false;
        }
    SPDocument *doc = SP_ACTIVE_DOCUMENT;

    SPItem *item = sel->singleItem();
    if (!item)
        {
        g_warning("Potrace::convertImageToPath: null image\n");
        return false;
        }

    if (!SP_IS_IMAGE(item))
        {
        g_warning("Potrace::convertImageToPath: object not an image\n");
        return false;
        }

    SPImage *img = SP_IMAGE(item);

    GdkPixbuf *pixbuf = img->pixbuf;

    if (!pixbuf)
        {
        g_warning("Potrace::convertImageToPath: image has no bitmap data\n");
        return false;
        }

    PotraceImpl potrace;
    potrace.threshold = 0.5;

    //##Get the dimensions
    int width       = gdk_pixbuf_get_width(pixbuf);
    int height      = gdk_pixbuf_get_height(pixbuf);
    guchar *pixdata = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride   = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels  = gdk_pixbuf_get_n_channels(pixbuf);

    g_message("w:%d h:%d rowstride:%d channels:%d\n",
           width, height, rowstride, n_channels);

    bitmap_t *bm = bm_new(width, height);
    bm_clear(bm, 0);
    double cutoff =  3.0 * ( potrace.threshold * 256.0 );
    //##Read the data out of the Pixbuf
    int x,y;
    int row=0;
    int by = height-1;
    for (y=0 ; y<height ; y++, by--)
        {
        guchar *p = pixdata + row;
        for (x=0 ; x<width ; x++)
            {
            double sum = (double)p[0]+(double)p[1]+(double)p[2];
            BM_UPUT(bm, x, by, sum > cutoff ? 0 : 1);
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
    if (!ret)
        {
        g_warning("Potrace::convertImageToPath: image has no bitmap data\n");
        return false;
        }

    //## Free the Potrace bitmap
    bm_free(bm);

    SPRepr   *pathRepr  = sp_repr_new("path");
    SPObject *reprobj   = doc->getObjectByRepr(pathRepr);
    SPPath   *path      = SP_PATH(reprobj);

    //#Add to tree
    SPRepr *par         = sp_repr_parent(SP_OBJECT(img)->repr);
    sp_repr_add_child(par, pathRepr, SP_OBJECT(img)->repr);

    SPCurve *curve      = SP_SHAPE(path)->curve;

    //## copy the path information into our Curve
    writePaths(curve, plist);


    pathlist_free(plist);

    return true;
}




};//namespace Potrace
};//namespace Inkscape

