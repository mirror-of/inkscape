#ifndef __NR_RECT_H__
#define __NR_RECT_H__

/* ex:set et ts=4 sw=4: */

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   MenTaLguY <mental@rydia.net>
 *
 * This code is in public domain
 */

#include <libnr/nr-coord.h>
#include <libnr/nr-i-coord.h>
#include <libnr/nr-dim2.h>
#include <libnr/nr-macros.h>
#include <libnr/nr-point.h>
#include <libnr/nr-values.h>
#include <libnr/nr-maybe.h>

/* NULL rect is infinite */

struct NRRect {
	NR::Coord x0, y0, x1, y1;
};

inline bool empty(NRRect const &r)
{
	return ( ( r.x0 > r.x1 ) ||
		 ( r.y0 > r.y1 ) );
}

#define nr_rect_d_set_empty(r) (*(r) = NR_RECT_EMPTY)
#define nr_rect_f_set_empty(r) (*(r) = NR_RECT_EMPTY)
#define nr_rect_l_set_empty(r) (*(r) = NR_RECT_L_EMPTY)

#define nr_rect_d_test_empty(r) ((r) && NR_RECT_DFLS_TEST_EMPTY (r))
#define nr_rect_f_test_empty(r) ((r) && NR_RECT_DFLS_TEST_EMPTY (r))
#define nr_rect_l_test_empty(r) ((r) && NR_RECT_DFLS_TEST_EMPTY (r))

#define nr_rect_d_test_intersect(r0,r1) \
	(!nr_rect_d_test_empty (r0) && !nr_rect_d_test_empty (r1) && \
	 !((r0) && (r1) && !NR_RECT_DFLS_TEST_INTERSECT (r0, r1)))
#define nr_rect_f_test_intersect(r0,r1) \
	(!nr_rect_f_test_empty (r0) && !nr_rect_f_test_empty (r1) && \
	 !((r0) && (r1) && !NR_RECT_DFLS_TEST_INTERSECT (r0, r1)))
#define nr_rect_l_test_intersect(r0,r1) \
	(!nr_rect_l_test_empty (r0) && !nr_rect_l_test_empty (r1) && \
	 !((r0) && (r1) && !NR_RECT_DFLS_TEST_INTERSECT (r0, r1)))

#define nr_rect_d_point_d_test_inside(r,p) ((p) && (!(r) || (!NR_RECT_DF_TEST_EMPTY (r) && NR_RECT_DF_POINT_DF_TEST_INSIDE (r,p))))
#define nr_rect_f_point_f_test_inside(r,p) ((p) && (!(r) || (!NR_RECT_DF_TEST_EMPTY (r) && NR_RECT_DF_POINT_DF_TEST_INSIDE (r,p))))

/* NULL values are OK for r0 and r1, but not for d */
NRRect *nr_rect_d_intersect (NRRect *d, const NRRect *r0, const NRRect *r1);
NRRect *nr_rect_f_intersect (NRRect *d, const NRRect *r0, const NRRect *r1);
NRRectL *nr_rect_l_intersect (NRRectL *d, const NRRectL *r0, const NRRectL *r1);

NRRect *nr_rect_d_union (NRRect *d, const NRRect *r0, const NRRect *r1);
NRRect *nr_rect_f_union (NRRect *d, const NRRect *r0, const NRRect *r1);
NRRectL *nr_rect_l_union (NRRectL *d, const NRRectL *r0, const NRRectL *r1);

NRRect *nr_rect_d_union_xy (NRRect *d, NR::Coord x, NR::Coord y);
NRRect *nr_rect_f_union_xy (NRRect *d, float x, float y);
NRRectL *nr_rect_l_union_xy (NRRectL *d, NR::ICoord x, NR::ICoord y);

NRRect *nr_rect_d_matrix_d_transform (NRRect *d, NRRect *s, NRMatrix *m);
NRRect *nr_rect_f_matrix_f_transform (NRRect *d, NRRect *s, NRMatrix *m);

#include <stdexcept>
#include <typeinfo>

namespace NR {

/** A rectangle is always aligned to the X and Y axis.  This means it
 * can be defined using only 4 coordinates, and determining
 * intersection is very efficient.  The points inside a rectangle are
 * min[dim] <= _pt[dim] <= max[dim].  Emptiness, however, is defined
 * as having zero area, meaning an empty rectangle may still contain
 * points.  Infinities are also permitted. */
class Rect {
public:
	Rect(const NRRect& r) : _min(r.x0, r.y0), _max(r.x1, r.y1) {}
	Rect(const Rect& r) : _min(r._min), _max(r._max) {}
	Rect(const Point &p0, const Point &p1);
	
	const Point &topleft() const { return _min; }
	Point topright() const { return Point(_max[X], _min[Y]); }
	Point bottomleft() const { return Point(_min[X], _max[Y]); }
	const Point &bottomright() const { return _max; }

	const Point &min() const { return _min; }
	const Point &max() const { return _max; }

	/** returns the four corners of the rectangle in the correct
	 *  winding order */
	Point corner(unsigned i) const;
	
	/** returns a vector from topleft to bottom right. */
	Point dimensions() const;

	/** returns the midpoint of this rect. */
	Point midpoint() const;
	
	/** does this rectangle have zero area? */
	bool isEmpty() const {
		return isEmpty<X>() && isEmpty<Y>();
	}

	bool intersects(const Rect &r) const {
		return intersects<X>(r) && intersects<Y>(r);
	}
	bool contains(const Rect &r) const {
		return contains<X>(r) && contains<Y>(r);
	}
	bool contains(const Point &p) const {
		return contains<X>(p) && contains<Y>(p);
	}

	/** Translates the rectangle by p. */
	void offset(Point p);
	
	/** Makes this rectangle large enough to include the point p. */
	void expandTo(Point p);

	/** Makes this rectangle large enough to include the rectangle r. */
	void expandTo(const Rect &r);
	
	/** Returns the set of points shared by both rectangles. */
	static Rect intersection(const Rect &a, const Rect &b);

	/** Returns the smallest rectangle that encloses both rectangles. */
	static Rect union_bounds(const Rect &a, const Rect &b);

private:
	Rect() {}

	template <Dim2 axis>
	bool isEmpty() const {
		return !( _min[axis] < _max[axis] );
	}

	template <Dim2 axis>
	bool intersects(const Rect &r) const {
		return r._min[axis] <=   _max[axis] &&
		         _min[axis] <= r._max[axis];
	}

	template <Dim2 axis>
	bool contains(const Rect &r) const {
		return contains(r._min) && contains(r._max);
	}

	template <Dim2 axis>
	bool contains(const Point &p) const {
		return p[axis] >= _min[axis] && p[axis] <= _max[axis];
	}

	Point _min, _max;
};

class ConvexHull {
public:
	ConvexHull(const Point &p) : _bounds(p, p) {}
	ConvexHull(const ConvexHull &h) : _bounds(h._bounds) {}
													
	void add(const Point &p) {
		_bounds.expandTo(p);
	}
	void add(const ConvexHull &h) {
		_bounds.expandTo(h._bounds);
	}
		
	const Rect &getBounds() const {
		return _bounds;
	}
	
private:
	Rect _bounds;
};

} /* namespace NR */

#endif
