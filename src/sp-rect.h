#ifndef __SP_RECT_H__
#define __SP_RECT_H__

/*
 * SVG <rect> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include "svg/svg-types.h"
#include "sp-shape.h"

G_BEGIN_DECLS

#define SP_TYPE_RECT            (sp_rect_get_type ())
#define SP_RECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_RECT, SPRect))
#define SP_RECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_RECT, SPRectClass))
#define SP_IS_RECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_RECT))
#define SP_IS_RECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_RECT))

typedef struct _SPRect SPRect;
typedef struct _SPRectClass SPRectClass;

struct _SPRect {
	SPShape shape;

	SPSVGLength x;
	SPSVGLength y;
	SPSVGLength width;
	SPSVGLength height;
	SPSVGLength rx;
	SPSVGLength ry;
};

struct _SPRectClass {
	SPShapeClass parent_class;
};


/* Standard GType function */
GType sp_rect_get_type (void);

void sp_rect_position_set (SPRect * rect, gdouble x, gdouble y, gdouble width, gdouble height);

/* If SET if FALSE, VALUE is just ignored */
void sp_rect_set_rx(SPRect * rect, gboolean set, gdouble value);
void sp_rect_set_ry(SPRect * rect, gboolean set, gdouble value);

G_END_DECLS

#endif
