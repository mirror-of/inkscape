#ifndef __SP_SPIRAL_H__
#define __SP_SPIRAL_H__

/*
 * <sodipodi:spiral> implementation
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-shape.h"

G_BEGIN_DECLS

#define SP_TYPE_SPIRAL            (sp_spiral_get_type ())
#define SP_SPIRAL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_SPIRAL, SPSpiral))
#define SP_SPIRAL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_SPIRAL, SPSpiralClass))
#define SP_IS_SPIRAL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_SPIRAL))
#define SP_IS_SPIRAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_SPIRAL))

typedef struct _SPSpiral SPSpiral;
typedef struct _SPSpiralClass SPSpiralClass;

struct _SPSpiral {
	SPShape shape;
	
	/*
	 * Spiral shape is defined as:
	 * x(t) = rad * t^exp cos(2 * Pi * revo*t + arg) + cx
	 * y(t) = rad * t^exp sin(2 * Pi * revo*t + arg) + cy
	 * where spiral curve is drawn for {t | t0 <= t <= 1}.
	 * rad and arg parameters can also be represented by
	 * transformation. shoud I remove these attributes?
	 */
	float cx, cy;
	float exp; /* Spiral expansion factor */
	float revo; /* Spiral revolution factor */
	float rad; /* Spiral radius */
	float arg; /* Spiral argument */
	float t0;
};

struct _SPSpiralClass {
	SPShapeClass parent_class;
};


/* Standard Gtk function */
GType sp_spiral_get_type  (void);

/* Lowlevel interface */
void    sp_spiral_position_set		(SPSpiral      *spiral,
				 gdouble	cx,
				 gdouble	cy,
				 gdouble	exp,
				 gdouble	revo,
				 gdouble	rad,
				 gdouble	arg,
				 gdouble	t0);

void    sp_spiral_get_xy	(SPSpiral      *spiral,
				 gdouble	t,
				 NRPointF      *p);
void    sp_spiral_get_polar	(SPSpiral      *spiral,
				 gdouble	t,
				 gdouble       *rad,
				 gdouble       *arg);
gboolean sp_spiral_is_invalid   (SPSpiral      *spiral);


G_END_DECLS

#endif
