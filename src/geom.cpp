#include "geom.h"
#include <math.h>

NR::Coord L1(const NR::Point p) {
	NR::Coord d = 0;
	for(int i = 0; i < 2; i++)
		d += fabs(p.pt[i]);
	return d;
}

NR::Coord L2(const NR::Point p) {
	return hypot(p.pt[0], p.pt[1]);
}

NR::Coord Linfty(const NR::Point p) {
	NR::Coord d = 0;
	for(int i = 0; i < 2; i++)
		d = MAX(d, fabs(p.pt[i]));
	return d;
}


/* Intersect two lines */

sp_intersector_kind
sp_intersector_line_intersection(const NR::Point n0, const double d0, const NR::Point n1, const double d1, NR::Point& result)
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
 *   a = n0.x     d = n1.x
 *   b = n0.y     e = n1.y
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

	denominator = cross(n0,n1);
	if(denominator == 0) {
		/*printf("Parallel!  So (%f, %f) should equal k(%f %f)\n", 
		       n0->x, n0.pt[NR::Y],
		       n1->x, n1.pt[NR::Y]);*/
		if(d1*n0.pt[NR::Y] - d0*n1.pt[NR::Y] == 0)
			return coincident;
		return parallel;
	}
	denominator = 1.0/denominator;
	X = (d0*n1.pt[NR::Y] - d1*n0.pt[NR::Y]); // replace with cross?
	Y = -(d0*n1.pt[NR::X] - d1*n0.pt[NR::X]);
	result = denominator*NR::Point(X, Y);
	return intersects;
}

/* ccw exists as a building block */
static int
sp_intersector_ccw(const NR::Point p0, const NR::Point p1, const NR::Point p2)
/* Determine which way a set of three points winds. */
{
	NR::Point d1 = p1 - p0;
	NR::Point d2 = p2 - p0;
/* compare slopes but avoid division operation */
	double c = cross(d1, d2);
	if(c > 0)
		return +1; // ccw - do these match def'n in header?
	if(c < 0)
		return -1; // cw
	if(d1.pt[0]*d2.pt[0] < 0 || d1.pt[1]*d2.pt[1] < 0)
		return -1; // p2 p0 p1 colinear
	if(dot(d1,d1) < dot(d2,d2))
		return +1;// p0 p1 p2 colinear
	return 0; // p0 p2 p1 collinear
}


int
sp_intersector_segment_intersectp(const NR::Point p00, const NR::Point p01, const NR::Point p10, const NR::Point p11)
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
sp_intersector_segment_intersect(const NR::Point p00, const NR::Point p01, const NR::Point p10, const NR::Point p11, NR::Point& result)
/* Determine whether two line segments intersect.  This doesn't find
   the intersection, use the line_intersect funcction above */
{
	if(sp_intersector_segment_intersectp(p00, p01, p10, p11)) {
		double d0, d1;

		NR::Point n0 = (p00 - p01).ccw();
		d0 = dot(n0,p00); // cross?

		NR::Point n1 = (p10 - p11).ccw();
		d1 = dot(n1,p10);
		return sp_intersector_line_intersection(n0, d0, n1, d1, result);
	} else {
		return no_intersection;
	}
}

