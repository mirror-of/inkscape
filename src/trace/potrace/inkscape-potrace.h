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
     * Sets whether I invert the product of the other filter(s)
     */
    void setInvert(bool val)
        {
        invert = val;
        }
    int getInvert()
        {
        return invert;
        }
    /**
     * Do I use the brightness threshold to make line art?
     */
    void setUseQuantization(bool val)
        {
        useQuantization = val;
        }
    bool getUseQuantization()
        {
        return useQuantization;
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
     * Do I use the brightness threshold to make line art?
     */
    void setUseBrightness(bool val)
        {
        useBrightness = val;
        }
    bool getUseBrightness()
        {
        return useBrightness;
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
     * Do I use Canny filtering
     */
    void setUseCanny(bool val)
        {
        useCanny = val;
        }
    bool getUseCanny()
        {
        return useCanny;
        }


    /**
     * Sets lower cutoff for canny non-maximalizing
     */
    void setCannyLowThreshold(double val)
        {
        cannyLowThreshold = val;
        }
    double getCannyLowThreshold()
        {
        return cannyLowThreshold;
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
     *  This is the working method of this implementing class, and all
     *  implementing classes.  Take a GdkPixbuf, trace it, and
     *  return the path data that is compatible with the d="" attribute
     *  of an SVG <path> element.
     */
    virtual char *getPathDataFromPixbuf(GdkPixbuf *pixbuf);

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

    //## do i invert at the end?
    bool invert;

    //## quantization items
    bool useQuantization;
    int quantizationNrColors;

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


