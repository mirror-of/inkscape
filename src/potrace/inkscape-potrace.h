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

#include "graymap.h"

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
    PotraceTracingEngine();

    /**
     *
     */
    virtual ~PotraceTracingEngine()
        {}

    /**
     * Do I use the brightness threshold to make line art?
     */
    void setUseBrightness(bool val)
        {
        useBrightness = val;
        }

    /**
     * Sets the halfway point for black/white
     */
    void setBrightnessThreshold(double val)
        {
        brightnessThreshold = val;
        }



    /**
     * Do I use Canny filtering
     */
    void setUseCanny(bool val)
        {
        useCanny = val;
        }


    /**
     * Sets lower cutoff for canny non-maximalizing
     */
    void setCannyLowThreshold(double val)
        {
        cannyLowThreshold = val;
        }

    /**
     * Sets upper cutoff for canny non-maximalizing
     */
    void setCannyHighThreshold(double val)
        {
        cannyHighThreshold = val;
        }

    /**
     *
     */
    virtual char *getPathDataFromPixbuf(GdkPixbuf *pixbuf);

    /**
     *
     */
    GrayMap *filter(GdkPixbuf * pixbuf);

    /**
     *
     */
    GdkPixbuf *preview(GdkPixbuf * pixbuf);



    private:

    //## brightness items
    bool useBrightness;
    double brightnessThreshold;

    //## canny items
    bool useCanny;
    double cannyLowThreshold;
    double cannyHighThreshold;


};//class PotraceTracingEngine



};//namespace Potrace
};//namespace Inkscape


#endif  //__INKSCAPE_POTRACE_H__


