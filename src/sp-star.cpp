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

#include "config.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "svg/svg.h"
#include "attributes.h"
#include "desktop.h"
#include "desktop-affine.h"
#include "dialogs/object-attributes.h"
#include "helper/sp-intl.h"
#include "xml/repr.h"
#include "libnr/nr-point.h"
#include "libnr/nr-point-fns.h"
#include "libnr/nr-point-ops.h"
#include "libnr/nr-point-l.h"

#include "sp-star.h"

static void sp_star_class_init (SPStarClass *klass);
static void sp_star_init (SPStar *star);

static void sp_star_build (SPObject * object, SPDocument * document, SPRepr * repr);
static SPRepr *sp_star_write (SPObject *object, SPRepr *repr, guint flags);
static void sp_star_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_star_update (SPObject *object, SPCtx *ctx, guint flags);

static gchar * sp_star_description (SPItem * item);
static std::vector<NR::Point> sp_star_snappoints(SPItem const *item);

static void sp_star_set_shape (SPShape *shape);

static SPShapeClass *parent_class;

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
		type = g_type_register_static (SP_TYPE_SHAPE, "SPStar", &info, (GTypeFlags)0);
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

	parent_class = (SPShapeClass *)g_type_class_ref (SP_TYPE_SHAPE);

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
	star->flatsided = 0;
	star->rounded = 0.0;
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
	sp_object_read_attr (object, "inkscape:rounded");
}

static SPRepr *
sp_star_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPStar *star = SP_STAR (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("path");
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
		sp_repr_set_double (repr, "inkscape:rounded", star->rounded);
	}

	sp_star_set_shape ((SPShape *) star);
	char *d = sp_svg_write_path (((SPShape *) star)->curve->bpath);
	sp_repr_set_attr (repr, "d", d);
	g_free (d);

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
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_CX:
		if (!sp_svg_length_read_ldd (value, &unit, NULL, &star->center[NR::X]) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			star->center[NR::X] = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_CY:
		if (!sp_svg_length_read_ldd (value, &unit, NULL, &star->center[NR::Y]) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			star->center[NR::Y] = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_R1:
		if (!sp_svg_length_read_ldd (value, &unit, NULL, &star->r[0]) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			star->r[0] = 1.0;
		}
		/* fixme: Need CLAMP (Lauris) */
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_R2:
		if (!sp_svg_length_read_ldd (value, &unit, NULL, &star->r[1]) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			star->r[1] = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		return;
	case SP_ATTR_SODIPODI_ARG1:
		if (value) {
			star->arg[0] = g_ascii_strtod (value, NULL);
		} else {
			star->arg[0] = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_ARG2:
		if (value) {
			star->arg[1] = g_ascii_strtod (value, NULL);
		} else {
			star->arg[1] = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_FLATSIDED:
		if (value && !strcmp (value, "true"))
			star->flatsided = true;
		else star->flatsided = false;
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_ROUNDED:
		if (value) {
			star->rounded = g_ascii_strtod (value, NULL);
		} else {
			star->rounded = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
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

/**
Returns a unit-length vector at 90 degrees to the direction from o to n
 */
NR::Point
rot90_rel (NR::Point o, NR::Point n)
{
	return ((1/NR::L2(n - o)) * NR::Point ((n - o)[NR::Y],  (o - n)[NR::X]));
}

NR::Point
sp_star_get_curvepoint (SPStar *star, SPStarPoint point, gint index, bool previ)
{
	// the point whose neighboring curve handle we're calculating
	NR::Point o = sp_star_get_xy (star, point, index);

	// indices of previous and next points
	gint pi = (index > 0)? (index - 1) : (star->sides - 1);
	gint ni = (index < star->sides - 1)? (index + 1) : 0;

	// the other point type
	SPStarPoint other = (point == SP_STAR_POINT_KNOT2? SP_STAR_POINT_KNOT1 : SP_STAR_POINT_KNOT2);

	// the neighbors of o; depending on flatsided, they're either the same type (polygon) or the other type (star)
	NR::Point prev = (star->flatsided? sp_star_get_xy (star, point, pi) : sp_star_get_xy (star, other, point == SP_STAR_POINT_KNOT2? index : pi));
	NR::Point next = (star->flatsided? sp_star_get_xy (star, point, ni) : sp_star_get_xy (star, other, point == SP_STAR_POINT_KNOT1? index : ni));

	// prev-next midpoint
	NR::Point mid =  0.5 * (prev + next);

	// point to which we direct the bissector of the curve handles;
	// it's far enough outside the star on the perpendicular to prev-next through mid
	NR::Point biss =  mid + 100000 * rot90_rel (mid, next); 

	// lengths of vectors to prev and next
	gdouble prev_len = NR::L2 (prev - o);
	gdouble next_len = NR::L2 (next - o);

	// unit-length vector perpendicular to o-biss
	NR::Point rot = rot90_rel (o, biss);

	// multiply rot by star->rounded coefficient and the distance to the star point; flip for next
	NR::Point ret;
	if (previ) {
		ret = (star->rounded * prev_len) * rot;
	} else {
		ret = (star->rounded * next_len * -1) * rot;
	}

	// add the vector to o to get the final curvepoint
	return o + ret;
}


#define NEXT false
#define PREV true

static void
sp_star_set_shape (SPShape *shape)
{
	SPStar *star = SP_STAR (shape);

	SPCurve *c = sp_curve_new ();
	
	gint sides = star->sides;
	bool not_rounded = (fabs (star->rounded) < 1e-4);

	// draw 1st segment
	sp_curve_moveto (c, sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0));
	if (star->flatsided == false) {
		if (not_rounded) {
			sp_curve_lineto (c, sp_star_get_xy (star, SP_STAR_POINT_KNOT2, 0));
		} else {
			sp_curve_curveto (c, 
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, 0, NEXT),
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT2, 0, PREV),
				sp_star_get_xy (star, SP_STAR_POINT_KNOT2, 0));
		}
	}

	// draw all middle segments
	for (gint i = 1; i < sides; i++) {
		if (not_rounded) {
			sp_curve_lineto (c, sp_star_get_xy (star, SP_STAR_POINT_KNOT1, i));
		} else {
		if (star->flatsided == false) {
			sp_curve_curveto (c, 
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT2, i - 1, NEXT),
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, i, PREV),
				sp_star_get_xy (star, SP_STAR_POINT_KNOT1, i));
		} else {
			sp_curve_curveto (c, 
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, i - 1, NEXT),
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, i, PREV),
				sp_star_get_xy (star, SP_STAR_POINT_KNOT1, i));
		}
		}
		if (star->flatsided == false) {

			if (not_rounded) {
                       sp_curve_lineto (c, sp_star_get_xy (star, SP_STAR_POINT_KNOT2, i));
			} else {
				sp_curve_curveto (c,
					sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, i, NEXT),
					sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT2, i, PREV),
					sp_star_get_xy (star, SP_STAR_POINT_KNOT2, i));
			}
		}
	}
	
	// draw last segment
		if (star->rounded == 0) {
			sp_curve_lineto (c, sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0));
		} else {
			if (star->flatsided == false) {
			sp_curve_curveto (c, 
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT2, sides - 1, NEXT),
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, 0, PREV),
				sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0));
			} else {
			sp_curve_curveto (c, 
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, sides - 1, NEXT),
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, 0, PREV),
				sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0));
			}
		}

	sp_curve_closepath (c);
	sp_shape_set_curve_insync (SP_SHAPE (star), c, TRUE);
	sp_curve_unref (c);
}

void
sp_star_position_set (SPStar *star, gint sides, NR::Point center, gdouble r1, gdouble r2, gdouble arg1, gdouble arg2, bool isflat, double rounded)
{
	g_return_if_fail (star != NULL);
	g_return_if_fail (SP_IS_STAR (star));
	
	star->sides = CLAMP (sides, 3, 32);
	star->center = center;
	star->r[0] = MAX (r1, 0.001);
	if (isflat == false) {
		star->r[1] = CLAMP (r2, 0.0, star->r[0]);
	} else {
		star->r[1] = CLAMP ( r1*cos(M_PI/sides) ,0.0, star->r[0] );
	}
	star->arg[0] = arg1;
	star->arg[1] = arg2;
	star->flatsided = isflat;
	star->rounded = rounded;
	SP_OBJECT(star)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: We should use all corners of star (Lauris) */

static std::vector<NR::Point> sp_star_snappoints(SPItem const *item)
{
	std::vector<NR::Point> p;
  
	if (((SPItemClass *) parent_class)->snappoints) {
		p = ((SPItemClass *) parent_class)->snappoints (item);
	}
  
	return p;
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

