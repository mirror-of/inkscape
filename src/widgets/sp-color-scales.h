#ifndef __SP_COLOR_SCALES_H__
#define __SP_COLOR_SCALES_H__

#include <gtk/gtkvbox.h>
#include "../color.h"
#include "sp-color-slider.h"
#include "sp-color-selector.h"

#include <glib.h>

G_BEGIN_DECLS

typedef struct _SPColorScales SPColorScales;
typedef struct _SPColorScalesClass SPColorScalesClass;


#define SP_TYPE_COLOR_SCALES (sp_color_scales_get_type ())
#define SP_COLOR_SCALES(o) (GTK_CHECK_CAST ((o), SP_TYPE_COLOR_SCALES, SPColorScales))
#define SP_COLOR_SCALES_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_SCALES, SPColorScalesClass))
#define SP_IS_COLOR_SCALES(o) (GTK_CHECK_TYPE ((o), SP_TYPE_COLOR_SCALES))
#define SP_IS_COLOR_SCALES_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_SCALES))

typedef enum {
	SP_COLOR_SCALES_MODE_NONE = 0,
	SP_COLOR_SCALES_MODE_RGB = 1,
	SP_COLOR_SCALES_MODE_HSV = 2,
	SP_COLOR_SCALES_MODE_CMYK = 3
} SPColorScalesMode;

struct _SPColorScales {
	SPColorSelector base;
	SPColorScalesMode mode;
	gboolean updating : 1;
	gboolean dragging : 1;
	GtkAdjustment *a[5]; /* Channel adjustments */
	GtkWidget *s[5]; /* Channel sliders */
	GtkWidget *b[5]; /* Spinbuttons */
	GtkWidget *l[5]; /* Labels */
};

struct _SPColorScalesClass {
	SPColorSelectorClass parent_class;
};

GType sp_color_scales_get_type (void);

GtkWidget *sp_color_scales_new (void);

void sp_color_scales_set_mode (SPColorScales *cs, SPColorScalesMode mode);
SPColorScalesMode sp_color_scales_get_mode (SPColorScales *cs);

void sp_color_scales_set_color_alpha (SPColorSelector *csel, const SPColor *color, gfloat alpha);
void sp_color_scales_get_color_alpha (SPColorSelector *csel, SPColor *color, gfloat *alpha);

G_END_DECLS

#endif
