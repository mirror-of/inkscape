#define __SP_OBJECT_EDIT_C__

/*
 * Node editing extension to objects
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Mitsuru Oka
 *
 * Licensed under GNU GPL
 */

#include "config.h"

#include <math.h>

#include "sp-rect.h"
#include "sp-ellipse.h"
#include "sp-star.h"
#include "sp-spiral.h"
#include "sp-offset.h"
#include "prefs-utils.h"

#include "object-edit.h"

#include <libnr/nr-point-fns.h>
#include <libnr/nr-scale.h>
#include <libnr/nr-scale-ops.h>

#define sp_round(v,m) (((v) < 0.0) ? ((ceil ((v) / (m) - 0.5)) * (m)) : ((floor ((v) / (m) + 0.5)) * (m)))

static SPKnotHolder *sp_rect_knot_holder (SPItem *item, SPDesktop *desktop);
static SPKnotHolder *sp_arc_knot_holder (SPItem *item, SPDesktop *desktop);
static SPKnotHolder *sp_star_knot_holder (SPItem *item, SPDesktop *desktop);
static SPKnotHolder *sp_spiral_knot_holder (SPItem * item, SPDesktop *desktop);
static SPKnotHolder *sp_offset_knot_holder (SPItem * item, SPDesktop *desktop);

SPKnotHolder *
sp_item_knot_holder (SPItem *item, SPDesktop *desktop)
{
	const gchar *name;
	name = sp_object_tagName_get ((SPObject *) item, NULL);
	if (SP_IS_RECT (item)) {
		return sp_rect_knot_holder (item, desktop);
	} else if (SP_IS_ARC (item)) {
		return sp_arc_knot_holder (item, desktop);
	} else if (SP_IS_STAR (item)) {
		return sp_star_knot_holder (item, desktop);
	} else if (SP_IS_SPIRAL (item)) {
		return sp_spiral_knot_holder (item, desktop);
	} else if (SP_IS_OFFSET (item)) {
		return sp_offset_knot_holder (item, desktop);
	}
	return NULL;
}

/* SPRect */

static NR::Point sp_rect_rx_get (SPItem *item)
{
	SPRect *rect = SP_RECT (item);

	return NR::Point(rect->x.computed + rect->width.computed - rect->rx.computed, rect->y.computed);
}

static void
sp_rect_rx_set (SPItem *item, const NR::Point &p, guint state)
{
	SPRect *rect = SP_RECT(item);
	
	if (state & GDK_CONTROL_MASK) {
		gdouble temp = MIN (rect->height.computed, rect->width.computed) / 2.0;
		rect->rx.computed = rect->ry.computed = CLAMP (rect->x.computed + rect->width.computed - p[NR::X], 0.0, temp);
		rect->rx.set = rect->ry.set = TRUE;
	} else {
		rect->rx.computed = CLAMP (rect->x.computed + rect->width.computed - p[NR::X], 0.0, rect->width.computed / 2.0);
		rect->rx.set = TRUE;
	}
	sp_object_request_update ((SPObject *) rect, SP_OBJECT_MODIFIED_FLAG);
}


static NR::Point sp_rect_ry_get (SPItem *item)
{
	SPRect *rect = SP_RECT(item);

	return NR::Point(rect->x.computed + rect->width.computed, rect->y.computed + rect->ry.computed);
}

static void
sp_rect_ry_set (SPItem *item, const NR::Point &p, guint state)
{
	SPRect *rect = SP_RECT(item);
	
	if (state & GDK_CONTROL_MASK) {
		gdouble temp = MIN (rect->height.computed, rect->width.computed) / 2.0;
		rect->rx.computed = rect->ry.computed = CLAMP (p[NR::Y] - rect->y.computed, 0.0, temp);
		rect->ry.set = rect->rx.set = TRUE;
	} else {
		if (!rect->rx.set || rect->rx.computed == 0) 
			rect->ry.computed = CLAMP (p[NR::Y] - rect->y.computed, 0.0, MIN(rect->height.computed / 2.0, rect->width.computed / 2.0));
		else 
			rect->ry.computed = CLAMP (p[NR::Y] - rect->y.computed, 0.0, rect->height.computed / 2.0);
		rect->ry.set = TRUE;
	}
	sp_object_request_update ((SPObject *) rect, SP_OBJECT_MODIFIED_FLAG);
}

static NR::Point sp_rect_wh_get (SPItem *item)
{
	SPRect *rect = SP_RECT(item);

	return NR::Point(rect->x.computed + rect->width.computed, rect->y.computed + rect->height.computed);
}

static void
sp_rect_wh_set (SPItem *item, const NR::Point &p, guint state)
{
	SPRect *rect = SP_RECT(item);
	
	if (state & GDK_CONTROL_MASK) {
		gdouble ratio = (rect->width.computed / rect->height.computed);
		gdouble minx = p[NR::X] - (rect->x.computed + rect->width.computed);
		gdouble miny = p[NR::Y] - (rect->y.computed + rect->height.computed);
		if (minx > miny) {
			rect->width.computed += miny * ratio;
			rect->height.computed += miny;
		} else {
			rect->width.computed += minx;
			rect->height.computed += minx / ratio;
		}
		rect->width.set = rect->height.set = TRUE;
	} else {
		rect->width.computed = fabs (p[NR::X] - rect->x.computed);
		if (2 * rect->rx.computed > rect->width.computed) {
			rect->rx.computed = 0.5 * rect->width.computed;
			rect->rx.set = TRUE;
		}
		rect->height.computed = fabs (p[NR::Y] - rect->y.computed);
		if (2 * rect->ry.computed > rect->height.computed) {
			rect->ry.computed = 0.5 * rect->height.computed;
			rect->ry.set = TRUE;
		}
		rect->width.set = rect->height.set = TRUE;
	}
	sp_object_request_update ((SPObject *) rect, SP_OBJECT_MODIFIED_FLAG);
}



static SPKnotHolder *
sp_rect_knot_holder (SPItem *item, SPDesktop *desktop)
{
	SPKnotHolder *knot_holder = sp_knot_holder_new (desktop, item, NULL);
	
	sp_knot_holder_add (knot_holder, sp_rect_rx_set, sp_rect_rx_get);
	sp_knot_holder_add (knot_holder, sp_rect_ry_set, sp_rect_ry_get);
	sp_knot_holder_add (knot_holder, sp_rect_wh_set, sp_rect_wh_get);
	
	return knot_holder;
}

/* SPArc */

/*
 * return values:
 *   1  : inside
 *   0  : on the curves
 *   -1 : outside
 */
static gint
sp_genericellipse_side (SPGenericEllipse *ellipse, const NR::Point &p)
{
	gdouble dx = (p[NR::X] - ellipse->cx.computed) / ellipse->rx.computed;
	gdouble dy = (p[NR::Y] - ellipse->cy.computed) / ellipse->ry.computed;

	gdouble s = dx * dx + dy * dy;
	if (s < 1.0) return 1;
	if (s > 1.0) return -1;
	return 0;
}

static void
sp_arc_start_set (SPItem *item, const NR::Point &p, guint state)
{
	int snaps = prefs_get_int_attribute ("options.rotationsnapsperpi", "value", 12);

	SPGenericEllipse *ge = SP_GENERICELLIPSE (item);
	SPArc *arc = SP_ARC(item);

	ge->closed = (sp_genericellipse_side (ge, p) == -1) ? TRUE : FALSE;

	NR::Point delta = p - NR::Point(ge->cx.computed, ge->cy.computed);
	NR::scale sc(ge->rx.computed, ge->ry.computed);
	ge->start = atan2 (delta * sc.inverse());
	if ( ( state & GDK_CONTROL_MASK )
	     && snaps )
	{
		ge->start = sp_round(ge->start, M_PI/snaps);
	}
	sp_genericellipse_normalize (ge);
	sp_object_request_update ((SPObject *) arc, SP_OBJECT_MODIFIED_FLAG);
}

static NR::Point sp_arc_start_get (SPItem *item)
{
	SPGenericEllipse *ge = SP_GENERICELLIPSE (item);
	SPArc *arc = SP_ARC (item);

	return sp_arc_get_xy (arc, ge->start);
}

static void
sp_arc_end_set (SPItem *item, const NR::Point &p, guint state)
{
	int snaps = prefs_get_int_attribute ("options.rotationsnapsperpi", "value", 12);

	SPGenericEllipse *ge = SP_GENERICELLIPSE (item);
	SPArc *arc = SP_ARC(item);

	ge->closed = (sp_genericellipse_side (ge, p) == -1) ? TRUE : FALSE;

	NR::Point delta = p - NR::Point(ge->cx.computed, ge->cy.computed);
	NR::scale sc(ge->rx.computed, ge->ry.computed);
	ge->end = atan2 (delta * sc.inverse());
	if ( ( state & GDK_CONTROL_MASK )
	     && snaps )
	{
		ge->end = sp_round(ge->end, M_PI/snaps);
	}
	sp_genericellipse_normalize (ge);
	sp_object_request_update ((SPObject *) arc, SP_OBJECT_MODIFIED_FLAG);
}

static NR::Point sp_arc_end_get (SPItem *item)
{
	SPGenericEllipse *ge = SP_GENERICELLIPSE (item);
	SPArc *arc = SP_ARC (item);

	return sp_arc_get_xy (arc, ge->end);
}

static SPKnotHolder *
sp_arc_knot_holder (SPItem *item, SPDesktop *desktop)
{
	SPKnotHolder *knot_holder = sp_knot_holder_new (desktop, item, NULL);
	
	sp_knot_holder_add (knot_holder,
			    sp_arc_start_set,
			    sp_arc_start_get);
	sp_knot_holder_add (knot_holder,
			    sp_arc_end_set,
			    sp_arc_end_get);
	
	return knot_holder;
}

/* SPStar */

static void
sp_star_knot1_set (SPItem *item, const NR::Point &p, guint state)
{
	SPStar *star = SP_STAR (item);


	NR::Point d = p - star->center;

        double arg1 = atan2(d);
	double darg1 = arg1 - star->arg[0];
        
	if (state & GDK_CONTROL_MASK) {
		star->r[0]    = L2(d);
	} else {		
		star->r[0]    = L2(d);
		star->arg[0]  = arg1;
		star->arg[1] += darg1;
	}
	sp_object_request_update ((SPObject *) star, SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_star_knot2_set (SPItem *item, const NR::Point &p, guint state)
{
	SPStar *star = SP_STAR (item);
    if (star->flatsided == false) {
	NR::Point d = p - star->center;

	    if (state & GDK_CONTROL_MASK)
	        {
		star->r[1]   = L2(d);
		star->arg[1] = star->arg[0] + M_PI / star->sides;
	        }
	    else {
		star->r[1]   = L2(d);
		star->arg[1] = atan2 (d);
	}
	sp_object_request_update ((SPObject *) star, SP_OBJECT_MODIFIED_FLAG);}
}

static NR::Point sp_star_knot1_get (SPItem *item)
{
	g_assert (item != NULL);

	SPStar *star = SP_STAR(item);

	return sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0);

}

static NR::Point sp_star_knot2_get (SPItem *item)
{
	g_assert (item != NULL);

	SPStar *star = SP_STAR(item);

	return sp_star_get_xy (star, SP_STAR_POINT_KNOT2, 0);
}

static SPKnotHolder *
sp_star_knot_holder (SPItem *item, SPDesktop *desktop)
{
	/* we don't need to get parent knot_holder */
	SPKnotHolder *knot_holder = sp_knot_holder_new (desktop, item, NULL);
	g_assert (item != NULL);

	SPStar *star = SP_STAR(item);


	sp_knot_holder_add (knot_holder, sp_star_knot1_set, sp_star_knot1_get);

	if (star->flatsided == false)
	    sp_knot_holder_add (knot_holder, sp_star_knot2_set, sp_star_knot2_get);

	return knot_holder;
}

/* SPSpiral */

/*
 * set attributes via inner (t=t0) knot point:
 *   [default] increase/decrease inner point
 *   [shift]   increase/decrease inner and outer arg synchronizely
 *   [control] constrain inner arg to round per PI/4
 */
static void
sp_spiral_inner_set (SPItem *item, const NR::Point &p, guint state)
{
	int snaps = prefs_get_int_attribute ("options.rotationsnapsperpi", "value", 12);

	SPSpiral *spiral = SP_SPIRAL (item);

	gdouble   dx = p[NR::X] - spiral->cx;
	gdouble   dy = p[NR::Y] - spiral->cy;
	gdouble   arg_t0;
	sp_spiral_get_polar (spiral, spiral->t0, NULL, &arg_t0);
	arg_t0 = 2.0*M_PI*spiral->revo * spiral->t0 + spiral->arg; //
	gdouble   arg_tmp = atan2(dy, dx) - arg_t0;
	gdouble   arg_t0_new = arg_tmp - floor((arg_tmp+M_PI)/(2.0*M_PI))*2.0*M_PI + arg_t0;
	spiral->t0 = (arg_t0_new - spiral->arg) / (2.0*M_PI*spiral->revo);

	/* round inner arg per PI/snaps, if CTRL is pressed */
	if ( ( state & GDK_CONTROL_MASK )
	     && ( fabs(spiral->revo) > SP_EPSILON_2 )
	     && ( snaps != 0 ) )
	{
		gdouble arg = 2.0*M_PI*spiral->revo*spiral->t0 + spiral->arg;
		spiral->t0 = (round(arg/(M_PI/snaps))*(M_PI/snaps) - spiral->arg)/(2.0*M_PI*spiral->revo);
	}

	spiral->t0 = CLAMP (spiral->t0, 0.0, 0.999);

	sp_object_request_update ((SPObject *) spiral, SP_OBJECT_MODIFIED_FLAG);
}

/*
 * set attributes via outer (t=1) knot point:
 *   [default] increase/decrease revolution factor
 *   [control] constrain inner arg to round per PI/4
 */
static void
sp_spiral_outer_set (SPItem *item, const NR::Point &p, guint state)
{
	int snaps = prefs_get_int_attribute ("options.rotationsnapsperpi", "value", 12);

	SPSpiral *spiral = SP_SPIRAL (item);

	gdouble  dx = p[NR::X] - spiral->cx;
	gdouble  dy = p[NR::Y] - spiral->cy;
	spiral->arg = atan2(dy, dx) - 2.0*M_PI*spiral->revo;

	if (!(state & GDK_MOD1_MASK)) {
		spiral->rad = MAX (hypot (dx, dy), 0.001);
	}

	if ( ( state & GDK_CONTROL_MASK )
	     && snaps ) {
		spiral->arg = round (spiral->arg/(M_PI/snaps))*(M_PI/snaps);
	}

	sp_object_request_update ((SPObject *) spiral, SP_OBJECT_MODIFIED_FLAG);
}

static NR::Point sp_spiral_inner_get (SPItem *item)
{
	SPSpiral *spiral = SP_SPIRAL (item);

	return sp_spiral_get_xy (spiral, spiral->t0);
}

static NR::Point sp_spiral_outer_get (SPItem *item)
{
	SPSpiral *spiral = SP_SPIRAL (item);

	return sp_spiral_get_xy (spiral, 1.0);
}

static SPKnotHolder *
sp_spiral_knot_holder (SPItem * item, SPDesktop *desktop)
{
	SPKnotHolder *knot_holder = sp_knot_holder_new (desktop, item, NULL);

	sp_knot_holder_add (knot_holder,
			    sp_spiral_inner_set,
			    sp_spiral_inner_get);
	sp_knot_holder_add (knot_holder,
			    sp_spiral_outer_set,
			    sp_spiral_outer_get);

	return knot_holder;
}

/* SPOffset */

static void
sp_offset_offset_set (SPItem *item, const NR::Point &p, guint state)
{
	SPOffset *offset = SP_OFFSET (item);
	
	offset->rad = sp_offset_distance_to_original(offset, p);
	offset->knot = p;
	offset->knotSet=true;
	
	sp_object_request_update ((SPObject *) offset, SP_OBJECT_MODIFIED_FLAG);
}


static NR::Point sp_offset_offset_get (SPItem *item)
{
	SPOffset *offset = SP_OFFSET (item);
	
	NR::Point np;
	sp_offset_top_point(offset,&np);
	return np;
}

static SPKnotHolder *
sp_offset_knot_holder (SPItem * item, SPDesktop *desktop)
{
	SPKnotHolder *knot_holder = sp_knot_holder_new (desktop, item, NULL);

	sp_knot_holder_add (knot_holder,
			    sp_offset_offset_set,
			    sp_offset_offset_get);

	return knot_holder;
}

