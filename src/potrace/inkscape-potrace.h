/*
 * A simple integration of Potrace into Inkscape.
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_POTRACE_H__
#define __INKSCAPE_POTRACE_H__

#include <glib.h>
#include <trace.h>

namespace Inkscape
{
namespace Potrace
{


class PotraceTracingEngine : public TracingEngine
{

    public:

    /**
     *
     */
    PotraceTracingEngine()
        {}

    /**
     *
     */
    virtual ~PotraceTracingEngine()
        {}

    /**
     *
     */
    virtual char *getPathDataFromPixbuf(GdkPixbuf *pixbuf);

};//class PotraceTracingEngine



};//namespace Potrace
};//namespace Inkscape


#endif  //__INKSCAPE_POTRACE_H__


