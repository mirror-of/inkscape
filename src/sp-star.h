#ifndef __SP_STAR_H__
#define __SP_STAR_H__

/*
 * <sodipodi:star> implementation
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

#include "sp-polygon.h"

G_BEGIN_DECLS

#define SP_TYPE_STAR            (sp_star_get_type ())
#define SP_STAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_STAR, SPStar))
#define SP_STAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_STAR, SPStarClass))
#define SP_IS_STAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_STAR))
#define SP_IS_STAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_STAR))

typedef struct _SPStar SPStar;
typedef struct _SPStarClass SPStarClass;

typedef enum {
	SP_STAR_POINT_KNOT1,
	SP_STAR_POINT_KNOT2
} SPStarPoint;

struct _SPStar {
	SPPolygon polygon;

	gint sides;

	float cx, cy;
	float r1, r2;
	float arg1, arg2;
};

struct _SPStarClass {
	SPPolygonClass parent_class;
};

GType sp_star_get_type (void);

void sp_star_position_set (SPStar *star, gint sides, gdouble cx, gdouble cy, gdouble r1, gdouble r2, gdouble arg1, gdouble arg2);

void sp_star_get_xy (SPStar *star, SPStarPoint point, gint index, NRPointF *p);

G_END_DECLS

#endif
