#ifndef __SP_COLOR_SCALES_H__
#define __SP_COLOR_SCALES_H__

#include <gtk/gtkvbox.h>
#include "../color.h"
#include "sp-color-slider.h"
#include "sp-color-selector.h"

#include <glib.h>



typedef struct _SPColorScales SPColorScales;
typedef struct _SPColorScalesClass SPColorScalesClass;

typedef enum {
	SP_COLOR_SCALES_MODE_NONE = 0,
	SP_COLOR_SCALES_MODE_RGB = 1,
	SP_COLOR_SCALES_MODE_HSV = 2,
	SP_COLOR_SCALES_MODE_CMYK = 3
} SPColorScalesMode;



class ColorScales: public ColorSelector
{
public:
    ColorScales( SPColorSelector* csel );
    virtual ~ColorScales();

    virtual void init();

    virtual void setSubmode( guint submode );
    virtual guint getSubmode() const;

    void setMode(SPColorScalesMode mode);
    SPColorScalesMode getMode() const;


protected:
    virtual void _colorChanged( const SPColor& color, gfloat alpha );

    static void _adjustmentAnyChanged ( GtkAdjustment *adjustment, SPColorScales *cs );
    static void _sliderAnyGrabbed( SPColorSlider *slider, SPColorScales *cs );
    static void _sliderAnyReleased( SPColorSlider *slider, SPColorScales *cs );
    static void _sliderAnyChanged( SPColorSlider *slider, SPColorScales *cs );
    static void _adjustmentChanged( SPColorScales *cs, guint channel );

    void _getRgbaFloatv( gfloat *rgba );
    void _getCmykaFloatv( gfloat *cmyka );
    guint32 _getRgba32();
    void _updateSliders( guint channels );
    void _recalcColor( gboolean changing );

    SPColorScalesMode _mode;
    gboolean _updating : 1;
    gboolean _dragging : 1;
    GtkAdjustment* _a[5]; /* Channel adjustments */
    GtkWidget* _s[5]; /* Channel sliders */
    GtkWidget* _b[5]; /* Spinbuttons */
    GtkWidget* _l[5]; /* Labels */

private:
    // By default, disallow copy constructor and assignment operator
    ColorScales( const ColorScales& obj );
    ColorScales& operator=( const ColorScales& obj );
};



#define SP_TYPE_COLOR_SCALES (sp_color_scales_get_type ())
#define SP_COLOR_SCALES(o) (GTK_CHECK_CAST ((o), SP_TYPE_COLOR_SCALES, SPColorScales))
#define SP_COLOR_SCALES_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_SCALES, SPColorScalesClass))
#define SP_IS_COLOR_SCALES(o) (GTK_CHECK_TYPE ((o), SP_TYPE_COLOR_SCALES))
#define SP_IS_COLOR_SCALES_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_SCALES))

struct _SPColorScales {
	SPColorSelector parent;
};

struct _SPColorScalesClass {
	SPColorSelectorClass parent_class;
};

GType sp_color_scales_get_type (void);

GtkWidget *sp_color_scales_new (void);



#endif
