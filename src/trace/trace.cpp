/*
 * A generic interface for plugging different
 *  autotracers into Inkscape.
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <glibmm.h>

#include <trace.h>

#include <potrace/inkscape-potrace.h>
#include <dialogs/tracedialog.h>

#include <inkscape.h>
#include <desktop.h>
#include <document.h>
#include <helper/sp-intl.h>
#include <selection.h>
#include <sp-image.h>
#include <sp-path.h>
#include <svg/stringstream.h>
#include <display/curve.h>
#include <xml/repr.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

namespace Inkscape
{

/**
 *
 */
Trace::Trace()
{
    engine = NULL;
    selectedItem = NULL;
}



/**
 *
 */
Trace::~Trace()
{
}


SPImage *
Trace::getSelectedSPImage()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        {
        g_warning("Trace: no active desktop\n");
        return NULL;
        }

    SPSelection *sel = desktop->selection;
    if (!sel)
        {
        char *msg = _("Trace: Nothing selected");
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        return NULL;
        }

    SPItem *item = sel->singleItem();
    if (!item)
        {
        char *msg = _("Trace: Nothing selected");  //same as above
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        return NULL;
        }

    if (!SP_IS_IMAGE(item))
        {
        char *msg = _("Trace: Selected object is not an image");
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        return NULL;
        }

    selectedItem = item;

    SPImage *img = SP_IMAGE(item);

    return img;

}


GdkPixbuf *
Trace::getSelectedImage()
{

    SPImage *img = getSelectedSPImage();
    if (!img)
        return NULL;

    GdkPixbuf *pixbuf = img->pixbuf;

    return pixbuf;

}



/**
 *  Threaded method that actually does the conversion
 */
void Trace::convertImageToPathThread()
{
    //## Remember. NEVER leave this method without setting
    //## engine back to NULL

    //## Prepare our kill flag.  We will watch this later to
    //## see if the main thread wants us to stop
    keepGoing = true;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        {
        g_warning("Trace: no active desktop\n");
        return;
        }

    if (!SP_ACTIVE_DOCUMENT)
        {
        char *msg = _("Trace: no active document");
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        engine = NULL;
        return;
        }
    SPDocument *doc = SP_ACTIVE_DOCUMENT;

    SPImage *img = getSelectedSPImage();
    if (!img || !selectedItem)
        {
        engine = NULL;
        return;
        }

    GdkPixbuf *pixbuf = img->pixbuf;

    if (!pixbuf)
        {
        char *msg = _("Trace: mage has no bitmap data");
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        engine = NULL;
        return;
        }

    char *d = engine->getPathDataFromPixbuf(pixbuf);

    //## EXAMPLE:  Check if we should stop
    if (!keepGoing)
        {
        engine = NULL;
        return;
        }

    SPRepr *pathRepr    = sp_repr_new("path");
    SPRepr *imgRepr     = SP_OBJECT(img)->repr;

    sp_repr_set_attr(pathRepr, "d", d);

    //### Copy position info from <image> to <path>
    NR::Matrix tf = sp_item_i2d_affine(selectedItem);
    /*
    double xpos = NR_MATRIX_DF_TRANSFORM_X(tf, ixval, iyval);
    double ypos = NR_MATRIX_DF_TRANSFORM_Y(tf, ixval, iyval);
    Inkscape::SVGOStringStream data;
    data << "translate(" << xpos << ", " << ypos << ")" ;
    sp_repr_set_attr(pathRepr, "transform", data.str().c_str());
    */
    /*
    char *xvalstr = (char *)sp_repr_attr(imgRepr, "x");
    char *yvalstr = (char *)sp_repr_attr(imgRepr, "y");
    if (xval && yval)
        {
        Inkscape::SVGOStringStream data;
        data << "translate(" << xval << ", " << yval << ")" ;
        sp_repr_set_attr(pathRepr, "transform", data.str().c_str());
        }
    */

    SPObject *reprobj   = doc->getObjectByRepr(pathRepr);

    //#Add to tree
    SPRepr *par = sp_repr_parent(imgRepr);
    sp_repr_add_child(par, pathRepr, imgRepr);

    free(d);

    //### Apply the transform from the image to the new shape
    SPShape *newShape = SP_SHAPE(reprobj);
    SPCurve *curve    = sp_shape_get_curve(newShape);
    sp_curve_transform(curve, tf);

    //## inform the document, so we can undo
    sp_document_done(doc);

    engine = NULL;

}

/**
 *  Static no-knowledge version
 */
void Trace::convertImageToPath(TracingEngine *theEngine)
{
    //Check if we are already running
    if (engine)
        return;

    engine = theEngine;

#if HAVE_THREADS
    //Ensure that thread support is running
    if (!Glib::thread_supported())
        Glib::thread_init();

    //Create our thread and run it
    Glib::Thread::create(
        sigc::mem_fun(*this, &Trace::convertImageToPathThread), false);
#else
    convertImageToPathThread();
#endif

}



/**
 *  Abort the thread that is executing convertImageToPath()
 */
void Trace::abort()
{

    g_message("Trace::abort() soon to be implemented\n");

    //## Inform Trace's working thread
    keepGoing = false;

    if (engine)
        {
        engine->abort();
        }

}






/**
 *  Static no-knowledge version
 */
gboolean Trace::staticConvertImageToPath()
{
    Trace trace;
    Inkscape::Potrace::PotraceTracingEngine engine;
    trace.convertImageToPath(&engine);
    return true;
}








}//namespace Inkscape


//#########################################################################
//# E N D   O F   F I L E
//#########################################################################

