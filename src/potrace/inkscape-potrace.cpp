
#include "inkscape-potrace.h"


#include "main.h"
#include "greymap.h"
#include "curve.h"
#include "path.h"
#include "bitmap.h"

#include <inkscape.h>
#include <desktop.h>
#include <selection.h>
#include <sp-image.h>
#include <sp-path.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

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

    //required by potrace
    struct info info;

};//class PotraceImpl



gboolean
Potrace::convertImageToPath()
{
    SPSelection *sel = SP_ACTIVE_DESKTOP->selection;
    if (!sel)
        {
        g_warning("Potrace::convertImageToPath: nothing selected\n");
        return false;
        }

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

    //##Get the dimensions
    int width       = gdk_pixbuf_get_width(pixbuf);
    int height      = gdk_pixbuf_get_height(pixbuf);
    guchar *pixdata = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride   = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels  = gdk_pixbuf_get_n_channels(pixbuf);


    bitmap_t *bm = bm_new(width, height);

    //##Read the data out of the Pixbuf
    int x,y;
    int row=0;
    int by = height-1;
    for (y=0 ; y<height ; y++, by--)
        {
        guchar *p = pixdata + row;
        for (x=0 ; x<width ; x++)
            {
            bm_word bpx =
                (((bm_word)p[0]) << 24) |
                (((bm_word)p[1]) << 16) |
                (((bm_word)p[2]) <<  8) |
                (((bm_word)p[3])      );
            BM_PUT(bm, x, by, bpx);
            p += n_channels;
            }
        row += rowstride;
        }

    //##Debug
    /**/
    FILE *f = fopen("poimage.pbm", "wb");
    bm_writepbm(f, bm);
    fclose(f);
    /**/

    //## Free the Potrace bitmap
    bm_free(bm);


    SPPath *path = NULL;

    return true;
}




};//namespace Potrace
};//namespace Inkscape

