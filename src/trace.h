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
#ifndef __TRACE_H__
#define __TRACE_H__


#include <glib.h>

#include <gdk-pixbuf/gdk-pixbuf.h>



namespace Inkscape
{

/**
 *
 */
class TracingEngine
{

    public:

    /**
     *
     */
    TracingEngine()
        {}

    /**
     *
     */
    virtual ~TracingEngine()
        {}

    /**
     *  This is the working method of this interface, and all
     *  implementing classes.  Take a GdkPixbuf, trace it, and
     *  return the path data that is compatible with the d="" attribute
     *  of an SVG <path> element.
     */
    virtual char *getPathDataFromPixbuf(GdkPixbuf *pixbuf)
        { return NULL; }

    /**
     *  Abort the thread that is executing getPathDataFromPixbuf()
     */
    virtual void abort()
        {}



};//class TracingEngine









/**
 *  This simple class allows a generic wrapper around a given
 *  TracingEngine object.  Its purpose is to provide a gateway
 *  to a variety of tracing engines, while maintaining a 
 *  consistent interface.
 */
class Trace
{
    public:


    /**
     *
     */
    Trace();


    /**
     *
     */
    ~Trace();


    /**
     *  A convenience method to allow other software to 'see' the
     *  same image that this class sees.
     */
    GdkPixbuf *getSelectedImage();

    /**
     * This is the main working method.  Trace the selected image, if
     * any, and create a <path> element from it, inserting it into
     * the current document.
     */
    void convertImageToPath(TracingEngine *engine);


    /**
     *  Abort the thread that is executing convertImageToPath()
     */
    void abort();

    /**
     *  Get a singleton instance of this class and execute
     *  convertImageToPath()
     */
    static gboolean staticConvertImageToPath();

    /**
     *  Get a singleton instance of this class and execute
     *  showTraceDialog()
     */
    static gboolean staticShowTraceDialog();





    private:

    /**
     * This is the thread code that is called by its counterpart above.
     */
    void convertImageToPathThread();

    /**
     * This is true during execution. Setting it to false (like abort()
     * does) should inform the threaded code that it needs to stop
     */
    bool keepGoing;

    /**
     *  During tracing, this is Non-null, and refers to the
     *  engine that is currently doing the tracing.
     */
    TracingEngine *engine;




};//class Trace








}//namespace Inkscape



#endif //__TRACE_H__

//#########################################################################
//# E N D   O F   F I L E
//#########################################################################

