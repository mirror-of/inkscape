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

#include "trace/trace.h"
#include "trace/potrace/inkscape-potrace.h"
#include <dialogs/tracedialog.h>

#include <inkscape.h>
#include <desktop.h>
#include <desktop-handles.h>
#include <document.h>
#include <glibmm/i18n.h>
#include <selection.h>
#include <sp-image.h>
#include <sp-path.h>
#include <svg/stringstream.h>
#include <display/curve.h>
#include <xml/repr.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


namespace Inkscape {

namespace Trace {

/**
 *
 */
Tracer::Tracer()
{
    engine = NULL;
    selectedItem = NULL;
}



/**
 *
 */
Tracer::~Tracer()
{
}



/**
 *
 */
SPImage *
Tracer::getSelectedSPImage()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        {
        g_warning("Trace: No active desktop\n");
        return NULL;
        }

    Inkscape::Selection *sel = desktop->selection;
    if (!sel)
        {
        char *msg = _("Select an <b>image</b> to trace");
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        return NULL;
        }

    SPItem *item = sel->singleItem();
    if (!item)
        {
        char *msg = _("Select an <b>image</b> to trace");  //same as above
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        return NULL;
        }

    if (!SP_IS_IMAGE(item))
        {
        char *msg = _("Select an <b>image</b> to trace");
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        return NULL;
        }

    selectedItem = item;

    SPImage *img = SP_IMAGE(item);

    return img;

}



/**
 *
 */
GdkPixbuf *
Tracer::getSelectedImage()
{

    SPImage *img = getSelectedSPImage();
    if (!img)
        return NULL;

    GdkPixbuf *pixbuf = img->pixbuf;

    return pixbuf;

}



//#########################################################################
//#  T R A C E
//#########################################################################


/**
 *  Threaded method that does single bitmap--->path conversion
 */
void Tracer::traceThread()
{
    //## Remember. NEVER leave this method without setting
    //## engine back to NULL

    //## Prepare our kill flag.  We will watch this later to
    //## see if the main thread wants us to stop
    keepGoing = true;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        {
        g_warning("Trace: No active desktop\n");
        return;
        }

    Inkscape::Selection *selection = SP_DT_SELECTION (desktop);

    if (!SP_ACTIVE_DOCUMENT)
        {
        char *msg = _("Trace: No active document");
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        engine = NULL;
        return;
        }
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    sp_document_ensure_up_to_date(doc);


    SPImage *img = getSelectedSPImage();
    if (!img || !selectedItem)
        {
        engine = NULL;
        return;
        }

    GdkPixbuf *pixbuf = img->pixbuf;

    if (!pixbuf)
        {
        char *msg = _("Trace: Image has no bitmap data");
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, msg);
        //g_warning(msg);
        engine = NULL;
        return;
        }

    int nrPaths;
    TracingEngineResult *results = engine->trace(pixbuf, &nrPaths);
    //printf("nrPaths:%d\n", nrPaths);

    //### Check if we should stop
    if (!keepGoing || !results || nrPaths<1)
        {
        engine = NULL;
        return;
        }

    //### Get pointers to the <image> and its parent
    Inkscape::XML::Node *imgRepr   = SP_OBJECT(img)->repr;
    Inkscape::XML::Node *par       = sp_repr_parent(imgRepr);

    //### Get some information for the new transform()
    double x      = 0.0;
    double y      = 0.0;
    double width  = 0.0;
    double height = 0.0;
    double dval   = 0.0;

    if (sp_repr_get_double(imgRepr, "x", &dval))
        x = dval;
    if (sp_repr_get_double(imgRepr, "y", &dval))
        y = dval;

    if (sp_repr_get_double(imgRepr, "width", &dval))
        width = dval;
    if (sp_repr_get_double(imgRepr, "height", &dval))
        height = dval;

    NR::Matrix trans(NR::translate(x, y));

    double iwidth  = (double)gdk_pixbuf_get_width(pixbuf);
    double iheight = (double)gdk_pixbuf_get_height(pixbuf);

    double iwscale = width  / iwidth;
    double ihscale = height / iheight;

    NR::Matrix scal(NR::scale(iwscale, ihscale));

    //# Convolve scale, translation, and the original transform
    NR::Matrix tf(scal);
    tf *= trans;
    tf *= selectedItem->transform;


    //#OK.  Now let's start making new nodes

    Inkscape::XML::Node *groupRepr = NULL;

    //# if more than 1, make a <g>roup of <path>s
    if (nrPaths > 1)
        {
        groupRepr = sp_repr_new("svg:g");
        sp_repr_add_child(par, groupRepr, imgRepr);
        }

    long totalNodeCount = 0L;

    for (TracingEngineResult *result=results ;
                  result ; result=result->next)
        {
	    totalNodeCount += result->getNodeCount();

        Inkscape::XML::Node *pathRepr = sp_repr_new("svg:path");
        sp_repr_set_attr(pathRepr, "style", result->getStyle());
        sp_repr_set_attr(pathRepr, "d",     result->getPathData());

        if (nrPaths > 1)
            sp_repr_add_child(groupRepr, pathRepr, NULL);
        else
            sp_repr_add_child(par, pathRepr, imgRepr);

        //### Apply the transform from the image to the new shape
        SPObject *reprobj = doc->getObjectByRepr(pathRepr);
        if (reprobj)
            {
            SPItem *newItem = SP_ITEM(reprobj);
            sp_item_write_transform(newItem, pathRepr, tf, NULL);
            }
        if (nrPaths == 1)
            {
            selection->clear();
            selection->addRepr(pathRepr);
            }
        sp_repr_unref (pathRepr);
        }


    delete results;

    // If we have a group, then focus on, then forget it
    if (nrPaths > 1)
        {
        selection->clear();
        selection->addRepr(groupRepr);
        sp_repr_unref (groupRepr);
        }

    //## inform the document, so we can undo
    sp_document_done(doc);

    engine = NULL;

    char *msg = g_strdup_printf(_("Trace: Done. %ld nodes created"), totalNodeCount);
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, msg);
    g_free(msg);

}

/**
 *  Main tracing method
 */
void Tracer::trace(TracingEngine *theEngine)
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
        sigc::mem_fun(*this, &Tracer::traceThread), false);
#else
    traceThread();
#endif

}





/**
 *  Abort the thread that is executing trace()
 */
void Tracer::abort()
{

    //## Inform Trace's working thread
    keepGoing = false;

    if (engine)
        {
        engine->abort();
        }

}



} // namespace Trace

} // namespace Inkscape


//#########################################################################
//# E N D   O F   F I L E
//#########################################################################

