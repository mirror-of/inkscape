#ifndef __SP_GRADIENT_POSITION_H__
#define __SP_GRADIENT_POSITION_H__

/*
 * Gradient positioning widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_GRADIENT_POSITION (sp_gradient_position_get_type ())
#define SP_GRADIENT_POSITION(o) (GTK_CHECK_CAST ((o), SP_TYPE_GRADIENT_POSITION, SPGradientPosition))
#define SP_GRADIENT_POSITION_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_GRADIENT_POSITION, SPGradientPositionClass))
#define SP_IS_GRADIENT_POSITION(o) (GTK_CHECK_TYPE ((o), SP_TYPE_GRADIENT_POSITION))
#define SP_IS_GRADIENT_POSITION_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_GRADIENT_POSITION))

typedef struct _SPGradientPosition SPGradientPosition;
typedef struct _SPGradientPositionClass SPGradientPositionClass;

#include <libnr/nr-gradient.h>
#include <gtk/gtkwidget.h>
#include "helper/nr-gradient-gpl.h"
#include "sp-gradient.h"

enum {
	SP_GRADIENT_POSITION_MODE_LINEAR,
	SP_GRADIENT_POSITION_MODE_RADIAL
};

struct _SPGPLGData {
	float x1, y1, x2, y2;
};

struct _SPGPRGData {
	float cx, cy, fx, fy, r;
};

struct _SPGradientPosition {
	GtkWidget widget;
	guint need_update : 1;
	guint dragging : 1;
	guint position : 2;
	guint mode : 1;
	guint changed : 1;
	SPGradient *gradient;
	NRRectS vbox; /* BBox in widget coordinates */
	GdkGC *gc;
	GdkPixmap *px;

	/* Spread type from libnr */
	guint spread : 2;

	unsigned char *cv;
	union {
		NRLGradientRenderer lgr;
		NRRGradientRenderer rgr;
	} renderer;

	NRMatrixF gs2d;

	union {
		struct _SPGPLGData linear;
		struct _SPGPRGData radial;
	} gdata;

	/* Gradiented bbox in document coordinates */
	NRRectF bbox;
	/* Window in document coordinates */
	NRRectF wbox;
	/* Window <-> document transformation */
	NRMatrixF w2d;
	NRMatrixF d2w;
	NRMatrixF w2gs;
	NRMatrixF gs2w;
};

struct _SPGradientPositionClass {
	GtkWidgetClass parent_class;

	void (* grabbed) (SPGradientPosition *pos);
	void (* dragged) (SPGradientPosition *pos);
	void (* released) (SPGradientPosition *pos);
	void (* changed) (SPGradientPosition *pos);

};

GtkType sp_gradient_position_get_type (void);

GtkWidget *sp_gradient_position_new (SPGradient *gradient);

void sp_gradient_position_set_gradient (SPGradientPosition *pos, SPGradient *gradient);

void sp_gradient_position_set_mode (SPGradientPosition *pos, guint mode);
void sp_gradient_position_set_bbox (SPGradientPosition *pos, gdouble x0, gdouble y0, gdouble x1, gdouble y1);

void sp_gradient_position_set_gs2d_matrix_f (SPGradientPosition *pos, NRMatrixF *gs2d);
void sp_gradient_position_get_gs2d_matrix_f (SPGradientPosition *pos, NRMatrixF *gs2d);

void sp_gradient_position_set_linear_position (SPGradientPosition *pos, float x1, float y1, float x2, float y2);
void sp_gradient_position_set_radial_position (SPGradientPosition *pos, float cx, float cy, float fx, float fy, float r);

void sp_gradient_position_set_spread (SPGradientPosition *pos, unsigned int spread);

void sp_gradient_position_get_linear_position_floatv (SPGradientPosition *gp, float *pos);
void sp_gradient_position_get_radial_position_floatv (SPGradientPosition *gp, float *pos);


#endif
