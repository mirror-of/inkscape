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

static void sp_star_class_init (SPStarClass *klass);
static void sp_star_init (SPStar *star);

static void sp_star_build (SPObject * object, SPDocument * document, SPRepr * repr);
static SPRepr *sp_star_write (SPObject *object, SPRepr *repr, guint flags);
static void sp_star_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_star_update (SPObject *object, SPCtx *ctx, guint flags);

static gchar * sp_star_description (SPItem * item);
static int sp_star_snappoints(SPItem *item, NR::Point p[], int size);

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
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_POLYGON, "SPStar", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_star_class_init (SPStarClass *klass)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	SPPathClass * path_class;
	SPShapeClass * shape_class;

	gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;
	path_class = (SPPathClass *) klass;
	shape_class = (SPShapeClass *) klass;

	parent_class = (SPPolygonClass *)g_type_class_ref (SP_TYPE_POLYGON);

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
	star->center = NR::Point(0, 0);
	star->r[0] = 1.0;
	star->r[1] = 0.001;
	star->arg[0] = star->arg[1] = 0.0;
	star->flatsided=0;
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
	sp_object_read_attr (object, "inkscape:flatsided");
}

static SPRepr *
sp_star_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPStar *star = SP_STAR (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("polygon");
	}

	if (flags & SP_OBJECT_WRITE_EXT) {
		sp_repr_set_attr (repr, "sodipodi:type", "star");
		sp_repr_set_int (repr, "sodipodi:sides", star->sides);
		sp_repr_set_double (repr, "sodipodi:cx", star->center[NR::X]);
		sp_repr_set_double (repr, "sodipodi:cy", star->center[NR::Y]);
		sp_repr_set_double (repr, "sodipodi:r1", star->r[0]);
		sp_repr_set_double (repr, "sodipodi:r2", star->r[1]);
		sp_repr_set_double (repr, "sodipodi:arg1", star->arg[0]);
		sp_repr_set_double (repr, "sodipodi:arg2", star->arg[1]);
		sp_repr_set_boolean (repr, "inkscape:flatsided", star->flatsided);
	}

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags);

	return repr;
}

static void
sp_star_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPSVGLengthUnit unit;

	SPStar *star = SP_STAR (object);

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
		if (!sp_svg_length_read_ldd (value, &unit, NULL, &star->center[NR::X]) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			star->center[NR::X] = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_CY:
		if (!sp_svg_length_read_ldd (value, &unit, NULL, &star->center[NR::Y]) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			star->center[NR::Y] = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_R1:
		if (!sp_svg_length_read_ldd (value, &unit, NULL, &star->r[0]) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			star->r[0] = 1.0;
		}
		/* fixme: Need CLAMP (Lauris) */
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_R2:
		if (!sp_svg_length_read_ldd (value, &unit, NULL, &star->r[1]) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			star->r[1] = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		return;
	case SP_ATTR_SODIPODI_ARG1:
		if (value) {
			star->arg[0] = g_ascii_strtod (value, NULL);
		} else {
			star->arg[0] = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_ARG2:
		if (value) {
			star->arg[1] = g_ascii_strtod (value, NULL);
		} else {
			star->arg[1] = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_FLATSIDED:
		if (value && !strcmp (value, "true"))
			star->flatsided = true;
        else star->flatsided = false;
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
	if (flags & (SP_OBJECT_MODIFIED_FLAG | 
		     SP_OBJECT_STYLE_MODIFIED_FLAG | 
		     SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
		sp_shape_set_shape ((SPShape *) object);
	}

	if (((SPObjectClass *) parent_class)->update)
		((SPObjectClass *) parent_class)->update (object, ctx, flags);
}

static gchar *
sp_star_description (SPItem *item)
{
	SPStar *star = SP_STAR (item);

	if (star->flatsided == false )
	return g_strdup_printf (_("Star of %d sides"), star->sides);
	else return g_strdup_printf (_("Polygon of %d sides"), star->sides);
}

static void
sp_star_set_shape (SPShape *shape)
{
	SPStar *star = SP_STAR (shape);

	SPCurve *c = sp_curve_new ();
	
	gint sides = star->sides;

	sp_curve_moveto (c, sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0));
	if (star->flatsided == false)
	sp_curve_lineto (c, sp_star_get_xy (star, SP_STAR_POINT_KNOT2, 0));
	
	for (gint i = 1; i < sides; i++)
	    {
		sp_curve_lineto (c, sp_star_get_xy (star, SP_STAR_POINT_KNOT1, i));
		if (star->flatsided == false)
		    sp_curve_lineto (c, sp_star_get_xy (star, SP_STAR_POINT_KNOT2, i));
	}
	
	sp_curve_closepath (c);
	sp_shape_set_curve_insync (SP_SHAPE (star), c, TRUE);
	sp_curve_unref (c);
}

void
sp_star_position_set (SPStar *star, gint sides, NR::Point center, gdouble r1, gdouble r2, gdouble arg1, gdouble arg2, bool isflat)
{
	g_return_if_fail (star != NULL);
	g_return_if_fail (SP_IS_STAR (star));
	
	star->sides = CLAMP (sides, 3, 32);
	star->center = center;
	star->r[0] = MAX (r1, 0.001);
	if (isflat == false)
	star->r[1] = CLAMP (r2, 0.0, star->r[0]);
	else {
		              star->r[1] =CLAMP ( r1*cos(M_PI/sides) ,0.0, star->r[0] );
	      }
	star->arg[0] = arg1;
	star->arg[1] = arg2;
	star->flatsided = isflat;
	sp_object_request_update ((SPObject *) star, SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: We should use all corners of star (Lauris) */

static int sp_star_snappoints(SPItem *item, NR::Point p[], int size)
{
	if (((SPItemClass *) parent_class)->snappoints)
		return ((SPItemClass *) parent_class)->snappoints (item, p, size);
	
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

NR::Point
sp_star_get_xy (SPStar *star, SPStarPoint point, gint index)
{
	gdouble darg = 2.0 * M_PI / (double) star->sides;

	double arg = star->arg[point];
	arg += index * darg;
	return star->r[point] * NR::Point(cos(arg), sin(arg)) + star->center;
}

