#define __SP_STAR_C__

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

#include <config.h>

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "svg/svg.h"
#include "attributes.h"
#include "desktop.h"
#include "desktop-affine.h"
#include "dialogs/object-attributes.h"
#include "helper/sp-intl.h"

#include "sp-star.h"

#define noSTAR_VERBOSE

#define SP_EPSILON 1e-9

static void sp_star_class_init (SPStarClass *class);
static void sp_star_init (SPStar *star);

static void sp_star_build (SPObject * object, SPDocument * document, SPRepr * repr);
static SPRepr *sp_star_write (SPObject *object, SPRepr *repr, guint flags);
static void sp_star_set (SPObject *object, unsigned int key, const unsigned char *value);
static void sp_star_update (SPObject *object, SPCtx *ctx, guint flags);

static gchar * sp_star_description (SPItem * item);
static int sp_star_snappoints (SPItem *item, NRPointF *p, int size);

static void sp_star_set_shape (SPShape *shape);

static SPPolygonClass *parent_class;

GType
sp_star_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (SPStarClass),
			NULL, NULL,
			(GClassInitFunc) sp_star_class_init,
			NULL, NULL,
			sizeof (SPStar),
			16,
			(GInstanceInitFunc) sp_star_init,
		};
		type = g_type_register_static (SP_TYPE_POLYGON, "SPStar", &info, 0);
	}
	return type;
}

static void
sp_star_class_init (SPStarClass *class)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	SPPathClass * path_class;
	SPShapeClass * shape_class;

	gobject_class = (GObjectClass *) class;
	sp_object_class = (SPObjectClass *) class;
	item_class = (SPItemClass *) class;
	path_class = (SPPathClass *) class;
	shape_class = (SPShapeClass *) class;

	parent_class = g_type_class_ref (SP_TYPE_POLYGON);

	sp_object_class->build = sp_star_build;
	sp_object_class->write = sp_star_write;
	sp_object_class->set = sp_star_set;
	sp_object_class->update = sp_star_update;

	item_class->description = sp_star_description;
	item_class->snappoints = sp_star_snappoints;

	shape_class->set_shape = sp_star_set_shape;
}

static void
sp_star_init (SPStar * star)
{
	star->sides = 5;
	star->cx = 0.0;
	star->cy = 0.0;
	star->r1 = 1.0;
	star->r2 = 0.001;
	star->arg1 = star->arg2 = 0.0;
}

static void
sp_star_build (SPObject * object, SPDocument * document, SPRepr * repr)
{

	if (((SPObjectClass *) parent_class)->build)
		((SPObjectClass *) parent_class)->build (object, document, repr);

	sp_object_read_attr (object, "sodipodi:cx");
	sp_object_read_attr (object, "sodipodi:cy");
	sp_object_read_attr (object, "sodipodi:sides");
	sp_object_read_attr (object, "sodipodi:r1");
	sp_object_read_attr (object, "sodipodi:r2");
	sp_object_read_attr (object, "sodipodi:arg1");
	sp_object_read_attr (object, "sodipodi:arg2");
}

static SPRepr *
sp_star_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPStar *star;

	star = SP_STAR (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("polygon");
	}

	if (flags & SP_OBJECT_WRITE_SODIPODI) {
		sp_repr_set_attr (repr, "sodipodi:type", "star");
		sp_repr_set_int (repr, "sodipodi:sides", star->sides);
		sp_repr_set_double (repr, "sodipodi:cx", star->cx);
		sp_repr_set_double (repr, "sodipodi:cy", star->cy);
		sp_repr_set_double (repr, "sodipodi:r1", star->r1);
		sp_repr_set_double (repr, "sodipodi:r2", star->r2);
		sp_repr_set_double (repr, "sodipodi:arg1", star->arg1);
		sp_repr_set_double (repr, "sodipodi:arg2", star->arg2);
	}

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags);

	return repr;
}

static void
sp_star_set (SPObject *object, unsigned int key, const unsigned char *value)
{
	SPShape *shape;
	SPStar *star;
	gulong unit;

	shape = SP_SHAPE (object);
	star = SP_STAR (object);

	/* fixme: we should really collect updates */
	switch (key) {
	case SP_ATTR_SODIPODI_SIDES:
		if (value) {
			star->sides = atoi (value);
			star->sides = CLAMP (star->sides, 3, 32);
		} else {
			star->sides = 5;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_CX:
		if (!sp_svg_length_read_lff (value, &unit, NULL, &star->cx) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			star->cx = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_CY:
		if (!sp_svg_length_read_lff (value, &unit, NULL, &star->cy) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			star->cy = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_R1:
		if (!sp_svg_length_read_lff (value, &unit, NULL, &star->r1) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			star->r1 = 1.0;
		}
		/* fixme: Need CLAMP (Lauris) */
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_R2:
		if (!sp_svg_length_read_lff (value, &unit, NULL, &star->r2) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			star->r2 = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		return;
	case SP_ATTR_SODIPODI_ARG1:
		if (value) {
			star->arg1 = atof (value);
		} else {
			star->arg1 = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_ARG2:
		if (value) {
			star->arg2 = atof (value);
		} else {
			star->arg2 = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	default:
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

static void
sp_star_update (SPObject *object, SPCtx *ctx, guint flags)
{
	SPStar *star;

	star = (SPStar *) object;

	if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
		sp_shape_set_shape ((SPShape *) object);
	}

	if (((SPObjectClass *) parent_class)->update)
		((SPObjectClass *) parent_class)->update (object, ctx, flags);
}

static gchar *
sp_star_description (SPItem *item)
{
	SPStar *star;

	star = SP_STAR (item);

	return g_strdup_printf ("Star of %d sides", star->sides);
}

static void
sp_star_set_shape (SPShape *shape)
{
	SPStar *star;
	gint i;
	gint sides;
	SPCurve *c;
	NRPointF p;
	
	star = SP_STAR (shape);

#if 0
	if ((star->r1 < 1e-12) || (star->r2 < 1e-12)) return;
	if (star->sides < 3) return;
#endif
	
	c = sp_curve_new ();
	
	sides = star->sides;

	/* i = 0 */
	sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0, &p);
	sp_curve_moveto (c, p.x, p.y);
	sp_star_get_xy (star, SP_STAR_POINT_KNOT2, 0, &p);
	sp_curve_lineto (c, p.x, p.y);

	for (i = 1; i < sides; i++) {
		sp_star_get_xy (star, SP_STAR_POINT_KNOT1, i, &p);
		sp_curve_lineto (c, p.x, p.y);
		sp_star_get_xy (star, SP_STAR_POINT_KNOT2, i, &p);
		sp_curve_lineto (c, p.x, p.y);
	}
	
	sp_curve_closepath (c);
	sp_shape_set_curve_insync (SP_SHAPE (star), c, TRUE);
	sp_curve_unref (c);
}

void
sp_star_position_set (SPStar *star, gint sides, gdouble cx, gdouble cy, gdouble r1, gdouble r2, gdouble arg1, gdouble arg2)
{
	g_return_if_fail (star != NULL);
	g_return_if_fail (SP_IS_STAR (star));
	
	star->sides = CLAMP (sides, 3, 32);
	star->cx = cx;
	star->cy = cy;
	star->r1 = MAX (r1, 0.001);
	star->r2 = CLAMP (r2, 0.0, star->r1);
	star->arg1 = arg1;
	star->arg2 = arg2;
	
	sp_object_request_update ((SPObject *) star, SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: We should use all corners of star (Lauris) */

static int
sp_star_snappoints (SPItem *item, NRPointF *p, int size)
{
#if 0
	SPStar *star;
	gdouble affine[6];
	
	star = SP_STAR(item);
	
	/* we use two points of star */
	sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0, &p1);
	sp_star_get_xy (star, SP_STAR_POINT_KNOT2, 0, &p2);
	p3.x = star->cx; p3.y = star->cy;
	sp_item_i2d_affine (item, affine);
	
	p = g_new (ArtPoint,1);
	art_affine_point (p, &p1, affine);
	points = g_slist_append (points, p);
	p = g_new (ArtPoint,1);
	art_affine_point (p, &p2, affine);
	points = g_slist_append (points, p);
	p = g_new (ArtPoint,1);
	art_affine_point (p, &p3, affine);
	points = g_slist_append (points, p);
#else
	if (((SPItemClass *) parent_class)->snappoints)
		return ((SPItemClass *) parent_class)->snappoints (item, p, size);
#endif
	
	return 0;
}

/**
 * sp_star_get_xy: Get X-Y value as item coordinate system
 * @star: star item
 * @point: point type to obtain X-Y value
 * @index: index of vertex
 * @p: pointer to store X-Y value
 *
 * Initial item coordinate system is same as document coordinate system.
 */

void
sp_star_get_xy (SPStar *star, SPStarPoint point, gint index, NRPointF *p)
{
	gdouble arg, darg;

	darg = 2.0 * M_PI / (double) star->sides;

	switch (point) {
	case SP_STAR_POINT_KNOT1:
		arg = star->arg1 + index * darg;
		p->x = star->r1 * cos (arg) + star->cx;
		p->y = star->r1 * sin (arg) + star->cy;
		break;
	case SP_STAR_POINT_KNOT2:
		arg = star->arg2 + index * darg;
		p->x = star->r2 * cos (arg) + star->cx;
		p->y = star->r2 * sin (arg) + star->cy;
		break;
	}
}

