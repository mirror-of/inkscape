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

/* minimal distance to norm before point is considered for snap */
#define MIN_DIST_NORM 1.0

#define SNAP_ON(d) (((d)->gridsnap > 0.0) || ((d)->guidesnap > 0.0))

const NR::Point horizontal(1, 0);
const NR::Point vertical(0, 1);

gdouble sp_desktop_dim_snap(SPDesktop const *dt, NR::Point& req, const int dim) {
	if(dim == NR::Y)
		return sp_desktop_vector_snap (dt, req, vertical);
	else if(dim == NR::X)
		return sp_desktop_vector_snap (dt, req, horizontal);
}

/* Snap a point in horizontal and vertical direction */

double
sp_desktop_free_snap (SPDesktop const *desktop, NR::Point& req)
{
	double dh, dv;
	NR::Point result = req;
	
	/* fixme: If allowing arbitrary snap targets, free snap is not the sum of h and v */
	dh = sp_desktop_dim_snap (desktop, result, NR::X);
	result.pt[NR::Y] = req.pt[NR::Y];
	dv = sp_desktop_dim_snap (desktop, result, NR::Y);
	req = result;
	
	if ((dh < 1e18) && (dv < 1e18)) return hypot (dh, dv);
	if (dh < 1e18) return dh;
	if (dv < 1e18) return dv;
	return 1e18;
}

/* Snap a point along a line n.X = d to another line X(t) = t*v + . */
double
sp_intersector_a_vector_snap (NR::Point& req, const NR::Point v, 
			const NR::Point n, double d)
/* This function returns the snap position and the distance from the
   starting point for doing a snap to arbitrary line. */
{
	NR::Point starting = req;
	NR::Point vperp = v.ccw();
	double d0 = dot(vperp, req);
	if(sp_intersector_line_intersection(vperp, d0, n, d, req) != intersects)
		return 1e18;
	return L2(req - starting);
}

/* Look for snappoint along a line given by req and the vector (dx,dy) */

double
sp_desktop_vector_snap (SPDesktop const * desktop, NR::Point& req, const NR::Point d)
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
		for (GSList * l = nv->vguides; l != NULL; l = l->next) {
			NR::Point trial = req;
			SPGuide* g = SP_GUIDE (l->data);
			gdouble dist = sp_intersector_a_vector_snap (trial, 
								 v, 
								 horizontal,
								 g->position);
			
			if (dist < upper) {
				upper = best = dist;
				actual = trial;
			}
		}
		for (GSList * l = nv->hguides; l != NULL; l = l->next) {
			NR::Point trial = req;
			SPGuide* g = SP_GUIDE (l->data);
			gdouble dist = sp_intersector_a_vector_snap (trial, 
								 v, 
								 vertical,
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
		{
			NR::Point trial = req;
			double iv = floor(((req.pt[NR::Y] - nv->gridorigin.pt[NR::Y]) / nv->gridspacingy)+0.5);

			double dist = sp_intersector_a_vector_snap (trial,
							     v,
							     vertical,
							     iv*nv->gridspacingy + nv->gridorigin.pt[NR::Y]);

			upper = MIN(best, desktop->gridsnap);
			if (dist < upper) {
				upper = best = dist;
				actual = trial;
			}
		}
		{
			double ih = floor(((req.pt[NR::X] - nv->gridorigin.pt[NR::X]) / 
					   nv->gridspacingx)+0.5);
			
			NR::Point trial = req;

			double dist = sp_intersector_a_vector_snap (trial,
							     v,
							     horizontal,
							     ih*nv->gridspacingx + nv->gridorigin.pt[NR::X]);

			if (dist < upper) {
				upper = best = dist;
				actual = trial;
			}
		}
	}
	req = actual;
	return best;
}

/* look for snappoint on a circle given by center (cx,cy) and distance center-req) */
// fixme: replace with line+circle intersector.

double
sp_desktop_circular_snap (SPDesktop const * desktop, NR::Point& req, const NR::Point center)
{
}


/* 
 * functions for lists of points
 *
 * All functions take a list of NR::Point and parameter indicating the proposed transformation.
 * They return the updated transformation parameter. 
 */

double
sp_desktop_dim_snap_list (SPDesktop const *desktop, NR::Point *p, const int length, const double dx, const int dim)
{
	gdouble dist = NR_HUGE;
	gdouble xdist = dx;

	if (SNAP_ON (desktop))
		for (int i = 0; i < length; i++) {
			NR::Point q = p[i];
			const gdouble pre = q.pt[dim];
			q.pt[dim] += dx;
			const gdouble d = sp_desktop_dim_snap (desktop, q, dim);
			if (d < dist) {
				xdist = q.pt[dim] - pre;
				dist = d;
			}
		}

	return xdist;
}

/*
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
		NR::Point q = p[i];
		NR::Point check = (q - norm) * s + norm;
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
*/

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

double
sp_desktop_dim_snap_list_scale (SPDesktop const *desktop, NR::Point *p, const int length, const NR::Point norm, const double sx, const int dim)
{
	if (!SNAP_ON (desktop))
		return sx;

	double dist = NR_HUGE;
	double scale = sx;

	for (int i = 0; i < length; i++) {
		NR::Point q = p[i];
		NR::Point check = q;
		check.pt[dim] = (sx * (q - norm) + norm).pt[dim];
		if (fabs (q.pt[dim] - norm.pt[dim]) > MIN_DIST_NORM) {
			const gdouble d = sp_desktop_dim_snap (desktop, check, dim);
			if ((d < 1e18) && (d < dist)) {
				dist = d;
				scale = (check.pt[dim] - norm.pt[dim]) / (q.pt[dim] - norm.pt[dim]);
			}
		}
	}

	return scale;
}

double
sp_desktop_dim_snap_list_skew (SPDesktop const *desktop, NR::Point *p, const int length, const NR::Point norm, const double sx, const int dim)
{
	if (!SNAP_ON (desktop))
		return sx;

	gdouble dist = NR_HUGE;
	gdouble skew = sx;
	
	for (int i = 0; i < length; i++) {
		NR::Point q = p[i];
		NR::Point check = q;
		// apply shear
		check.pt[dim] += sx * (q.pt[1- dim] - norm.pt[1- dim]);
		if (fabs (q.pt[1- dim] - norm.pt[1- dim]) > MIN_DIST_NORM) {
			const gdouble d = sp_desktop_dim_snap (desktop, check, dim);
			if ((d < 1e18) && (d < fabs (dist))) {
				dist = d;
				skew = (check.pt[dim] - q.pt[dim]) / (q.pt[1- dim] - norm.pt[1- dim]);
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
	double d = sp_desktop_vector_snap (dt, p, NR::Point(1.0, 0.0));
	*req = p;
	return d;
}

gdouble sp_desktop_vertical_snap(SPDesktop const *dt, NRPoint* req) {
	NR::Point p = *req;
	double d = sp_desktop_vector_snap (dt, p, NR::Point(0.0, 1.0));
	*req = p;
	return d;
}

double sp_desktop_dim_snap_list (SPDesktop const *desktop, NRPoint *p, const int length, const double dx, int dim) {
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


double sp_desktop_dim_snap_list_scale (SPDesktop const *desktop, NRPoint *p, const int length, const NRPoint* norm, const double sx, int dim) {
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

