#define __NR_RECT_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "nr-rect.h"

NRRect *
nr_rect_d_intersect (NRRect *d, const NRRect *r0, const NRRect *r1)
{
	if (!r0) {
		if (!r1) {
			d->x0 = -NR_HUGE;
			d->y0 = -NR_HUGE;
			d->x1 = NR_HUGE;
			d->y1 = NR_HUGE;
		} else {
			*d = *r1;
		}
	} else {
		if (!r1) {
			*d = *r0;
		} else {
			NR::Coord t;
			t = MAX (r0->x0, r1->x0);
			d->x1 = MIN (r0->x1, r1->x1);
			d->x0 = t;
			t = MAX (r0->y0, r1->y0);
			d->y1 = MIN (r0->y1, r1->y1);
			d->y0 = t;
		}
	}
	return d;
}

NRRect *
nr_rect_f_intersect (NRRect *d, const NRRect *r0, const NRRect *r1)
{
	return nr_rect_d_intersect(d, r0, r1);
}

NRRectL *
nr_rect_l_intersect (NRRectL *d, const NRRectL *r0, const NRRectL *r1)
{
	if (!r0) {
		if (!r1) {
			d->x0 = -NR_HUGE_L - 1;
			d->y0 = -NR_HUGE_L - 1;
			d->x1 = NR_HUGE_L;
			d->y1 = NR_HUGE_L;
		} else {
			*d = *r1;
		}
	} else {
		if (!r1) {
			*d = *r0;
		} else {
			NR::ICoord t;
			t = MAX (r0->x0, r1->x0);
			d->x1 = MIN (r0->x1, r1->x1);
			d->x0 = t;
			t = MAX (r0->y0, r1->y0);
			d->y1 = MIN (r0->y1, r1->y1);
			d->y0 = t;
		}
	}
	return d;
}

NRRect *
nr_rect_d_union (NRRect *d, const NRRect *r0, const NRRect *r1)
{
	if (!r0 || !r1) {
		d->x0 = -NR_HUGE;
		d->y0 = -NR_HUGE;
		d->x1 = NR_HUGE;
		d->y1 = NR_HUGE;
	} else if (NR_RECT_DFLS_TEST_EMPTY (r0)) {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			nr_rect_d_set_empty (d);
		} else {
			*d = *r1;
		}
	} else {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			*d = *r0;
		} else {
			NR::Coord t;
			t = MIN (r0->x0, r1->x0);
			d->x1 = MAX (r0->x1, r1->x1);
			d->x0 = t;
			t = MIN (r0->y0, r1->y0);
			d->y1 = MAX (r0->y1, r1->y1);
			d->y0 = t;
		}
	}
	return d;
}

NRRect *
nr_rect_f_union (NRRect *d, const NRRect *r0, const NRRect *r1)
{
	return nr_rect_d_union(d, r0, r1);
}

NRRectL *
nr_rect_l_union (NRRectL *d, const NRRectL *r0, const NRRectL *r1)
{
	if (!r0 || !r1) {
		d->x0 = -NR_HUGE_L - 1;
		d->y0 = -NR_HUGE_L - 1;
		d->x1 = NR_HUGE_L;
		d->y1 = NR_HUGE_L;
	} else if (NR_RECT_DFLS_TEST_EMPTY (r0)) {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			nr_rect_l_set_empty (d);
		} else {
			*d = *r1;
		}
	} else {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			*d = *r0;
		} else {
			NR::ICoord t;
			t = MIN (r0->x0, r1->x0);
			d->x1 = MAX (r0->x1, r1->x1);
			d->x0 = t;
			t = MIN (r0->y0, r1->y0);
			d->y1 = MAX (r0->y1, r1->y1);
			d->y0 = t;
		}
	}
	return d;
}

NRRect *
nr_rect_d_union_xy (NRRect *d, NR::Coord x, NR::Coord y)
{
	if (d) {
		if ((d->x0 <= d->x1) && (d->y0 <= d->y1)) {
			d->x0 = MIN (d->x0, x);
			d->y0 = MIN (d->y0, y);
			d->x1 = MAX (d->x1, x);
			d->y1 = MAX (d->y1, y);
		} else {
			d->x0 = d->x1 = x;
			d->y0 = d->y1 = y;
		}
	}
	return d;
}

NRRect *
nr_rect_f_union_xy (NRRect *d, float x, float y)
{
	return nr_rect_d_union_xy(d, x, y);
}

NRRectL *
nr_rect_l_union_xy (NRRectL *d, NR::ICoord x, NR::ICoord y)
{
	if (d) {
		if ((d->x0 <= d->x1) && (d->y0 <= d->y1)) {
			d->x0 = MIN (d->x0, x);
			d->y0 = MIN (d->y0, y);
			d->x1 = MAX (d->x1, x);
			d->y1 = MAX (d->y1, y);
		} else {
			d->x0 = d->x1 = x;
			d->y0 = d->y1 = y;
		}
	}
	return d;
}

NRRect *
nr_rect_d_matrix_d_transform (NRRect *d, NRRect *s, NRMatrix *m)
{
	if (nr_rect_d_test_empty (s)) {
		nr_rect_d_set_empty (d);
	} else {
		NR::Coord x0, y0, x1, y1, x, y;
		x0 = x1 = m->c[0] * s->x0 + m->c[2] * s->y0 + m->c[4];
		y0 = y1 = m->c[1] * s->x0 + m->c[3] * s->y0 + m->c[5];
		x = m->c[0] * s->x0 + m->c[2] * s->y1 + m->c[4];
		y = m->c[1] * s->x0 + m->c[3] * s->y1 + m->c[5];
		x0 = MIN (x0, x);
		y0 = MIN (y0, y);
		x1 = MAX (x1, x);
		y1 = MAX (y1, y);
		x = m->c[0] * s->x1 + m->c[2] * s->y0 + m->c[4];
		y = m->c[1] * s->x1 + m->c[3] * s->y0 + m->c[5];
		x0 = MIN (x0, x);
		y0 = MIN (y0, y);
		x1 = MAX (x1, x);
		y1 = MAX (y1, y);
		x = m->c[0] * s->x1 + m->c[2] * s->y1 + m->c[4];
		y = m->c[1] * s->x1 + m->c[3] * s->y1 + m->c[5];
		d->x0 = MIN (x0, x);
		d->y0 = MIN (y0, y);
		d->x1 = MAX (x1, x);
		d->y1 = MAX (y1, y);
	}
	return d;
}

NRRect *
nr_rect_f_matrix_f_transform (NRRect *d, NRRect *s, NRMatrix *m)
{
	return nr_rect_d_matrix_d_transform(d, s, m);
}


namespace NR{

/** returns the four corners of the rectangle in sequence for
 * the correct winding order. */
const Point Rect::corner(const int i) {
	switch(i % 4) {
	case 0:
		return topleft();
	case 1:
		return topright();
	case 2:
		return bottomright();
	case 3:
		return bottomleft();
	}
}
	
/** returns a vector from topleft to bottom right. */
	const Point Rect::dimensions() {
		return max - min;
	}

/** returns the midpoint of this rect. */
	const Point Rect::centre() {
		return (min + max)/2;
	}

/** Translates the rectangle by p. */
	void Rect::offset(Point p) {
		min += p;
		max += p;
	}

/** Makes this rectangle large enough to include the point p. */
	void Rect::least_bound(Point p) {
		for(int i=0; i < 2; i++) {
			min[i] = MIN(min[i], p[i]);
			max[i] = MAX(max[i], p[i]);
		}
	}

/** Returns the set of points shared by both rectangles. */
	Rect Rect::intersect(const Rect a, const Rect b) {
		Rect r;
		for(int i=0; i < 2; i++) {
			r.min[i] = MAX(a.min[i], b.min[i]);
			r.max[i] = MIN(a.max[i], b.max[i]);
		}
		return r;
	}
/** Returns the smallest rectangle that encloses both rectangles. */
	Rect Rect::least_bound(const Rect a, const Rect b) {
		Rect r;
		for(int i=0; i < 2; i++) {
			r.min[i] = MIN(a.min[i], b.min[i]);
			r.max[i] = MAX(a.max[i], b.max[i]);
		}
		return r;
	}

};
