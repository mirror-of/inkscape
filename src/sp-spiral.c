#define __SP_SPIRAL_C__

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

#include <config.h>

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "svg/svg.h"
#include "attributes.h"
#include "helper/bezier-utils.h"
#include "dialogs/object-attributes.h"
#include "helper/sp-intl.h"

#include "sp-spiral.h"

#define noSPIRAL_VERBOSE

#define SP_EPSILON       1e-5
#define SP_EPSILON_2     (SP_EPSILON * SP_EPSILON)
#define SP_HUGE          1e5

#define SPIRAL_TOLERANCE 3.0
#define SAMPLE_STEP      (1.0/4.0) /* step per 2PI */
#define SAMPLE_SIZE      8      /* sample size per one bezier */


static void sp_spiral_class_init (SPSpiralClass *class);
static void sp_spiral_init (SPSpiral *spiral);

static void sp_spiral_build (SPObject * object, SPDocument * document, SPRepr * repr);
static SPRepr *sp_spiral_write (SPObject *object, SPRepr *repr, guint flags);
static void sp_spiral_set (SPObject *object, unsigned int key, const unsigned char *value);
static void sp_spiral_update (SPObject *object, SPCtx *ctx, guint flags);

static gchar * sp_spiral_description (SPItem * item);
static int sp_spiral_snappoints (SPItem *item, NRPointF *p, int size);
static void sp_spiral_set_shape (SPShape *shape);

static SPShapeClass *parent_class;

GType
sp_spiral_get_type (void)
{
	static GType spiral_type = 0;

	if (!spiral_type) {
		GTypeInfo spiral_info = {
			sizeof (SPSpiralClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_spiral_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPSpiral),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_spiral_init,
		};
		spiral_type = g_type_register_static (SP_TYPE_SHAPE, "SPSPiral", &spiral_info, 0);
	}
	return spiral_type;
}

static void
sp_spiral_class_init (SPSpiralClass *class)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	SPShapeClass *shape_class;

	gobject_class = (GObjectClass *) class;
	sp_object_class = (SPObjectClass *) class;
	item_class = (SPItemClass *) class;
	shape_class = (SPShapeClass *) class;

	parent_class = g_type_class_ref (SP_TYPE_SHAPE);

	sp_object_class->build = sp_spiral_build;
	sp_object_class->write = sp_spiral_write;
	sp_object_class->set = sp_spiral_set;
	sp_object_class->update = sp_spiral_update;

	item_class->description = sp_spiral_description;
	item_class->snappoints = sp_spiral_snappoints;

	shape_class->set_shape = sp_spiral_set_shape;
}

static void
sp_spiral_init (SPSpiral * spiral)
{
	spiral->cx         = 0.0;
	spiral->cy         = 0.0;
	spiral->exp        = 1.0;
	spiral->revo       = 3.0;
	spiral->rad        = 1.0;
	spiral->arg        = 0.0;
	spiral->t0         = 0.0;
}

static void
sp_spiral_build (SPObject * object, SPDocument * document, SPRepr * repr)
{
	if (((SPObjectClass *) parent_class)->build)
		((SPObjectClass *) parent_class)->build (object, document, repr);

	sp_object_read_attr (object, "sodipodi:cx");
	sp_object_read_attr (object, "sodipodi:cy");
	sp_object_read_attr (object, "sodipodi:expansion");
	sp_object_read_attr (object, "sodipodi:revolution");
	sp_object_read_attr (object, "sodipodi:radius");
	sp_object_read_attr (object, "sodipodi:argument");
	sp_object_read_attr (object, "sodipodi:t0");
}

static SPRepr *
sp_spiral_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPSpiral *spiral;
	char *d;

	spiral = SP_SPIRAL (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("path");
	}

	if (flags & SP_OBJECT_WRITE_SODIPODI) {
		/* Fixme: we may replace these attributes by
		 * sodipodi:spiral="cx cy exp revo rad arg t0"
		 */
		sp_repr_set_attr (repr, "sodipodi:type", "spiral");
		sp_repr_set_double_attribute (repr, "sodipodi:cx", spiral->cx);
		sp_repr_set_double_attribute (repr, "sodipodi:cy", spiral->cy);
		sp_repr_set_double_attribute (repr, "sodipodi:expansion", spiral->exp);
		sp_repr_set_double_attribute (repr, "sodipodi:revolution", spiral->revo);
		sp_repr_set_double_attribute (repr, "sodipodi:radius", spiral->rad);
		sp_repr_set_double_attribute (repr, "sodipodi:argument", spiral->arg);
		sp_repr_set_double_attribute (repr, "sodipodi:t0", spiral->t0);
	}

	d = sp_svg_write_path (((SPShape *) spiral)->curve->bpath);
	sp_repr_set_attr (repr, "d", d);
	g_free (d);

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags | SP_SHAPE_WRITE_PATH);

	return repr;
}

static void
sp_spiral_set (SPObject *object, unsigned int key, const unsigned char *value)
{
	SPSpiral *spiral;
	SPShape  *shape;
	gulong unit;

	spiral = SP_SPIRAL (object);
	shape  = SP_SHAPE (object);

	/* fixme: we should really collect updates */
	switch (key) {
	case SP_ATTR_SODIPODI_CX:
		if (!sp_svg_length_read_lff (value, &unit, NULL, &spiral->cx) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			spiral->cx = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_CY:
		if (!sp_svg_length_read_lff (value, &unit, NULL, &spiral->cy) ||
		    (unit == SP_SVG_UNIT_EM) ||
		    (unit == SP_SVG_UNIT_EX) ||
		    (unit == SP_SVG_UNIT_PERCENT)) {
			spiral->cy = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_EXPANSION:
		if (value) {
			spiral->exp = atof (value);
			spiral->exp = CLAMP (spiral->exp, 0.0, 1000.0);
		} else {
			spiral->exp = 1.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_REVOLUTION:
		if (value) {
			spiral->revo = atof (value);
			spiral->revo = CLAMP (spiral->revo, 0.05, 20.0);
		} else {
			spiral->revo = 3.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_RADIUS:
		if (!sp_svg_length_read_lff (value, &unit, NULL, &spiral->rad) ||
		    (unit != SP_SVG_UNIT_EM) ||
		    (unit != SP_SVG_UNIT_EX) ||
		    (unit != SP_SVG_UNIT_PERCENT)) {
			spiral->rad = MAX (spiral->rad, 0.001);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_ARGUMENT:
		if (value) {
			spiral->arg = atof (value);
		} else {
			spiral->arg = 0.0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_T0:
		if (value) {
			spiral->t0 = atof (value);
			spiral->t0 = CLAMP (spiral->t0, -1.0, 0.999);
		} else {
			spiral->t0 = 0.0;
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
sp_spiral_update (SPObject *object, SPCtx *ctx, guint flags)
{
	if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
		sp_shape_set_shape ((SPShape *) object);
	}

	if (((SPObjectClass *) parent_class)->update)
		((SPObjectClass *) parent_class)->update (object, ctx, flags);
}

static gchar *
sp_spiral_description (SPItem * item)
{
	return g_strdup ("Spiral");
}

static void
sp_spiral_fit_and_draw (SPSpiral *spiral,
			SPCurve	 *c,
			double dstep,
			NRPointF *darray,
			NRPointF *hat1,
			NRPointF *hat2,
			double    t)
{
#define BEZIER_SIZE   4
#define FITTING_DEPTH 3
#define BEZIER_LENGTH (BEZIER_SIZE * (2 << (FITTING_DEPTH - 1)))

	NRPointF bezier[BEZIER_LENGTH];
	double d;
	int depth, i;
	
	for (d = t, i = 0; i <= SAMPLE_SIZE; d += dstep, i++) {
		sp_spiral_get_xy (spiral, d, &darray[i]);
	}
	
	sp_darray_center_tangent (darray, SAMPLE_SIZE - 1, hat2);
	
	/* Fixme:
	   we should use better algorithm to specify maximum error.
	*/
	depth = sp_bezier_fit_cubic_full (bezier, darray, SAMPLE_SIZE,
					  hat1, hat2,
					  SPIRAL_TOLERANCE*SPIRAL_TOLERANCE,
					  FITTING_DEPTH);
#ifdef SPIRAL_DEBUG
	if (t==spiral->t0 || t==1.0)
		g_print ("[%s] depth=%d, dstep=%g, t0=%g, t=%g, arg=%g\n",
			 debug_state, depth, dstep, spiral->t0, t, spiral->arg);
#endif
	if (depth != -1) {
		for (i = 0; i < 4*depth; i += 4) {
			sp_curve_curveto (c, bezier[i + 1].x, bezier[i + 1].y,
					  bezier[i + 2].x, bezier[i + 2].y,
					  bezier[i + 3].x, bezier[i + 3].y);
#ifdef SPIRAL_DEBUG
			if (debug_fit_and_draw)
				g_print("[(%g,%g)-(%g,%g)-(%g,%g)]\n", bezier[i+1].x, bezier[i+1].y, bezier[i+2].x, bezier[i+2].y, bezier[i+3].x, bezier[i+3].y);
#endif
		}
	} else {
#ifdef SPIRAL_VERBOSE
		g_print ("cant_fit_cubic: t=%g\n", t);
#endif
		for (i = 1; i < SAMPLE_SIZE; i++)
			sp_curve_lineto (c, darray[i].x, darray[i].y);
	}
}

static void
sp_spiral_set_shape (SPShape *shape)
{
	SPSpiral *spiral;
	NRPointF darray[SAMPLE_SIZE + 1];
	NRPointF hat1, hat2;
	int i;
	double tstep, t;
	double dstep, d;
	SPCurve *c;

	spiral = SP_SPIRAL(shape);

	sp_object_request_modified (SP_OBJECT (spiral), SP_OBJECT_MODIFIED_FLAG);

#if 0
	if (spiral->rad < SP_EPSILON) return;
#endif
	
	c = sp_curve_new ();
	
#ifdef SPIRAL_VERBOSE
	g_print ("ex=%g, revo=%g, rad=%g, arg=%g, t0=%g\n",
		 spiral->cx,
		 spiral->cy,
		 spiral->exp,
		 spiral->revo,
		 spiral->rad,
		 spiral->arg,
		 spiral->t0);
#endif
	
	tstep = SAMPLE_STEP/spiral->revo;
	dstep = tstep/(SAMPLE_SIZE - 1.0);

	if (spiral->t0 - dstep >= 0.0) {
		for (d = spiral->t0 - dstep, i = 0; i <= 2; d += dstep, i++)
			sp_spiral_get_xy (spiral, d, &darray[i]);

		sp_darray_center_tangent (darray, 1, &hat1);
		hat1.x = - hat1.x;
		hat1.y = - hat1.y;
	} else {
		for (d = spiral->t0, i = 1; i <= 2; d += dstep, i++)
			sp_spiral_get_xy (spiral, d, &darray[i]);

		sp_darray_left_tangent (darray, 1, 2, &hat1);
	}

	sp_curve_moveto (c, darray[1].x, darray[1].y);

	for (t = spiral->t0; t < (1.0-tstep); t += tstep)
	{
		sp_spiral_fit_and_draw (spiral, c, dstep, darray, &hat1, &hat2, t);

		hat1.x = - hat2.x;
		hat1.y = - hat2.y;
	}
	if ((1.0 - t) > SP_EPSILON)
		sp_spiral_fit_and_draw (spiral, c, (1.0 - t)/(SAMPLE_SIZE - 1.0),
					darray, &hat1, &hat2, t);
  
	sp_shape_set_curve_insync ((SPShape *) spiral, c, TRUE);
	sp_curve_unref (c);
}

void
sp_spiral_position_set       (SPSpiral          *spiral,
		     gdouble            cx,
		     gdouble            cy,
		     gdouble            exp,
		     gdouble            revo,
		     gdouble            rad,
		     gdouble            arg,
		     gdouble            t0)
{
	g_return_if_fail (spiral != NULL);
	g_return_if_fail (SP_IS_SPIRAL (spiral));
	
	spiral->cx         = cx;
	spiral->cy         = cy;
	spiral->exp        = exp;
	spiral->revo       = revo;
	spiral->rad        = MAX (rad, 0.001);
	spiral->arg        = arg;
	spiral->t0         = t0;
	
	sp_object_request_update ((SPObject *) spiral, SP_OBJECT_MODIFIED_FLAG);
}

static int
sp_spiral_snappoints (SPItem *item, NRPointF *p, int size)
{
#if 0
	/* fixme: (Lauris) */
	SPSpiral *spiral;
	ArtPoint * p, p1, p2, p3;
	gdouble affine[6];
	
	spiral = SP_SPIRAL(item);
	
	sp_spiral_get_xy (spiral, 0.0, &p1);
	sp_spiral_get_xy (spiral, spiral->t0, &p2);
	sp_spiral_get_xy (spiral, 1.0, &p3);
	
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

void
sp_spiral_get_xy (SPSpiral *spiral, gdouble t, NRPointF *p)
{
	gdouble rad, arg;

	g_return_if_fail (spiral != NULL);
	g_return_if_fail (SP_IS_SPIRAL(spiral));
	g_return_if_fail (p != NULL);

	rad = spiral->rad * pow(t, spiral->exp);
	arg = 2.0 * M_PI * spiral->revo * t + spiral->arg;
	
	p->x = rad * cos (arg) + spiral->cx;
	p->y = rad * sin (arg) + spiral->cy;
}

void
sp_spiral_get_polar (SPSpiral *spiral, gdouble t, gdouble *rad, gdouble *arg)
{
	g_return_if_fail (spiral != NULL);
	g_return_if_fail (SP_IS_SPIRAL(spiral));

	if (rad)
		*rad = spiral->rad * pow(t, spiral->exp);
	if (arg)
		*arg = 2.0 * M_PI * spiral->revo * t + spiral->arg;
}

gboolean
sp_spiral_is_invalid (SPSpiral *spiral)
{
	gdouble rad;

	sp_spiral_get_polar (spiral, 0.0, &rad, NULL);
	if (rad < 0.0 || rad > SP_HUGE) {
		g_print ("rad(t=0)=%g\n", rad);
		return TRUE;
	}
	sp_spiral_get_polar (spiral, 1.0, &rad, NULL);
	if (rad < 0.0 || rad > SP_HUGE) {
		g_print ("rad(t=1)=%g\n", rad);
		return TRUE;
	}
	return FALSE;
}

