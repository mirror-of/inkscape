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
    PotraceTracingEngine();

    /**
     *
     */
    virtual ~PotraceTracingEngine()
        {}

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
     *
     */
    virtual char *getPathDataFromPixbuf(GdkPixbuf *pixbuf);

    /**
     *
     */
    GdkPixbuf *preview(GdkPixbuf * pixbuf);



    private:

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


