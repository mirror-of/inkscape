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


#include <glib.h>

#include <trace.h>

#include <potrace/inkscape-potrace.h>
#include <dialogs/tracedialog.h>

#include <inkscape.h>
#include <desktop.h>
#include <document.h>
#include <selection.h>
#include <sp-image.h>
#include <sp-path.h>
#include <svg/stringstream.h>
#include <xml/repr.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

namespace Inkscape
{

/**
 *
 */
Trace::Trace()
{

}



/**
 *
 */
Trace::~Trace()
{
}



/**
 *
 */
void Trace::setTracingEngine(TracingEngine *newEngine)
{
    engine = newEngine;
}


/**
 *
 */
TracingEngine *Trace::getTracingEngine()
{
    return engine;
}



/**
 *  Static no-knowledge version
 */
gboolean Trace::convertImageToPath()
{
    if (!SP_ACTIVE_DESKTOP)
        {
        g_warning("Trace::convertImageToPath: no active desktop\n");
        return false;
        }

    SPSelection *sel = SP_ACTIVE_DESKTOP->selection;
    if (!sel)
        {
        g_warning("Trace::convertImageToPath: nothing selected\n");
        return false;
        }

    if (!SP_ACTIVE_DOCUMENT)
        {
        g_warning("Trace::convertImageToPath: no active document\n");
        return false;
        }
    SPDocument *doc = SP_ACTIVE_DOCUMENT;

    SPItem *item = sel->singleItem();
    if (!item)
        {
        g_warning("Trace::convertImageToPath: null image\n");
        return false;
        }

    if (!SP_IS_IMAGE(item))
        {
        g_warning("Trace::convertImageToPath: object not an image\n");
        return false;
        }

    SPImage *img = SP_IMAGE(item);

    GdkPixbuf *pixbuf = img->pixbuf;

    if (!pixbuf)
        {
        g_warning("Trace::convertImageToPath: image has no bitmap data\n");
        return false;
        }

    char *d = engine->getPathDataFromPixbuf(pixbuf);

    SPRepr *pathRepr    = sp_repr_new("path");
    SPRepr *imgRepr     = SP_OBJECT(img)->repr;

    sp_repr_set_attr(pathRepr, "d", d);

    //### Copy position info from <image> to <path>
    char *xval = (char *)sp_repr_attr(imgRepr, "x");
    char *yval = (char *)sp_repr_attr(imgRepr, "y");
    if (xval && yval)
        {
        Inkscape::SVGOStringStream data;
        data << "translate(" << xval << ", " << yval << ")" ;
        sp_repr_set_attr(pathRepr, "transform", data.str().c_str());
        }


    //SPObject *reprobj   = doc->getObjectByRepr(pathRepr);

    //#Add to tree
    SPRepr *par         = sp_repr_parent(SP_OBJECT(img)->repr);
    sp_repr_add_child(par, pathRepr, SP_OBJECT(img)->repr);

    free(d);

    //## inform the document, so we can undo
    sp_document_done(doc);

    return true;

}

/**
 *  Static no-knowledge version
 */
gboolean Trace::staticConvertImageToPath()
{
    Trace trace;
    trace.convertImageToPath();
    return true;
}


/**
 *  Static no-knowledge version
 */
gboolean Trace::staticShowTraceDialog()
{
    Inkscape::UI::Dialogs::TraceDialog *dlg = 
          Inkscape::UI::Dialogs::TraceDialog::getInstance();
    dlg->setTrace(NULL);
    dlg->show();
    
    return true;
}






}//namespace Inkscape


//#########################################################################
//# E N D   O F   F I L E
//#########################################################################

