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
     *
     */
    virtual char *getPathDataFromPixbuf(GdkPixbuf *pixbuf)
        { return NULL; }


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
     *
     */
    TracingEngine *getTracingEngine();

    /**
     *
     */
    void setTracingEngine(TracingEngine *engine);



    /**
     *
     */
    gboolean convertImageToPath();

    /**
     *
     */
    static gboolean staticConvertImageToPath();

    /**
     *
     */
    static gboolean staticShowTraceDialog();





    private:



    TracingEngine *engine;


};//class Trace








}//namespace Inkscape



#endif //__TRACE_H__

//#########################################################################
//# E N D   O F   F I L E
//#########################################################################

