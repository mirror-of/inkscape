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

#include <math.h>
#include "sp-guide.h"
#include "sp-namedview.h"
#include "desktop-snap.h"

/* minimal distance to norm before point is considered for snap */
#define MIN_DIST_NORM 1.0

#define SNAP_ON(d) (((d)->gridsnap > 0.0) || ((d)->guidesnap > 0.0))

/* Snap a point in horizontal and vertical direction */

double
sp_desktop_free_snap (SPDesktop *desktop, NRPointF *req)
{
	double dh, dv;
	NRPointF result = *req;

	/* fixme: If allowing arbitrary snap targtes, free snap is not the sum of h and v */
	dh = sp_desktop_vector_snap (desktop, &result, 1.0, 0.0);
	result.y = req->y;
	dv = sp_desktop_vector_snap (desktop, &result, 0.0, 1.0);
	*req = result;
	
	if ((dh < 1e18) && (dv < 1e18)) return hypot (dh, dv);
	if (dh < 1e18) return dh;
	if (dv < 1e18) return dv;
	return 1e18;
}

/* Intersect two lines */

typedef enum sp_intersector_kind{
	intersects = 0,
	parallel,
	coincident,
	no_intersection
} sp_intersector_kind;

sp_intersector_kind
sp_intersector_line_intersection(NRPointF *n0, double d0, NRPointF *n1, double d1, NRPointF* result)
/* This function finds the intersection of the two lines (infinite)
 * defined by n0.X = d0 and x1.X = d1.  The algorithm is as follows:
 * To compute the intersection point use kramer's rule:
 * 
 * convert lines to form
 * ax + by = c
 * dx + ey = f
 * 
 * (
 *  e.g. a = (x2 - x1), b = (y2 - y1), c = (x2 - x1)*x1 + (y2 - y1)*y1
 * )
 * 
 * In our case we use:
 *   a = n0->x     d = n1->x
 *   b = n0->y     e = n1->y
 *   c = d0        f = d1
 * 
 * so:
 * 
 * adx + bdy = cd
 * adx + aey = af
 * 
 * bdy - aey = cd - af
 * (bd - ae)y = cd - af
 * 
 * y = (cd - af)/(bd - ae)
 * 
 * repeat for x and you get:
 * 
 * x = (fb - ce)/(bd - ae)
 * 
 * if the denominator (bd-ae) is 0 then the lines are parallel, if the
 * numerators are then 0 then the lines coincide. */
{
	double denominator;
	double X, Y;

	g_assert(n0);
	g_assert(n1);
	denominator = n0->y*n1->x - n0->x*n1->y;
	if(denominator == 0) {
		/*printf("Parallel!  So (%f, %f) should equal k(%f %f)\n", 
		       n0->x, n0->y,
		       n1->x, n1->y);*/
		if(d1*n0->y - d0*n1->y == 0)
			return coincident;
		return parallel;
	}
	denominator = 1.0/denominator;
	X = d1*n0->y - d0*n1->y;
	Y = d0*n1->x - d1*n0->x;
	g_assert(result != 0);
	result->x = X*denominator;
	result->y = Y*denominator;
	return intersects;
}

/* ccw exists as a building block */
static int
sp_intersector_ccw(NRPointF p0, NRPointF p1, NRPointF p2)
/* Determine which way a set of three points winds. */
{
	int dx1, dx2, dy1, dy2;
	dx1 = p1.x-p0.x; dy1 = p1.y - p0.y;
	dx2 = p2.x-p0.x; dy2 = p2.y - p0.y;
/* compare slopes but avoid division operation */
	if(dx1*dy2 > dy1*dx2)
		return +1; // ccw
	if(dx1*dy2 < dy1*dx2)
		return -1; // cw
	if(dx1*dx2 < 0 || dy1*dy2 < 0)
		return -1; // p2 p0 p1 collinear
	if(dx1*dx1 + dy1*dy1 < dx2*dx2 + dy2*dy2)
		return +1;// p0 p1 p2 collinear
	return 0; // p0 p2 p1 collinear
}


int
sp_intersector_segment_intersectp(NRPointF p00, NRPointF p01, NRPointF p10, NRPointF p11)
/* Determine whether two line segments intersect.  This doesn't find
   the intersection, use the line_intersect funcction above */
{
	return ((sp_intersector_ccw(p00,p01, p10)
		 *sp_intersector_ccw(p00, p01, p11)) <=0 )
		&&
		((sp_intersector_ccw(p10,p11, p00)
		  *sp_intersector_ccw(p10, p11, p01)) <=0 );
}

sp_intersector_kind
sp_intersector_segment_intersect(NRPointF p00, NRPointF p01, NRPointF p10, NRPointF p11, NRPointF* result)
/* Determine whether two line segments intersect.  This doesn't find
   the intersection, use the line_intersect funcction above */
{
	if(sp_intersector_segment_intersectp(p00, p01, p10, p11)) {
		double d0, d1;

		NRPointF n0, n1;
		n0.x = p00.y - p01.y;
		n0.y = -(p00.x - p01.x);
		d0 = n0.x*p00.x + n0.y*p00.y;
		n1.x = p10.y - p11.y;
		n1.y = -(p10.x - p11.x);
		d1 = n1.x*p10.x + n1.y*p10.y;
		return sp_intersector_line_intersection(&n0, d0, &n1, d1, result);
	} else {
		return no_intersection;
	}
}


/* Snap a point along a line to another line. */
double
sp_intersector_a_vector_snap (NRPointF * req, NRPointF * v, 
			NRPointF * n, double d)
/* This function returns the snap position and the distance from the
   starting point for doing a snap to arbitrary line. */
{
	NRPointF starting;
	NRPointF vperp; /* Perpendicular vector to v */
	double d0;

	vperp.x = v->y;
	vperp.y = -v->x;
	starting.x = req->x;
	starting.y = req->y;
	d0 = vperp.x*req->x + vperp.y*req->y;
	if(sp_intersector_line_intersection(&vperp, d0, n, d, req) != intersects)
		return 1e18;
	return hypot(req->x - starting.x, req->y - starting.y);
}

/* Look for snappoint along a line given by req and the vector (dx,dy) */

double
sp_desktop_vector_snap (SPDesktop * desktop, NRPointF *req, double dx, double dy)
{
	SPNamedView * nv;
	NRPointF actual;
	double len, best = 1e18, upper = 1e18;
	GSList * l;
	NRPointF v;
	NRPointF horizontal;
	NRPointF vertical;

	horizontal.x = 1; horizontal.y = 0;
	vertical.x = 0; vertical.y = 1;

	g_assert (desktop != NULL);
	g_assert (SP_IS_DESKTOP (desktop));
	g_assert (req != NULL);

	len = hypot (dx, dy);
	if (len < 1e-18) return sp_desktop_free_snap (desktop, req);
	v.x = dx/len; v.y = dy/len;

	nv = desktop->namedview;
	actual = *req;

	if (nv->snaptoguides) {
		upper = desktop->guidesnap;
		for (l = nv->vguides; l != NULL; l = l->next) {
			NRPointF trial = *req;
			gdouble dist = sp_intersector_a_vector_snap (&trial, 
								 &v, 
								 &horizontal,
								 SP_GUIDE (l->data)->position);
			
			if (dist < upper) {
				upper = best = dist;
				actual = trial;
			}
		}
		for (l = nv->hguides; l != NULL; l = l->next) {
			NRPointF trial = *req;
			gdouble dist = sp_intersector_a_vector_snap (&trial, 
								 &v, 
								 &vertical,
								 SP_GUIDE (l->data)->position);
			
			if (dist < upper) {
				upper = best = dist;
				actual = trial;
			}
		}
	}

	if (nv->snaptogrid) {
		double iv, ih, dist, upper;
		NRPointF trial = *req;
/*  find nearest grid line (either H or V whatever is closer) along
 *  the vector to the requested point.  If the distance along the
 *  vector is less than the snap distance then snap. */
		iv = floor(((req->y - nv->gridoriginy) / nv->gridspacingy)+0.5);
		dist = sp_intersector_a_vector_snap (&trial,
							 &v,
							 &vertical,
							 iv*nv->gridspacingy + nv->gridoriginy);
		upper = MIN(best, desktop->gridsnap);
		if (dist < upper) {
			upper = best = dist;
			actual = trial;
		}
		ih = floor(((req->x - nv->gridoriginx) / 
			nv->gridspacingx)+0.5);
		
		trial = *req;
		dist = sp_intersector_a_vector_snap (&trial,
						 &v,
						 &horizontal,
						 ih*nv->gridspacingx + nv->gridoriginx);
		if (dist < upper) {
			upper = best = dist;
			actual = trial;
		}
	}
	* req = actual;
	return best;
}

/* look for snappoint on a circle given by center (cx,cy) and distance center-req) */

double
sp_desktop_circular_snap (SPDesktop * desktop, NRPointF * req, double cx, double cy)
{
	SPNamedView * nv;
	NRPointF actual;
	gdouble best = 1e18, dist, h, dx, dy;
	gboolean snapped;
	GSList * l;

	g_assert (desktop != NULL);
	g_assert (SP_IS_DESKTOP (desktop));
	g_assert (req != NULL);

	nv = desktop->namedview;
	actual = *req;
	snapped = FALSE;

	h = (req->x - cx)*(req->x - cx) + (req->y - cy)*(req->y - cy); // h is sqare of hypotenuse
	if (h < 1e-15) return 1e18;

	if (nv->snaptoguides) {
		/* snap distance in desktop units */
		best = desktop->guidesnap;
		best *= best; // best is sqare of best distance 
		// vertical guides
		for (l = nv->vguides; l != NULL; l = l->next) {
			dx = fabs(SP_GUIDE (l->data)->position - cx);
			if (dx * dx <= h) {
				dy = sqrt (h - dx * dx);
				// down
				dist = (req->x - SP_GUIDE (l->data)->position) * (req->x - SP_GUIDE (l->data)->position) + 
					(req->y - (cy - dy)) * (req->y - (cy - dy));
				if (dist < best) {
					best = dist;
					actual.x = SP_GUIDE (l->data)->position;
					actual.y = cy - dy;
					snapped = TRUE;
				}
				// up
				dist = (req->x - SP_GUIDE (l->data)->position) * (req->x - SP_GUIDE (l->data)->position) + 
					(req->y - (cy + dy)) * (req->y - (cy + dy));
				if (dist < best) {
					best = dist;
					actual.x = SP_GUIDE (l->data)->position;
					actual.y = cy + dy;
					snapped = TRUE;
				}
		    
			}
		} // vertical guides
		// horizontal guides
		for (l = nv->hguides; l != NULL; l = l->next) {
			dy = fabs(SP_GUIDE (l->data)->position - cy);
			if (dy * dy <= h) {
				dx = sqrt (h - dy * dy);
				// down
				dist = (req->y - SP_GUIDE (l->data)->position) * (req->y - SP_GUIDE (l->data)->position) + 
					(req->x - (cx - dx)) * (req->x - (cx - dx));
				if (dist < best) {
					best = dist;
					actual.y = SP_GUIDE (l->data)->position;
					actual.x = cx - dx;
					snapped = TRUE;
				}
				// up
				dist = (req->y - SP_GUIDE (l->data)->position) * (req->y - SP_GUIDE (l->data)->position) + 
					(req->x - (cx + dx)) * (req->x - (cx + dx));
				if (dist < best) {
					best = dist;
					actual.y = SP_GUIDE (l->data)->position;
					actual.x = cx + dx;
					snapped = TRUE;
				}
		    
			}
		} // horizontal guides
	} // snaptoguides

	if (nv->snaptogrid) {
	        gdouble p1, p2;
		/* snap distance in desktop units */
		if (best == 1e18) {
			best = desktop->gridsnap;
			best *= best; // best is square of best distance 
		}
		// horizontal gridlines
       		p1 = nv->gridoriginx + floor ((req->x - nv->gridoriginx) / nv->gridspacingx) * nv->gridspacingx;
		p2 = p1 + nv->gridspacingx;
		// lower gridline
		dx = fabs(p1 - cx);
		if (dx * dx <= h) {
			dy = sqrt (h - dx * dx);
			// down
			dist = (req->x - p1) * (req->x - p1) + (req->y - (cy - dy)) * (req->y - (cy - dy));
			if (dist < best) {
				best = dist;
				actual.x = p1;
				actual.y = cy - dy;
				snapped = TRUE;
			}
			// up
			dist = (req->x - p1) * (req->x - p1) + (req->y - (cy + dy)) * (req->y - (cy + dy));
			if (dist < best) {
				best = dist;
				actual.x = p1;
				actual.y = cy + dy;
				snapped = TRUE;
			}
		}
		// upper gridline
		dx = fabs(p2 - cx);
		if (dx * dx <= h) {
			dy = sqrt (h - dx * dx);
			// down
			dist = (req->x - p2) * (req->x - p2) + (req->y - (cy - dy)) * (req->y - (cy - dy));
			if (dist < best) {
				best = dist;
				actual.x = p2;
				actual.y = cy - dy;
				snapped = TRUE;
			}
			// up
			dist = (req->x - p2) * (req->x - p2) + (req->y - (cy + dy)) * (req->y - (cy + dy));
			if (dist < best) {
				best = dist;
				actual.x = p2;
				actual.y = cy + dy;
				snapped = TRUE;
			}
		}
		
		// vertical gridline
		p1 = nv->gridoriginy + floor ((req->y - nv->gridoriginy) / nv->gridspacingy) * nv->gridspacingy;
		p2 = p1 + nv->gridspacingy;
		//lower gridline
		dy = fabs(p1 - cy);
		if (dy * dy <= h) {
			dx = sqrt (h - dy * dy);
			// down
			dist = (req->y - p1) * (req->y - p1) + 
				(req->x - (cx - dx)) * (req->x - (cx - dx));
			if (dist < best) {
				best = dist;
				actual.y = p1;
				actual.x = cx - dx;
				snapped = TRUE;
			}
		  // up
		  dist = (req->y - p1) * (req->y - p1) + 
		         (req->x - (cx + dx)) * (req->x - (cx + dx));
		  if (dist < best) {
		    best = dist;
		    actual.y = p1;
		    actual.x = cx + dx;
		    snapped = TRUE;
		  }
		}
		//lower gridline
		dy = fabs(p2 - cy);
		if (dy * dy <= h) {
		  dx = sqrt (h - dy * dy);
		  // down
		  dist = (req->y - p2) * (req->y - p2) + 
		         (req->x - (cx - dx)) * (req->x - (cx - dx));
		  if (dist < best) {
		    best = dist;
		    actual.y = p2;
		    actual.x = cx - dx;
		    snapped = TRUE;
		  }
		  // up
		  dist = (req->y - p2) * (req->y - p2) + 
		         (req->x - (cx + dx)) * (req->x - (cx + dx));
		  if (dist < best) {
		    best = dist;
		    actual.y = p2;
		    actual.x = cx + dx;
		    snapped = TRUE;
		  }
		}
	} // snaptogrid
	
	dist = (snapped) ? best : 1e18;

	*req = actual;

	return dist;
}


/* 
 * functions for lists of points
 *
 * All functions take a list of NRPointF and parameter indicating the proposed transformation.
 * They return the updated transformation parameter. 
 */

double
sp_desktop_horizontal_snap_list (SPDesktop *desktop, NRPointF *p, int length, double dx)
{
	NRPointF q;
	double xdist, xpre, dist, d;
	int i;

	dist = NR_HUGE_F;
	xdist = dx;

	if (SNAP_ON (desktop))
		for (i = 0; i < length; i++) {
			q = p[i];
			xpre = q.x;
			q.x += dx;
			d = sp_desktop_vector_snap (desktop, &q, 1, 0);
			if (d < dist) {
				xdist = q.x - xpre;
				dist = d;
			}
		}

	return xdist;
}

double
sp_desktop_vertical_snap_list (SPDesktop *desktop, NRPointF *p, int length, double dy)
{
	NRPointF q;
	double ydist, ypre, dist, d;
	int i;

	dist = NR_HUGE_F;
	ydist = dy;
	if(SNAP_ON (desktop))
		for (i = 0; i < length; i++) {
			q = p[i];
			ypre = q.y;
			q.y += dy;
			d = sp_desktop_vector_snap (desktop, &q, 0, 1);
			if (d < dist) {
				ydist = q.y - ypre;
				dist = d;
			}
		}

	return ydist;
}

double
sp_desktop_horizontal_snap_list_scale (SPDesktop *desktop, NRPointF *p, int length, NRPointF *norm, double sx)
{
	NRPointF q, check;
	double xscale, xdist, d;
	int i;

	if (!SNAP_ON (desktop)) return sx;

	xdist = NR_HUGE_F;
	xscale = sx;

	for (i = 0; i < length; i++) {
		q = p[i];
		check.x = sx * (q.x - norm->x) + norm->x;
		check.y = q.y;
		if (fabs (q.x - norm->x) > MIN_DIST_NORM) {
			d = sp_desktop_horizontal_snap (desktop, &check);
			if ((d < 1e18) && (d < xdist)) {
				xdist = d;
				xscale = (check.x - norm->x) / (q.x - norm->x);
			}
		}
	}

	return xscale;
}

double
sp_desktop_vertical_snap_list_scale (SPDesktop *desktop, NRPointF *p, int length, NRPointF *norm, double sy)
{
	NRPointF q, check;
	double yscale, ydist, d;
	int i;

	if (!SNAP_ON (desktop)) return sy;

	ydist = NR_HUGE_F;
	yscale = sy;

	for (i = 0; i < length; i++) {
		q = p[i];
		check.y = sy * (q.y - norm->y) + norm->y;
		if (fabs (q.y - norm->y) > MIN_DIST_NORM) {
			d = sp_desktop_vertical_snap (desktop, &check);
			if ((d < 1e18) && (d < fabs (ydist))) {
				ydist = d;
				yscale = (check.y - norm->y)/(q.y - norm->y);
			}
		}
	}

	return yscale;
}

double
sp_desktop_vector_snap_list (SPDesktop *desktop, NRPointF *p, int length, NRPointF *norm, double sx, double sy)
{
	NRPointF q, check;
	double dist, d, ratio;
	int i;

	if (!SNAP_ON (desktop)) return sx;

	dist = NR_HUGE_F;
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
sp_desktop_horizontal_snap_list_skew (SPDesktop *desktop, NRPointF *p, int length, NRPointF *norm, double sx)
{
	NRPointF q, check;
	double xskew, xdist, d;
	int i;

	if (!SNAP_ON (desktop)) return sx;

	xdist = NR_HUGE_F;
	xskew = sx;

	for (i = 0; i < length; i++) {
		q = p[i];
		check.x = sx * (q.y - norm->y) + q.x;
		if (fabs (q.y - norm->y) > MIN_DIST_NORM) {
			d = sp_desktop_horizontal_snap (desktop, &check);
			if ((d < 1e18) && (d < fabs (xdist))) {
				xdist = d;
				xskew = (check.x - q.x) / (q.y - norm->y);
			}
		}
	}

	return xskew;
}

double
sp_desktop_vertical_snap_list_skew (SPDesktop *desktop, NRPointF *p, int length, NRPointF *norm, double sy)
{
	NRPointF q, check;
	gdouble yskew, ydist, d;
	int i;

	if (!SNAP_ON (desktop)) return sy;

	ydist = NR_HUGE_F;
	yskew = sy;

	for (i = 0; i < length; i++) {
		q = p[i];
		check.y = sy * (q.x - norm->x) + q.y;
		if (fabs (q.x - norm->x) > MIN_DIST_NORM) {
			d = sp_desktop_vertical_snap (desktop, &check);
			if ((d < 1e18) && (d < fabs (ydist))) {
				ydist = d;
				yskew = (check.y - q.y)/(q.x - norm->x);
			}
		}
	}

	return yskew;
}

/* 
   this function takes the whole transformation matrix as parameter
   working with angles would be too complex
*/
NRMatrixF *
sp_desktop_circular_snap_list (SPDesktop *desktop, NRPointF *p, int length, NRPointF *norm, NRMatrixF *rotate)
{
	NRPointF q1, q2, q, check;
	gdouble d, best, h1, h2;
	int i;

	if (!SNAP_ON (desktop)) return rotate;

	best = NR_HUGE_F;

	for (i = 0; i < length; i++) {
		q = p[i];
		check.x = NR_MATRIX_DF_TRANSFORM_X (rotate, q.x, q.y);
		check.y = NR_MATRIX_DF_TRANSFORM_Y (rotate, q.x, q.y);
		d = sp_desktop_circular_snap (desktop, &check, norm->x, norm->y);
		if (d < best) {
			q1 = q;
			q2 = check;
			best = d;
		}
	}

	// compute the new transformation (rotation) from the snapped point
	if (best < 1e18) {
		NRMatrixF r1, r2, p2n, n2p;

		h1 = hypot (q1.x - norm->x, q1.y - norm->y);
		q1.x = (q1.x - norm->x) / h1;
		q1.y = (q1.y - norm->y) / h1;
		h2 = hypot (q2.x - norm->x, q2.y - norm->y);
		q2.x = (q2.x - norm->x) / h2;
		q2.y = (q2.y - norm->y) / h2;
		r1.c[0] = q1.x;  r1.c[1] = -q1.y;  r1.c[2] =  q1.y;  r1.c[3] = q1.x;  r1.c[4] = 0;  r1.c[5] = 0;
		r2.c[0] = q2.x;  r2.c[1] =  q2.y;  r2.c[2] = -q2.y;  r2.c[3] = q2.x;  r2.c[4] = 0;  r2.c[5] = 0;

		nr_matrix_f_set_translate (&n2p, norm->x, norm->y);
		nr_matrix_f_invert (&p2n, &n2p);
		nr_matrix_multiply_fff (rotate, &p2n, &r1);
		nr_matrix_multiply_fff (rotate, rotate, &r2);
		nr_matrix_multiply_fff (rotate, rotate, &n2p);
	}

	return rotate;
}
