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

NRRectD *
nr_rect_d_intersect (NRRectD *d, const NRRectD *r0, const NRRectD *r1)
{
	if (!r0) {
		if (!r1) {
			d->x0 = -NR_HUGE_D;
			d->y0 = -NR_HUGE_D;
			d->x1 = NR_HUGE_D;
			d->y1 = NR_HUGE_D;
		} else {
			*d = *r1;
		}
	} else {
		if (!r1) {
			*d = *r0;
		} else {
			double t;
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

NRRectF *
nr_rect_f_intersect (NRRectF *d, const NRRectF *r0, const NRRectF *r1)
{
	if (!r0) {
		if (!r1) {
			d->x0 = -NR_HUGE_F;
			d->y0 = -NR_HUGE_F;
			d->x1 = NR_HUGE_F;
			d->y1 = NR_HUGE_F;
		} else {
			*d = *r1;
		}
	} else {
		if (!r1) {
			*d = *r0;
		} else {
			float t;
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
			NRLong t;
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

NRRectS *
nr_rect_s_intersect (NRRectS *d, const NRRectS *r0, const NRRectS *r1)
{
	if (!r0) {
		if (!r1) {
			d->x0 = -NR_HUGE_S - 1;
			d->y0 = -NR_HUGE_S - 1;
			d->x1 = NR_HUGE_S;
			d->y1 = NR_HUGE_S;
		} else {
			*d = *r1;
		}
	} else {
		if (!r1) {
			*d = *r0;
		} else {
			NRShort t;
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

NRRectD *
nr_rect_d_union (NRRectD *d, const NRRectD *r0, const NRRectD *r1)
{
	if (!r0 || !r1) {
		d->x0 = -NR_HUGE_D;
		d->y0 = -NR_HUGE_D;
		d->x1 = NR_HUGE_D;
		d->y1 = NR_HUGE_D;
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
			double t;
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

NRRectF *
nr_rect_f_union (NRRectF *d, const NRRectF *r0, const NRRectF *r1)
{
	if (!r0 || !r1) {
		d->x0 = -NR_HUGE_F;
		d->y0 = -NR_HUGE_F;
		d->x1 = NR_HUGE_F;
		d->y1 = NR_HUGE_F;
	} else if (NR_RECT_DFLS_TEST_EMPTY (r0)) {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			nr_rect_f_set_empty (d);
		} else {
			*d = *r1;
		}
	} else {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			*d = *r0;
		} else {
			float t;
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
			NRLong t;
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

NRRectS *
nr_rect_s_union (NRRectS *d, const NRRectS *r0, const NRRectS *r1)
{
	if (!r0 || !r1) {
		d->x0 = -NR_HUGE_S - 1;
		d->y0 = -NR_HUGE_S - 1;
		d->x1 = NR_HUGE_S;
		d->y1 = NR_HUGE_S;
	} else if (NR_RECT_DFLS_TEST_EMPTY (r0)) {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			nr_rect_s_set_empty (d);
		} else {
			*d = *r1;
		}
	} else {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			*d = *r0;
		} else {
			NRShort t;
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

NRRectD *
nr_rect_d_union_xy (NRRectD *d, double x, double y)
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

NRRectF *
nr_rect_f_union_xy (NRRectF *d, float x, float y)
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

NRRectL *
nr_rect_l_union_xy (NRRectL *d, NRLong x, NRLong y)
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

NRRectS *
nr_rect_s_union_xy (NRRectS *d, NRShort x, NRShort y)
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

NRRectD *
nr_rect_d_matrix_d_transform (NRRectD *d, NRRectD *s, NRMatrixD *m)
{
	if (nr_rect_d_test_empty (s)) {
		nr_rect_d_set_empty (d);
	} else {
		double x0, y0, x1, y1, x, y;
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

NRRectF *
nr_rect_f_matrix_f_transform (NRRectF *d, NRRectF *s, NRMatrixF *m)
{
	if (nr_rect_f_test_empty (s)) {
		nr_rect_f_set_empty (d);
	} else {
		float x0, y0, x1, y1, x, y;
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

