#ifndef __NR_TYPES_H__
#define __NR_TYPES_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Class-ifying NRPoint, Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * This code is in public domain
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

namespace NR {

typedef double Coord;
typedef gint32 ICoord;

} /* namespace NR */

struct NRMatrix {
	NR::Coord c[6];
};

struct NRPoint {
	NR::Coord x, y;
};

#define NR_POINT_DF_TEST_CLOSE(a,b,e) (NR_DF_TEST_CLOSE ((a)->x, (b)->x, e) && NR_DF_TEST_CLOSE ((a)->y, (b)->y, e))

namespace NR {

enum dimT { X, Y };

class Point{
 public:
	Coord pt[2];

	Point() {
	}

	Point(Coord x, Coord y) {
		pt[X] = x;
		pt[Y] = y;
	}

	Point(const NRPoint &p) {
		pt[X] = p.x;
		pt[Y] = p.y;
	}

	/** Return a point like this point but rotated -90 degrees.
	    (If the y axis grows downwards and the x axis grows to the
	    right, then this is 90 degrees counter-clockwise.)
	 **/
	Point ccw() const {
		return Point(pt[Y], -pt[X]);
	}

	/** Return a point like this point but rotated +90 degrees.
	    (If the y axis grows downwards and the x axis grows to the
	    right, then this is 90 degrees clockwise.)
	 **/
	Point cw() const {
		return Point(-pt[Y], pt[X]);
	}

	Coord L1() const ;
/** Compute the L1 norm, or manhattan distance, of this vector */
	Coord L2() const ;
/** Compute the L2 or euclidean norm of this vector */
	Coord Linfty() const ;
/** Compute the L infinity or maximum norm of this vector */
	void Normalize();
	
	operator NRPoint() const {
		NRPoint nrp;
		nrp.x = pt[0];
		nrp.y = pt[1];
		return nrp;
	}
};

inline Point
operator+(Point const &a, Point const &b) {
	Point r;
	for(int i = 0; i < 2; i++) {
		r.pt[i] = a.pt[i] + b.pt[i];
	}
	return r;
}

inline Point
operator-(Point const &a, Point const &b) {
	Point r;
	for(int i = 0; i < 2; i++) {
		r.pt[i] = a.pt[i] - b.pt[i];
	}
	return r;
}

inline Point
operator*(Point const &a, Point const &b) {
	Point r;
	for(int i = 0; i < 2; i++)
		r.pt[i] = a.pt[i]*b.pt[i];
	return r;
}

inline Point
operator*(const Coord s, Point const &b) {
	Point ret;
	for(int i = 0; i < 2; i++) {
		ret.pt[i] = s * b.pt[i];
	}
	return ret;
}

inline Point
operator/(Point const &b, const Coord d) {
	Point ret;
	for(int i = 0; i < 2; i++) {
		ret.pt[i] = b.pt[i] / d;
	}
	return ret;
}

inline Coord
dot(Point const &a, Point const &b) {
	Coord ret = 0;
	for(int i = 0; i < 2; i++) {
		ret += a.pt[i] * b.pt[i];
	}
	return ret;
}

inline Coord
cross(Point const &a, Point const &b) {
	Coord ret = 0;
	for(int i = 0; i < 2; i++)
		ret = a.pt[1-i] * b.pt[i] - ret;
	return ret;
}

inline void Point::Normalize() {
	Coord d = L2();
	if(d > 0.0001) // Why this number?
		*this = (1/d)**this;
}

Point abs(Point const &b);

} /* namespace NR */

struct NRRect {
	NR::Coord x0, y0, x1, y1;
};

struct NRPointL {
	NR::ICoord x, y;
};

struct NRRectL {
	NR::ICoord x0, y0, x1, y1;
};

#endif
