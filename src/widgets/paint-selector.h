#ifndef __SP_PAINT_SELECTOR_H__
#define __SP_PAINT_SELECTOR_H__

/*
 * SPPaintSelector
 *
 * Generic paint selector widget
 *
 * Copyright (C) Lauris 2002
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#define SP_TYPE_PAINT_SELECTOR (sp_paint_selector_get_type ())
#define SP_PAINT_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_PAINT_SELECTOR, SPPaintSelector))
#define SP_PAINT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_PAINT_SELECTOR, SPPaintSelectorClass))
#define SP_IS_PAINT_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_PAINT_SELECTOR))
#define SP_IS_PAINT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_PAINT_SELECTOR))

typedef struct _SPPaintSelector SPPaintSelector;
typedef struct _SPPaintSelectorClass SPPaintSelectorClass;

#include <gtk/gtkvbox.h>

#include "../forward.h"

typedef enum {
	SP_PAINT_SELECTOR_MODE_EMPTY,
	SP_PAINT_SELECTOR_MODE_MULTIPLE,
	SP_PAINT_SELECTOR_MODE_NONE,
	SP_PAINT_SELECTOR_MODE_COLOR_RGB,
	SP_PAINT_SELECTOR_MODE_COLOR_CMYK,
	SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR,
	SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL,
	SP_PAINT_SELECTOR_MODE_PATTERN,
	SP_PAINT_SELECTOR_MODE_FRACTAL,
} SPPaintSelectorMode;

struct _SPPaintSelector {
	GtkVBox vbox;

	guint update : 1;

	SPPaintSelectorMode mode;

	GtkWidget *style;
	GtkWidget *none, *solid, *gradient, *radial, *pattern, *fractal;
	GtkWidget *frame, *selector;

	guint32 rgba;
};

struct _SPPaintSelectorClass {
	GtkVBoxClass parent_class;

	void (* mode_changed) (SPPaintSelector *psel, SPPaintSelectorMode mode);

	void (* grabbed) (SPPaintSelector *psel);
	void (* dragged) (SPPaintSelector *psel);
	void (* released) (SPPaintSelector *psel);
	void (* changed) (SPPaintSelector *psel);
};

GtkType sp_paint_selector_get_type (void);

GtkWidget *sp_paint_selector_new (void);

void sp_paint_selector_set_mode (SPPaintSelector *psel, SPPaintSelectorMode mode);

void sp_paint_selector_set_color_rgba_floatv (SPPaintSelector *psel, gfloat *rgba);
void sp_paint_selector_set_color_cmyka_floatv (SPPaintSelector *psel, gfloat *cmyka);

void sp_paint_selector_set_gradient_linear (SPPaintSelector *psel, SPGradient *vector);
void sp_paint_selector_set_lgradient_position (SPPaintSelector *psel, gdouble x0, gdouble y0, gdouble x1, gdouble y1);

void sp_paint_selector_set_gradient_radial (SPPaintSelector *psel, SPGradient *vector);
void sp_paint_selector_set_rgradient_position (SPPaintSelector *psel, gdouble cx, gdouble cy, gdouble fx, gdouble fy, gdouble r);

void sp_paint_selector_set_gradient_bbox (SPPaintSelector *psel, gdouble x0, gdouble y0, gdouble x1, gdouble y1);

void sp_paint_selector_set_gradient_gs2d_matrix_f (SPPaintSelector *psel, NRMatrixF *gs2d);
void sp_paint_selector_get_gradient_gs2d_matrix_f (SPPaintSelector *psel, NRMatrixF *gs2d);

void sp_paint_selector_set_gradient_properties (SPPaintSelector *psel, unsigned int units, unsigned int spread);
void sp_paint_selector_get_gradient_properties (SPPaintSelector *psel, unsigned int *units, unsigned int *spread);

void sp_paint_selector_get_rgba_floatv (SPPaintSelector *psel, gfloat *rgba);
void sp_paint_selector_get_cmyka_floatv (SPPaintSelector *psel, gfloat *cmyka);

SPGradient *sp_paint_selector_get_gradient_vector (SPPaintSelector *psel);

void sp_paint_selector_get_gradient_position_floatv (SPPaintSelector *psel, gfloat *pos);

void sp_paint_selector_write_lineargradient (SPPaintSelector *psel, SPLinearGradient *lg, SPItem *item);
void sp_paint_selector_write_radialgradient (SPPaintSelector *psel, SPRadialGradient *rg, SPItem *item);

void sp_paint_selector_system_color_set (SPPaintSelector *psel, const SPColor *color, float opacity);

G_END_DECLS

#endif
