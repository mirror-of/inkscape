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

/** a "real" type with sufficient precision for coordinates */
typedef double Coord;

/** an integer type with sufficient precision for coordinates */
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

enum dimT { X=0, Y };

class Point{
 public:
	Coord pt[2];

	Point() {
	}

	Point(Coord x, Coord y) {
		pt[X] = x;
		pt[Y] = y;
	}

	Point(NRPoint const &p) {
		pt[X] = p.x;
		pt[Y] = p.y;
	}

	double operator[](unsigned i) const {
		g_assert(i <= 1);
		return pt[i];
	}

	double &operator[](unsigned i) {
		g_assert(i <= 1);
		return pt[i];
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

	void normalize();
	
	operator NRPoint() const {
		NRPoint nrp;
		nrp.x = pt[0];
		nrp.y = pt[1];
		return nrp;
	}

	Point &operator+=(Point const &o) {
		for(unsigned i = 0; i < 2; ++i) {
			pt[i] += o.pt[i];
		}
		return *this;
	}
  
	Point &operator-=(Point const &o) {
		for(unsigned i = 0; i < 2; ++i) {
			pt[i] -= o.pt[i];
		}
		return *this;
	}
  
	Point &operator/=(double s) {
		for(unsigned i = 0; i < 2; ++i) {
			pt[i] /= s;
		}
		return *this;
	}

	Point &operator*=(double s) {
		for(unsigned i = 0; i < 2; ++i) {
			pt[i] *= s;
		}
		return *this;
	}
  
	Point &operator*=(Point const &s) {
		for(unsigned i = 0; i < 2; ++i) {
			pt[i] *= s[i];
		}
		return *this;
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
operator^(Point const &a, Point const &b) { // this is a rotation (sort of)
	Point r;
  r.pt[0] = a.pt[0] * b.pt[0] - a.pt[1] * b.pt[1];
  r.pt[1] = a.pt[1] * b.pt[0] + a.pt[0] * b.pt[1];
	return r;
}

inline Point
operator-(Point const &a) {
	Point ret;
	for(unsigned i = 0; i < 2; i++) {
		ret.pt[i] = -a.pt[i];
	}
	return ret;
}

inline Point
operator*(double const s, Point const &b) {
	Point ret;
	for(int i = 0; i < 2; i++) {
		ret.pt[i] = s * b.pt[i];
	}
	return ret;
}

inline Point
operator/(Point const &b, double const d) {
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
cross(Point const &a, Point const &b) { // defined as dot(a,b.cw())
	Coord ret = 0;
  ret -= a.pt[0] * b.pt[1];
  ret += a.pt[1] * b.pt[0];
	return ret;
}

inline bool
operator==(Point const &a, Point const &b) {
	return ((a.pt[X] == b.pt[X])  &&
		(a.pt[Y] == b.pt[Y]));
}

inline bool
operator!=(Point const &a, Point const &b) {
	return ((a.pt[X] != b.pt[X])  ||
		(a.pt[Y] != b.pt[Y]));
}

Point abs(Point const &b);

class Rect{
/** A rectangle is always aligned to the X and Y axis.  This means it
 * can be defined using only 4 coordinates, and determining
 * intersection is very efficient.  The points inside a rectangle are
 * min[dim] <= pt[dim] < max[dim].  This means that any rectangle
 * whose max is less than _or_ equal to its min is empty (contains no
 * points).  Infinities are allowed.*/
 private:
	Point min, max;
 public:
	Point topleft() {return min;}
	const Point topright() {return Point(max[X], min[Y]);}
	const Point bottomleft() {return Point(min[X], max[Y]);}
	Point bottomright() {return max;}
/** returns the four corners of the rectangle in sequence for the
 * correct winding order. */
	const Point corner(unsigned const i);
	
/** returns a vector from topleft to bottom right. */
	const Point dimensions();
/** returns the midpoint of this rect. */
	const Point centre();
	
/** Does this rectangle surround any points? */
	inline bool empty() const {
		return (min[0] > max[0]) || (min[1] > max[0]);
	}
/** Translates the rectangle by p. */
	void offset(Point p);
	
/** Makes this rectangle large enough to include the point p. */
	void least_bound(Point p);
	
/** Returns the set of points shared by both rectangles. */
	static Rect intersect(const Rect a, const Rect b);
/** Returns the smallest rectangle that encloses both rectangles. */
	static Rect least_bound(const Rect a, const Rect b);

};

} /* namespace NR */

struct NRRect {
	NR::Coord x0, y0, x1, y1;
};

inline bool empty(const NRRect r) {
	return (r.x0 > r.x1) || (r.y0 > r.y1);
}

struct NRPointL {
	NR::ICoord x, y;
};

struct NRRectL {
	NR::ICoord x0, y0, x1, y1;
};

#endif
