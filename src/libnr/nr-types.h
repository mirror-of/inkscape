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
#include <stdexcept>

namespace NR {

/** a "real" type with sufficient precision for coordinates */
typedef double Coord;

/** an integer type with sufficient precision for coordinates */
typedef gint32 ICoord;

class Point;
class Rect;
class Matrix;

} /* namespace NR */

struct NRMatrix {
	NR::Coord c[6];

	NR::Coord &operator[](int i) { return c[i]; }
	NR::Coord operator[](int i) const { return c[i]; }
};

struct NRPoint {
	NR::Coord x, y;
};

#define NR_POINT_DF_TEST_CLOSE(a,b,e) (NR_DF_TEST_CLOSE ((a)->x, (b)->x, e) && NR_DF_TEST_CLOSE ((a)->y, (b)->y, e))

struct NRRect {
	NR::Coord x0, y0, x1, y1;
};

inline bool empty(const NRRect &r) {
	return ( r.x0 > r.x1 ) || ( r.y0 > r.y1 );
}

struct NRPointL {
	NR::ICoord x, y;
};

struct NRRectL {
	NR::ICoord x0, y0, x1, y1;
};

namespace NR {

class Matrix;

enum Dim2 { X=0, Y };

class Point {
public:
	Point() {
	}

	Point(Coord x, Coord y) {
		_pt[X] = x;
		_pt[Y] = y;
	}

	Point(NRPoint const &p) {
		_pt[X] = p.x;
		_pt[Y] = p.y;
	}

	Coord operator[](unsigned i) const throw(std::out_of_range) {
		if ( i > Y ) {
			throw std::out_of_range("index out of range");
		}
		return _pt[i];
	}
	Coord &operator[](unsigned i) throw(std::out_of_range) {
		if ( i > Y ) {
			throw std::out_of_range("index out of range");
		}
		return _pt[i];
	}

	Coord operator[](Dim2 d) const throw() { return _pt[d]; }
	Coord &operator[](Dim2 d) throw() { return _pt[d]; }

	/** Return a point like this point but rotated -90 degrees.
	    (If the y axis grows downwards and the x axis grows to the
	    right, then this is 90 degrees counter-clockwise.)
	 **/
	Point ccw() const {
		return Point(_pt[Y], -_pt[X]);
	}

	/** Return a point like this point but rotated +90 degrees.
	    (If the y axis grows downwards and the x axis grows to the
	    right, then this is 90 degrees clockwise.)
	 **/
	Point cw() const {
		return Point(-_pt[Y], _pt[X]);
	}

	void normalize();
	
	operator NRPoint() const {
		NRPoint nrp;
		nrp.x = _pt[X];
		nrp.y = _pt[Y];
		return nrp;
	}

	Point &operator+=(Point const &o) {
		for (unsigned i = 0; i < 2; ++i) {
			_pt[i] += o._pt[i];
		}
		return *this;
	}
  
	Point &operator-=(Point const &o) {
		for ( unsigned i = 0 ; i < 2 ; ++i ) {
			_pt[i] -= o._pt[i];
		}
		return *this;
	}
  
	Point &operator/=(double s) {
		for ( unsigned i = 0 ; i < 2 ; ++i ) {
			_pt[i] /= s;
		}
		return *this;
	}

	Point &operator*=(double s) {
		for ( unsigned i = 0 ; i < 2 ; ++i ) {
			_pt[i] *= s;
		}
		return *this;
	}
  
	Point &operator*=(Point const &s) {
		for ( unsigned i = 0 ; i < 2 ; ++i ) {
			_pt[i] *= s[i];
		}
		return *this;
	}

	Point &operator*=(Matrix const &m);

private:
	Coord _pt[2];
};

inline Point
operator+(Point const &a, Point const &b) {
	Point r;
	for(int i = 0; i < 2; i++) {
		r[i] = a[i] + b[i];
	}
	return r;
}

inline Point
operator-(Point const &a, Point const &b) {
	Point r;
	for(int i = 0; i < 2; i++) {
		r[i] = a[i] - b[i];
	}
	return r;
}

inline Point
operator^(Point const &a, Point const &b) { // this is a rotation (sort of)
	Point r;
	r[0] = a[0] * b[0] - a[1] * b[1];
	r[1] = a[1] * b[0] + a[0] * b[1];
	return r;
}

inline Point
operator-(Point const &a) {
	Point ret;
	for(unsigned i = 0; i < 2; i++) {
		ret[i] = -a[i];
	}
	return ret;
}

inline Point
operator*(double const s, Point const &b) {
	Point ret;
	for(int i = 0; i < 2; i++) {
		ret[i] = s * b[i];
	}
	return ret;
}

inline Point
operator/(Point const &b, double const d) {
	Point ret;
	for(int i = 0; i < 2; i++) {
		ret[i] = b[i] / d;
	}
	return ret;
}

inline Coord
dot(Point const &a, Point const &b) {
	Coord ret = 0;
	for ( int i = 0 ; i < 2 ; i++ ) {
		ret += a[i] * b[i];
	}
	return ret;
}

inline Coord
cross(Point const &a, Point const &b) { // defined as dot(a,b.cw())
	Coord ret = 0;
	ret -= a[0] * b[1];
	ret += a[1] * b[0];
	return ret;
}

inline bool
operator==(Point const &a, Point const &b) {
	return ( ( a[X] == b[X] ) && ( a[Y] == b[Y] ) );
}

inline bool
operator!=(Point const &a, Point const &b) {
	return ( ( a[X] != b[X] ) || ( a[Y] != b[Y] ) );
}

Point abs(Point const &b);

} /* namespace NR */

#endif
