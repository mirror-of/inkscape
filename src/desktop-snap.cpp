#define __SP_DESKTOP_SNAP_C__

/*
 * Various snapping methods
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/* TODO:
 * Sort out 1e18, 1e15 mess (handle large numbers gracefully
 * Circular snap, path snap?
 */

#include <math.h>
#include "sp-guide.h"
#include "sp-namedview.h"
#include "desktop-snap.h"
#include "geom.h"
#include <libnr/nr-point-fns.h>
#include <libnr/nr-values.h>

/* minimal distance to norm before point is considered for snap */
#define MIN_DIST_NORM 1.0

#define SNAP_ON(d) (((d)->gridsnap > 0.0) || ((d)->guidesnap > 0.0))

static gdouble sp_desktop_dim_snap(SPDesktop const *dt, NR::Point &req, unsigned const dim) {
	g_assert(dim < 2);
	return sp_desktop_vector_snap (dt, req, component_vectors[dim]);
}

/* Snap a point in horizontal and vertical direction */

double
sp_desktop_free_snap (SPDesktop const *desktop, NR::Point& req)
{
	double dh, dv;
	NR::Point result = req;
	
	/* fixme: If allowing arbitrary snap targets, free snap is not the sum of h and v */
	dh = sp_desktop_dim_snap (desktop, result, NR::X);
	result[NR::Y] = req[NR::Y];
	dv = sp_desktop_dim_snap (desktop, result, NR::Y);
	req = result;
	
	if ((dh < 1e18) && (dv < 1e18)) return hypot (dh, dv);
	if (dh < 1e18) return dh;
	if (dv < 1e18) return dv;
	return 1e18;
}

/** Add some multiple of \a mv to \a req to make it line on the line {p : dot(n, p) == d} (within
    rounding error); unless that isn't possible (e.g. \a mv and \a n are orthogonal, or \a mv or \a
    n is zero-length), in which case \a req remains unchanged, and a big number is returned.

    Returns a badness measure of snapping to the specified line: if snapping was possible then
    L2(req - req0) (i.e. the distance moved); otherwise returns a large number.
**/
static double sp_intersector_a_vector_snap(NR::Point &req, NR::Point const &mv,
					   NR::Point const &n, double const d) {
	NR::Point const req0(req);
	/* Implement "move from req0 by some multiple of mv" as "dot product with something
	   orthogonal to mv remains unchanged". */
	NR::Point const n2(rot90(mv));
	double const d2 = dot(n2, req);
	if(sp_intersector_line_intersection(n2, d2, n, d, req) == intersects) {
		return L2(req - req0);
	} else {
		return 1e300;
	}
}

static double round_to_nearest_multiple_plus(double x, double const c1, double const c0);

/**
 * Look for snappoint along a line given by \a req and the vector \a d.
 *
 * Requires: d != (0, 0).
 */
double
sp_desktop_vector_snap (SPDesktop const *desktop, NR::Point &req, NR::Point const &d)
{
	double best = 1e18, upper = 1e18;

	g_assert (desktop != NULL);
	g_assert (SP_IS_DESKTOP (desktop));

	double len = L2(d);
	if (len < 1e-18)
		return sp_desktop_free_snap (desktop, req);
	NR::Point v = d;
	v.normalize();

	SPNamedView * nv = desktop->namedview;
	NR::Point actual = req;
	
	if (nv->snaptoguides) {
		upper = desktop->guidesnap;
		for (GSList * l = nv->guides; l != NULL; l = l->next) {
			NR::Point trial(req);
			SPGuide const *g = SP_GUIDE (l->data);
			gdouble dist = sp_intersector_a_vector_snap (trial, 
								 v, 
								 g->normal,
								 g->position);
			
			if (dist < upper) {
				upper = best = dist;
				actual = trial;
			}
		}
	}

	if (nv->snaptogrid) {
/*  find nearest grid line (either H or V whatever is closer) along
 *  the vector to the requested point.  If the distance along the
 *  vector is less than the snap distance then snap. */
		upper = MIN(best, desktop->gridsnap);
		for(unsigned i = 0; i < 2; ++i) {
			NR::Point trial(req);
			double const rounded
				= round_to_nearest_multiple_plus(req[i],
								 nv->gridspacing[i],
								 nv->gridorigin[i]);

			double const dist = sp_intersector_a_vector_snap (trial,
									  v,
									  component_vectors[i],
									  rounded);

			if (dist < upper) {
				upper = best = dist;
				actual = trial;
			}
		}
	}
	req = actual;
	return best;
}

/**
 * If c1==0 (and c0 is finite), then returns +/-inf.  This makes grid spacing of zero
 * mean "ignore the grid in this dimention".  We're currently discussing "good" semantics
 * for guide/grid snapping.
 */
static double
round_to_nearest_multiple_plus(double x, double const c1, double const c0)
{
	return floor((x - c0) / c1 + .5) * c1 + c0;
}

/* look for snappoint on a circle given by center (cx,cy) and distance center-req) */
// fixme: replace with line+circle intersector.

#if 0
double
sp_desktop_circular_snap (SPDesktop const * desktop, NR::Point& req, const NR::Point center)
{
}
#endif


/* 
 * functions for lists of points
 *
 * All functions take a list of NR::Point and parameter indicating the proposed transformation.
 * They return the updated transformation parameter. 
 */

static double
sp_desktop_dim_snap_list (SPDesktop const *desktop, NR::Point *p, int const length, double const dx, int const dim)
{
	gdouble dist = NR_HUGE;
	gdouble xdist = dx;

	if (SNAP_ON (desktop))
		for (int i = 0; i < length; i++) {
			NR::Point q = p[i];
			const gdouble pre = q[dim];
			q[dim] += dx;
			const gdouble d = sp_desktop_dim_snap (desktop, q, dim);
			if (d < dist) {
				xdist = q[dim] - pre;
				dist = d;
			}
		}

	return xdist;
}

#if 0
static inline NR::Point
map_mul(NR::Point const &a, NR::Point const &b)
{
	using NR::X;
	using NR::Y;
	return NR::Point(a[X] * b[X],
			 a[Y] * b[Y]);
}

double
sp_desktop_vector_snap_list (SPDesktop const *desktop, NR::Point *p, int length,const NR::Point norm, NR::Point s)
{
	NR::Point check;
	double dist, d, ratio;
	int i;

	if (!SNAP_ON (desktop)) return L2(s);

	dist = NR_HUGE;
	ratio = L2(s);

	for (i = 0; i < length; i++) {
		NR::Point p_from_norm = p[i] - norm;
		NR::Point check = map_mul(p_from_norm, s) + norm;
		if ( ( fabs p_from_norm[X] > MIN_DIST_NORM )  ||
		     ( fabs p_from_norm[Y] > MIN_DIST_NORM ) ) {
			d = sp_desktop_vector_snap (desktop, &check, check.x - norm->x, check.y - norm->y);
			if ((d < 1e18) && (d < dist)) {
				dist = d;
				unsigned dominant = ( ( fabs(p_from_norm[X]) > fabs(p_from_norm[Y]) )
						      ? X
						      : Y );
				ratio = ( ( check[dominant] - norm[dominant] )
					  / p_from_norm[dominant] );
			}
		}
	}
  
	return ratio;
}
#endif /* to be deleted */

double
sp_desktop_vector_snap_list (SPDesktop const *desktop, NRPoint *p, int length, NRPoint const *norm, double sx, double sy)
{
	NRPoint q, check;
	double dist, d, ratio;
	int i;

	if (!SNAP_ON (desktop)) return sx;

	dist = NR_HUGE;
	ratio = fabs (sx);

	for (i = 0; i < length; i++) {
		q = p[i];
		check.x = (q.x - norm->x) * sx + norm->x;
		check.y = (q.y - norm->y) * sy + norm->y;
		if ((fabs (q.y - norm->y) > MIN_DIST_NORM) || (fabs (q.y - norm->y) > MIN_DIST_NORM)) {
			d = sp_desktop_vector_snap (desktop, &check, check.x - norm->x, check.y - norm->y);
			if ((d < 1e18) && (d < dist)) {
				dist = d;
				ratio = (fabs(q.x - norm->x) > fabs(q.y - norm->y)) ? 
					(check.x - norm->x) / (q.x - norm->x) : 
					(check.y - norm->y) / (q.y - norm->y); 
			}
		}
	}
  
	return ratio;
}

static double
sp_desktop_dim_snap_list_scale (SPDesktop const *desktop, NR::Point *p, int const length, NR::Point const &norm, double const sx, int const dim)
{
	if (!SNAP_ON (desktop))
		return sx;

	double dist = NR_HUGE;
	double scale = sx;

	for (int i = 0; i < length; i++) {
		NR::Point q = p[i];
		NR::Point check = q;
		check[dim] = (sx * (q - norm) + norm)[dim];
		if (fabs (q[dim] - norm[dim]) > MIN_DIST_NORM) {
			const gdouble d = sp_desktop_dim_snap (desktop, check, dim);
			if ((d < 1e18) && (d < dist)) {
				dist = d;
				scale = (check[dim] - norm[dim]) / (q[dim] - norm[dim]);
			}
		}
	}

	return scale;
}

static double
sp_desktop_dim_snap_list_skew (SPDesktop const *desktop, NR::Point *p, int const length, NR::Point const &norm, double const sx, int const dim)
{
	if (!SNAP_ON (desktop))
		return sx;

	gdouble dist = NR_HUGE;
	gdouble skew = sx;
	
	for (int i = 0; i < length; i++) {
		NR::Point q = p[i];
		NR::Point check = q;
		// apply shear
		check[dim] += sx * (q[1- dim] - norm[1- dim]);
		if (fabs (q[1- dim] - norm[1- dim]) > MIN_DIST_NORM) {
			const gdouble d = sp_desktop_dim_snap (desktop, check, dim);
			if ((d < 1e18) && (d < fabs (dist))) {
				dist = d;
				skew = (check[dim] - q[dim]) / (q[1- dim] - norm[1- dim]);
			}
		}
	}

	return skew;
}












/********************
 * Obsolete methods *
 ********************/

/* Single point methods */
double sp_desktop_free_snap (SPDesktop const *desktop, NRPoint *req) {
	NR::Point r = *req;
	NR::Coord d = sp_desktop_free_snap(desktop, r);
	*req = r;
	return d;
}

double sp_desktop_vector_snap (SPDesktop const *desktop, NRPoint *req, double dx, double dy) {
	NR::Point r = *req;
	NR::Point d(dx, dy);
	NR::Coord result = sp_desktop_vector_snap(desktop, r, d);
	*req = r;
	return result;
}


gdouble sp_desktop_horizontal_snap(SPDesktop const *dt, NRPoint* req) {
	NR::Point p = *req;
	double d = sp_desktop_dim_snap (dt, p, NR::X);
	*req = p;
	return d;
}

gdouble sp_desktop_vertical_snap(SPDesktop const *dt, NRPoint* req) {
	NR::Point p = *req;
	double d = sp_desktop_dim_snap (dt, p, NR::Y);
	*req = p;
	return d;
}

static double sp_desktop_dim_snap_list (SPDesktop const *desktop, NRPoint *p, const int length, const double dx, int dim) {
	NR::Point* l = new NR::Point[length];
	for(int i = 0; i < length; i++)
		l[i] = (NR::Point)p[i];
	double r = sp_desktop_dim_snap_list (desktop, l, length, dx, dim);
	for(int i = 0; i < length; i++)
		p[i] = l[i];
	return r;
}

double sp_desktop_horizontal_snap_list (SPDesktop const *desktop, NRPoint *p, const int length, const double dx) {
	return sp_desktop_dim_snap_list(desktop, p, length, dx, NR::X);
}


double sp_desktop_vertical_snap_list (SPDesktop const *desktop, NRPoint *p, const int length, const double dx) {
	return sp_desktop_dim_snap_list(desktop, p, length, dx, NR::Y);
}


static double sp_desktop_dim_snap_list_scale (SPDesktop const *desktop, NRPoint *p, const int length, NRPoint const * norm, const double sx, int dim) {
	NR::Point* l = new NR::Point[length];
	for(int i = 0; i < length; i++)
		l[i] = (NR::Point)p[i];
	NR::Point n = *norm;
	double r = sp_desktop_dim_snap_list_scale (desktop, l, length, n, sx, dim);
	for(int i = 0; i < length; i++)
		p[i] = l[i];
	return r;
}

double sp_desktop_horizontal_snap_list_scale (SPDesktop const *desktop, NRPoint *p, const int length, const NRPoint* norm, const double sx) {
	return sp_desktop_dim_snap_list_scale(desktop, p, length, norm, sx, NR::X);
}

double sp_desktop_vertical_snap_list_scale (SPDesktop const *desktop, NRPoint *p, const int length, const NRPoint* norm, const double sx) {
	return sp_desktop_dim_snap_list_scale(desktop, p, length, norm, sx, NR::Y);
}


double sp_desktop_dim_snap_list_skew (SPDesktop const *desktop, NRPoint *p, const int length, const NRPoint* norm, const double sx, int dim) {
	NR::Point* l = new NR::Point[length];
	for(int i = 0; i < length; i++)
		l[i] = (NR::Point)p[i];
	NR::Point n = *norm;
	double r = sp_desktop_dim_snap_list_skew (desktop, l, length, n, sx, dim);
	for(int i = 0; i < length; i++)
		p[i] = l[i];
	return r;
}

double sp_desktop_horizontal_snap_list_skew (SPDesktop const *desktop, NRPoint *p, const int length, const NRPoint* norm, const double sx) {
	return sp_desktop_dim_snap_list_skew(desktop, p, length, norm, sx, NR::X);
}

double sp_desktop_vertical_snap_list_skew (SPDesktop const *desktop, NRPoint *p, const int length, const NRPoint* norm, const double sx) {
	return sp_desktop_dim_snap_list_skew(desktop, p, length, norm, sx, NR::Y);	
}

