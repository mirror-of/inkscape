/*
 * This is the C++ glue between Inkscape and Potrace
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 * 
 * Potrace, the wonderful tracer located at http://potrace.sourceforge.net,
 * is provided by the generosity of Peter Selinger, to whom we are grateful.
 *
 */

#ifndef __INKSCAPE_POTRACE_H__
#define __INKSCAPE_POTRACE_H__

#include <glib.h>
#include <trace/trace.h>
#include <trace/imagemap.h>

namespace Inkscape {

namespace Trace {

namespace Potrace {

typedef enum
    {
    TRACE_BRIGHTNESS,
    TRACE_CANNY,
    TRACE_QUANT,
    TRACE_QUANT_COLOR,
    TRACE_QUANT_MONO
    } TraceType;


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

    void setTraceType(TraceType val)
        {
        traceType = val;
        }
    TraceType getTraceType()
        {
        return traceType;
        }

    /**
     * Sets/gets whether I invert the product of the other filter(s)
     */
    void setInvert(bool val)
        {
        invert = val;
        }
    bool getInvert()
        {
        return invert;
        }

    /**
     * Sets the halfway point for black/white
     */
    void setQuantizationNrColors(int val)
        {
        quantizationNrColors = val;
        }
    int getQuantizationNrColors()
        {
        return quantizationNrColors;
        }

    /**
     * Sets the halfway point for black/white
     */
    void setBrightnessThreshold(double val)
        {
        brightnessThreshold = val;
        }
    double getBrightnessThreshold()
        {
        return brightnessThreshold;
        }

    /**
     * Sets upper cutoff for canny non-maximalizing
     */
    void setCannyHighThreshold(double val)
        {
        cannyHighThreshold = val;
        }
    double getCannyHighThreshold()
        {
        return cannyHighThreshold;
        }

    /**
     * Sets the number of colors for quant multiscan
     */
    void setQuantScanNrColors(int val)
        {
        quantScanNrColors = val;
        }
    int getQuantScanNrColors()
        {
        return quantScanNrColors;
        }


    /**
     *  This is the working method of this implementing class, and all
     *  implementing classes.  Take a GdkPixbuf, trace it, and
     *  return the path data that is compatible with the d="" attribute
     *  of an SVG <path> element.
     */
    virtual TracingEngineResult *trace(GdkPixbuf *pixbuf, int *nrPaths);

    /**
     *  Abort the thread that is executing getPathDataFromPixbuf()
     */
    virtual void abort();

    /**
     *
     */
    GdkPixbuf *preview(GdkPixbuf * pixbuf);

    /**
     *
     */
    int keepGoing;

    

    private:

    TraceType traceType;

    //## do i invert at the end?
    bool invert;

    //## Color-->b&w quantization
    int quantizationNrColors;

    //## brightness items
    double brightnessThreshold;

    //## canny items
    double cannyHighThreshold;

    //## Color-->multiscan quantization
    int quantScanNrColors;

    char *grayMapToPath(GrayMap *gm);
    
    TracingEngineResult *traceMultiple(GdkPixbuf *pixbuf, int *nrPaths);
    TracingEngineResult *traceSingle(GdkPixbuf *pixbuf, int *nrPaths);


};//class PotraceTracingEngine



}  // namespace Potrace
}  // namespace Trace
}  // namespace Inkscape


#endif  //__INKSCAPE_POTRACE_H__


