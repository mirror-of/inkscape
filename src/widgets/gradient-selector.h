#ifndef __SP_GRADIENT_SELECTOR_H__
#define __SP_GRADIENT_SELECTOR_H__

/*
 * Gradient vector and position widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

G_BEGIN_DECLS

typedef struct _SPGradientSelector SPGradientSelector;
typedef struct _SPGradientSelectorClass SPGradientSelectorClass;

#define SP_TYPE_GRADIENT_SELECTOR (sp_gradient_selector_get_type ())
#define SP_GRADIENT_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_GRADIENT_SELECTOR, SPGradientSelector))
#define SP_GRADIENT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_GRADIENT_SELECTOR, SPGradientSelectorClass))
#define SP_IS_GRADIENT_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_GRADIENT_SELECTOR))
#define SP_IS_GRADIENT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_GRADIENT_SELECTOR))

#include <libnr/nr-types.h>
#include <gtk/gtkvbox.h>
#include "../forward.h"

enum {
	SP_GRADIENT_SELECTOR_MODE_LINEAR,
	SP_GRADIENT_SELECTOR_MODE_RADIAL
};

struct _SPGradientSelector {
	GtkVBox vbox;

	guint mode : 1;

	guint gradientUnits : 1;
	guint gradientSpread : 2;

	/* Vector selector */
	GtkWidget *vectors;
	/* Editing buttons */
	GtkWidget *edit, *add;
	/* Position widget */
	GtkWidget *position;
	/* Units selector */
	GtkWidget *units;
	/* Spread selector */
	GtkWidget *spread;
};

struct _SPGradientSelectorClass {
	GtkVBoxClass parent_class;

	void (* grabbed) (SPGradientSelector *sel);
	void (* dragged) (SPGradientSelector *sel);
	void (* released) (SPGradientSelector *sel);
	void (* changed) (SPGradientSelector *sel);
};

GtkType sp_gradient_selector_get_type (void);

GtkWidget *sp_gradient_selector_new (void);

void sp_gradient_selector_set_mode (SPGradientSelector *sel, guint mode);
void sp_gradient_selector_set_units (SPGradientSelector *sel, guint units);
void sp_gradient_selector_set_spread (SPGradientSelector *sel, guint spread);
void sp_gradient_selector_set_vector (SPGradientSelector *sel, SPDocument *doc, SPGradient *vector);
void sp_gradient_selector_set_bbox (SPGradientSelector *sel, gdouble x0, gdouble y0, gdouble x1, gdouble y1);

void sp_gradient_selector_set_gs2d_matrix_f (SPGradientSelector *gsel, NRMatrixF *gs2d);
void sp_gradient_selector_get_gs2d_matrix_f (SPGradientSelector *gsel, NRMatrixF *gs2d);

void sp_gradient_selector_set_lgradient_position (SPGradientSelector *sel, gdouble x0, gdouble y0, gdouble x1, gdouble y1);
void sp_gradient_selector_set_rgradient_position (SPGradientSelector *sel, gdouble cx, gdouble cy, gdouble fx, gdouble fy, gdouble r);

SPGradient *sp_gradient_selector_get_vector (SPGradientSelector *sel);

void sp_gradient_selector_get_lgradient_position_floatv (SPGradientSelector *gsel, gfloat *pos);
void sp_gradient_selector_get_rgradient_position_floatv (SPGradientSelector *gsel, gfloat *pos);

void sp_gradient_selector_get_lgradient_position_floatv (SPGradientSelector *gsel, gfloat *pos);
void sp_gradient_selector_get_rgradient_position_floatv (SPGradientSelector *gsel, gfloat *pos);

G_END_DECLS

#endif
