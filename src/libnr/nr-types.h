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

#include <libnr/nr_config.h>

typedef struct _NRMatrix {
	double c[6];
} NRMatrix;

typedef struct _NRPoint {
	double x, y;
} NRPoint;

namespace NR {

enum dimT { X, Y };

class Point{
 public:
	double pt[2];

	Point() {
	}

	Point(double x, double y) {
		pt[X] = x;
		pt[Y] = y;
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
	Point cw() {
		return Point(-pt[Y], pt[X]);
	}

	double L1();
/** Compute the L1 norm, or manhattan distance, of this vector */
	double L2();
/** Compute the L2 or euclidean norm of this vector */
	double Linfty();
/** Compute the L infinity or maximum norm of this vector */
	void Normalize();
	
	operator NRPoint() {
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
operator*(double s, Point const &b) {
	Point ret;
	for(int i = 0; i < 2; i++) {
		ret.pt[i] = s * b.pt[i];
	}
	return ret;
}

inline double
dot(Point const &a, Point const &b) {
	double ret = 0;
	for(int i = 0; i < 2; i++) {
		ret += a.pt[i] * b.pt[i];
	}
	return ret;
}

inline double
cross(Point const &a, Point const &b) {
	double ret = 0;
	for(int i = 0; i < 2; i++)
		ret = a.pt[1-i] * b.pt[i] - ret;
	return ret;
}

inline void Point::Normalize() {
	double d = L2();
	if(d > 0.0001) // Why this number?
		*this = (1/d)**this;
}
}

typedef struct _NRPointL {
	NRLong x, y;
} NRPointL;

typedef struct _NRPointS {
	NRShort x, y;
} NRPointS;

typedef struct _NRRect {
	double x0, y0, x1, y1;
} NRRect;

typedef struct _NRRectL {
	NRLong x0, y0, x1, y1;
} NRRectL;

typedef struct _NRRectS {
	NRShort x0, y0, x1, y1;
} NRRectS;

#endif
