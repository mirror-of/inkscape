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

#include "object-edit.h"

#define sp_round(v,m) (((v) < 0.0) ? ((ceil ((v) / (m) - 0.5)) * (m)) : ((floor ((v) / (m) + 0.5)) * (m)))

static SPKnotHolder *sp_rect_knot_holder (SPItem *item, SPDesktop *desktop);
static SPKnotHolder *sp_arc_knot_holder (SPItem *item, SPDesktop *desktop);
static SPKnotHolder *sp_star_knot_holder (SPItem *item, SPDesktop *desktop);
static SPKnotHolder *sp_spiral_knot_holder (SPItem * item, SPDesktop *desktop);

SPKnotHolder *
sp_item_knot_holder (SPItem *item, SPDesktop *desktop)
{
	const unsigned char *name;
	name = sp_object_tagName_get ((SPObject *) item, NULL);
	if (SP_IS_RECT (item)) {
		return sp_rect_knot_holder (item, desktop);
	} else if (SP_IS_ARC (item)) {
		return sp_arc_knot_holder (item, desktop);
	} else if (SP_IS_STAR (item)) {
		return sp_star_knot_holder (item, desktop);
	} else if (SP_IS_SPIRAL (item)) {
		return sp_spiral_knot_holder (item, desktop);
	}
	return NULL;
}

/* SPRect */

static void
sp_rect_rx_get (SPItem *item, NRPointF *p)
{
	SPRect *rect;

	rect = SP_RECT (item);

	p->x = rect->x.computed + rect->rx.computed;
	p->y = rect->y.computed;
}

static void
sp_rect_rx_set (SPItem *item, const NRPointF *p, guint state)
{
	SPRect *rect;

	rect = SP_RECT(item);
	
	if (state & GDK_CONTROL_MASK) {
		gdouble temp = MIN (rect->height.computed, rect->width.computed) / 2.0;
		rect->rx.computed = rect->ry.computed = CLAMP (p->x - rect->x.computed, 0.0, temp);
		rect->rx.set = rect->ry.set = TRUE;
	} else {
		rect->rx.computed = CLAMP (p->x - rect->x.computed, 0.0, rect->width.computed / 2.0);
		rect->rx.set = TRUE;
	}
	sp_object_request_update ((SPObject *) rect, SP_OBJECT_MODIFIED_FLAG);
}


static void
sp_rect_ry_get (SPItem *item, NRPointF *p)
{
	SPRect *rect;

	rect = SP_RECT(item);

	p->x = rect->x.computed;
	p->y = rect->y.computed + rect->ry.computed;
}

static void
sp_rect_ry_set (SPItem *item, const NRPointF *p, guint state)
{
	SPRect *rect;

	rect = SP_RECT(item);
	
	if (state & GDK_CONTROL_MASK) {
		gdouble temp = MIN (rect->height.computed, rect->width.computed) / 2.0;
		rect->rx.computed = rect->ry.computed = CLAMP (p->y - rect->y.computed, 0.0, temp);
		rect->ry.set = rect->rx.set = TRUE;
	} else {
		rect->ry.computed = CLAMP (p->y - rect->y.computed, 0.0, rect->height.computed / 2.0);
		rect->ry.set = TRUE;
	}
	sp_object_request_update ((SPObject *) rect, SP_OBJECT_MODIFIED_FLAG);
}

static SPKnotHolder *
sp_rect_knot_holder (SPItem *item, SPDesktop *desktop)
{
	SPRect *rect;
	SPKnotHolder *knot_holder;

	rect = (SPRect *) item;
	knot_holder = sp_knot_holder_new (desktop, item, NULL);
	
	sp_knot_holder_add (knot_holder, sp_rect_rx_set, sp_rect_rx_get);
	sp_knot_holder_add (knot_holder, sp_rect_ry_set, sp_rect_ry_get);
	
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
sp_genericellipse_side (SPGenericEllipse *ellipse, const NRPointF *p)
{
	gdouble dx, dy;
	gdouble s;

	dx = p->x - ellipse->cx.computed;
	dy = p->y - ellipse->cy.computed;

	s = dx * dx / (ellipse->rx.computed * ellipse->rx.computed) + dy * dy / (ellipse->ry.computed * ellipse->ry.computed);
	if (s < 1.0) return 1;
	if (s > 1.0) return -1;
	return 0;
}

static void
sp_arc_start_set (SPItem *item, const NRPointF *p, guint state)
{
	SPGenericEllipse *ge;
	SPArc *arc;
	gdouble dx, dy;

	ge = SP_GENERICELLIPSE (item);
	arc = SP_ARC(item);

	ge->closed = (sp_genericellipse_side (ge, p) == -1) ? TRUE : FALSE;

	dx = p->x - ge->cx.computed;
	dy = p->y - ge->cy.computed;

	ge->start = atan2 (dy / ge->ry.computed, dx / ge->rx.computed);
	if (state & GDK_CONTROL_MASK) {
		ge->start = sp_round(ge->start, M_PI_4);
	}
	sp_genericellipse_normalize (ge);
	sp_object_request_update ((SPObject *) arc, SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_arc_start_get (SPItem *item, NRPointF *p)
{
	SPGenericEllipse *ge;
	SPArc *arc;

	ge = SP_GENERICELLIPSE (item);
	arc = SP_ARC (item);

	sp_arc_get_xy (arc, ge->start, p);
}

static void
sp_arc_end_set (SPItem *item, const NRPointF *p, guint state)
{
	SPGenericEllipse *ge;
	SPArc *arc;
	gdouble dx, dy;

	ge = SP_GENERICELLIPSE (item);
	arc = SP_ARC(item);

	ge->closed = (sp_genericellipse_side (ge, p) == -1) ? TRUE : FALSE;

	dx = p->x - ge->cx.computed;
	dy = p->y - ge->cy.computed;
	ge->end = atan2 (dy / ge->ry.computed, dx / ge->rx.computed);
	if (state & GDK_CONTROL_MASK) {
		ge->end = sp_round(ge->end, M_PI_4);
	}
	sp_genericellipse_normalize (ge);
	sp_object_request_update ((SPObject *) arc, SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_arc_end_get (SPItem *item, NRPointF *p)
{
	SPGenericEllipse *ge;
	SPArc *arc;

	ge = SP_GENERICELLIPSE (item);
	arc = SP_ARC (item);

	sp_arc_get_xy (arc, ge->end, p);
}

static SPKnotHolder *
sp_arc_knot_holder (SPItem *item, SPDesktop *desktop)
{
	SPArc *arc;
	SPKnotHolder *knot_holder;

	arc = SP_ARC (item);
	knot_holder = sp_knot_holder_new (desktop, item, NULL);
	
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
sp_star_knot1_set (SPItem *item, const NRPointF *p, guint state)
{
	SPStar *star;
	gdouble dx, dy, arg1, darg1;

	star = SP_STAR (item);

	dx = p->x - star->cx;
	dy = p->y - star->cy;

        arg1 = atan2 (dy, dx);
	darg1 = arg1 - star->arg1;
        
	if (state & GDK_CONTROL_MASK) {
		star->r1    = hypot(dx, dy);
	} else {		
		star->r1    = hypot(dx, dy);
		star->arg1  = arg1;
		star->arg2 += darg1;
	}
	sp_object_request_update ((SPObject *) star, SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_star_knot2_set (SPItem *item, const NRPointF *p, guint state)
{
	SPStar *star;
	gdouble dx, dy;

	star = SP_STAR (item);

	dx = p->x - star->cx;
	dy = p->y - star->cy;

	if (state & GDK_CONTROL_MASK) {
		star->r2   = hypot (dx, dy);
		star->arg2 = star->arg1 + M_PI / star->sides;
	} else {
		star->r2   = hypot (dx, dy);
		star->arg2 = atan2 (dy, dx);
	}
	sp_object_request_update ((SPObject *) star, SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_star_knot1_get (SPItem *item, NRPointF *p)
{
	SPStar *star;

	g_return_if_fail (item != NULL);
	g_return_if_fail (p != NULL);

	star = SP_STAR(item);

	sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0, p);
}

static void
sp_star_knot2_get (SPItem *item, NRPointF *p)
{
	SPStar *star;

	g_return_if_fail (item != NULL);
	g_return_if_fail (p != NULL);

	star = SP_STAR(item);

	sp_star_get_xy (star, SP_STAR_POINT_KNOT2, 0, p);
}

static SPKnotHolder *
sp_star_knot_holder (SPItem *item, SPDesktop *desktop)
{
	SPStar  *star;
	SPKnotHolder *knot_holder;
	
	star = SP_STAR(item);

#if 0
	/* if you want to get parent knot_holder, do it */
	if (SP_ITEM_CLASS(parent_class)->knot_holder)
		knot_holder = (* SP_ITEM_CLASS(parent_class)->knot_holder) (item);
#else
	/* we don't need to get parent knot_holder */
	knot_holder = sp_knot_holder_new (desktop, item, NULL);
#endif

	sp_knot_holder_add (knot_holder,
			    sp_star_knot1_set,
			    sp_star_knot1_get);
	sp_knot_holder_add (knot_holder,
			    sp_star_knot2_set,
			    sp_star_knot2_get);

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
sp_spiral_inner_set (SPItem *item, const NRPointF *p, guint state)
{
	SPSpiral *spiral;
	gdouble   dx, dy;
	gdouble   arg_tmp;
	gdouble   arg_t0;
	gdouble   arg_t0_new;

	spiral = SP_SPIRAL (item);

	dx = p->x - spiral->cx;
	dy = p->y - spiral->cy;
	sp_spiral_get_polar (spiral, spiral->t0, NULL, &arg_t0);
/*  	arg_t0 = 2.0*M_PI*spiral->revo * spiral->t0 + spiral->arg; */
	arg_tmp = atan2(dy, dx) - arg_t0;
	arg_t0_new = arg_tmp - floor((arg_tmp+M_PI)/(2.0*M_PI))*2.0*M_PI + arg_t0;
	spiral->t0 = (arg_t0_new - spiral->arg) / (2.0*M_PI*spiral->revo);
#if 0				/* we need round function */
	/* round inner arg per PI/4, if CTRL is pressed */
	if ((state & GDK_CONTROL_MASK) &&
	    (fabs(spiral->revo) > SP_EPSILON_2)) {
		gdouble arg = 2.0*M_PI*spiral->revo*spiral->t0 + spiral->arg;
		t0 = (round(arg/(0.25*M_PI))*0.25*M_PI
		      - spiral->arg)/(2.0*M_PI*spiral->revo);
	}
#endif
	spiral->t0 = CLAMP (spiral->t0, 0.0, 0.999);

#if 0
	/* outer point synchronize with inner point, if SHIFT is pressed */
	if (state & GDK_SHIFT_MASK) {
		spiral->revo += spiral->revo * (t0 - spiral->t0);
	}
	spiral->t0 = t0;
#endif
	sp_object_request_update ((SPObject *) spiral, SP_OBJECT_MODIFIED_FLAG);
}

/*
 * set attributes via outer (t=1) knot point:
 *   [default] increase/decrease revolution factor
 *   [control] constrain inner arg to round per PI/4
 */
static void
sp_spiral_outer_set (SPItem *item, const NRPointF *p, guint state)
{
	SPSpiral *spiral;
	gdouble   dx, dy;
/*  	gdouble arg; */
	
	spiral = SP_SPIRAL (item);

	dx = p->x - spiral->cx;
	dy = p->y - spiral->cy;
	spiral->arg = atan2(dy, dx) - 2.0*M_PI*spiral->revo;
	spiral->rad = MAX (hypot (dx, dy), 0.001);
#if 0
 /* we need round function */
/*  	arg  = -atan2(p->y, p->x) - spiral->arg; */
	if (state & GDK_CONTROL_MASK) {
		spiral->revo = (round(arg/(0.25*M_PI))*0.25*M_PI)/(2.0*M_PI);
	} else {
		spiral->revo = arg/(2.0*M_PI);
	}
	spiral->revo = arg/(2.0*M_PI);
#endif
	sp_object_request_update ((SPObject *) spiral, SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_spiral_inner_get (SPItem *item, NRPointF *p)
{
	SPSpiral *spiral;

	spiral = SP_SPIRAL (item);

	sp_spiral_get_xy (spiral, spiral->t0, p);
}

static void
sp_spiral_outer_get (SPItem *item, NRPointF *p)
{
	SPSpiral *spiral;

	spiral = SP_SPIRAL (item);

	sp_spiral_get_xy (spiral, 1.0, p);
}

static SPKnotHolder *
sp_spiral_knot_holder (SPItem * item, SPDesktop *desktop)
{
	SPSpiral *spiral;
	SPKnotHolder *knot_holder;

	spiral = SP_SPIRAL (item);
	knot_holder = sp_knot_holder_new (desktop, item, NULL);

	sp_knot_holder_add (knot_holder,
			    sp_spiral_inner_set,
			    sp_spiral_inner_get);
	sp_knot_holder_add (knot_holder,
			    sp_spiral_outer_set,
			    sp_spiral_outer_get);

	return knot_holder;
}

