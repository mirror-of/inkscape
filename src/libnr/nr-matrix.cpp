#define __NR_MATRIX_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "nr-matrix.h"

NRMatrix *
nr_matrix_d_from_f (NRMatrix *d, const NRMatrix *s)
{
	if (s) {
		for(int i = 0; i < 6; i++)
			d->c[i] = s->c[i];
	} else {
		nr_matrix_set_identity (d);
	}

	return d;
}

NRMatrix *
nr_matrix_f_from_d (NRMatrix *d, const NRMatrix *s)
{
	return nr_matrix_d_from_f(d, s);
}

NRMatrix *
nr_matrix_multiply (NRMatrix *d, const NRMatrix *m0, const NRMatrix *m1)
{
	if (m0) {
		if (m1) {
			NR::Coord d0, d1, d2, d3, d4, d5;

			d0 = m0->c[0] * m1->c[0] + m0->c[1] * m1->c[2];
			d1 = m0->c[0] * m1->c[1] + m0->c[1] * m1->c[3];
			d2 = m0->c[2] * m1->c[0] + m0->c[3] * m1->c[2];
			d3 = m0->c[2] * m1->c[1] + m0->c[3] * m1->c[3];
			d4 = m0->c[4] * m1->c[0] + m0->c[5] * m1->c[2] + m1->c[4];
			d5 = m0->c[4] * m1->c[1] + m0->c[5] * m1->c[3] + m1->c[5];
			d->c[0] = d0;
			d->c[1] = d1;
			d->c[2] = d2;
			d->c[3] = d3;
			d->c[4] = d4;
			d->c[5] = d5;
		} else {
			*d = *m0;
		}
	} else {
		if (m1) {
			*d = *m1;
		} else {
			nr_matrix_set_identity (d);
		}
	}

	return d;
}

NRMatrix *
nr_matrix_invert (NRMatrix *d, const NRMatrix *m)
{
	if (m) {
		NR::Coord det;
		det = m->c[0] * m->c[3] - m->c[1] * m->c[2];
		if (!NR_DF_TEST_CLOSE (det, 0.0, NR_EPSILON)) {
			NR::Coord rdet, t;
			rdet = 1.0 / det;
			t = m->c[3] * rdet;
			d->c[3] = m->c[0] * rdet;
			d->c[0] = t;
			t = -m->c[1] * rdet;
			d->c[1] = -m->c[1] * rdet;
			d->c[2] = -m->c[2] * rdet;
			d->c[4] = -m->c[4] * d->c[0] - m->c[5] * d->c[2];
			d->c[5] = -m->c[4] * d->c[1] - m->c[5] * d->c[3];
		} else {
			nr_matrix_set_identity (d);
		}
	} else {
		nr_matrix_set_identity (d);
	}

	return d;
}

NRMatrix *
nr_matrix_set_translate (NRMatrix *m, NR::Coord x, NR::Coord y)
{
	m->c[0] = 1.0;
	m->c[1] = 0.0;
	m->c[2] = 0.0;
	m->c[3] = 1.0;
	m->c[4] = x;
	m->c[5] = y;

	return m;
}

NRMatrix *
nr_matrix_set_scale (NRMatrix *m, NR::Coord sx, NR::Coord sy)
{
	m->c[0] = sx;
	m->c[1] = 0.0;
	m->c[2] = 0.0;
	m->c[3] = sy;
	m->c[4] = 0.0;
	m->c[5] = 0.0;

	return m;
}

NRMatrix *
nr_matrix_set_rotate (NRMatrix *m, NR::Coord theta)
{
	NR::Coord s, c;
	s = sin (theta);
	c = cos (theta);
	m->c[0] = c;
	m->c[1] = s;
	m->c[2] = -s;
	m->c[3] = c;
	m->c[4] = 0.0;
	m->c[5] = 0.0;
	return m;
}


namespace NR {

Matrix::Matrix(NRMatrix nr) {
	for(int i = 0; i < 6; i++)
		c[i] = nr.c[i];
}
	
Matrix::operator NRMatrix() const {
	NRMatrix nrm;
	for(int i = 0; i < 6; i++)
		nrm.c[i] = c[i];
	return nrm;
}

};

/*
Matrix Matrix::multiply (const Matrix m0, const Matrix m1)
{
	Matrix d;
	NR::Coord d0, d1, d2, d3, d4, d5;
	
	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 2; j++)
			d.c[] = m0.c[i*2]*
	d.c[0] = m0.c[0] * m1.c[0] + m0.c[1] * m1.c[2];
	d.c[1] = m0.c[0] * m1.c[1] + m0.c[1] * m1.c[3];
	d.c[2] = m0.c[2] * m1.c[0] + m0.c[3] * m1.c[2];
	d.c[3] = m0.c[2] * m1.c[1] + m0.c[3] * m1.c[3];
	d.c[4] = m0.c[4] * m1.c[0] + m0.c[5] * m1.c[2] + m1.c[4];
	d.c[5] = m0.c[4] * m1.c[1] + m0.c[5] * m1.c[3] + m1.c[5];

	return d;
}

Matrix Matrix::invert ()
{
	Matrix d;
	if (m) {
		NR::Coord det;
		det = m.c[0] * m.c[3] - m.c[1] * m.c[2];
		if (!NR_DF_TEST_CLOSE (det, 0.0, NR_EPSILON)) {
			NR::Coord rdet, t;
			rdet = 1.0 / det;
			t = m.c[3] * rdet;
			d.c[3] = m.c[0] * rdet;
			d.c[0] = t;
			t = -m.c[1] * rdet;
			d.c[1] = -m.c[1] * rdet;
			d.c[2] = -m.c[2] * rdet;
			d.c[4] = -m.c[4] * d.c[0] - m.c[5] * d.c[2];
			d.c[5] = -m.c[4] * d.c[1] - m.c[5] * d.c[3];
		} else {
			nr_matrix_set_identity (d);
		}
	} else {
		nr_matrix_set_identity (d);
	}

	return d;
}

Matrix Matrix::set_translate (Point p)
{
	c[0] = 1.0; c[2] = 0.0;
	c[1] = 0.0; c[3] = 1.0;
	// translation
	c[4] = x;
	c[5] = y;

	return m;
}

void set_scale (Point scale)
{
	c[0] = sx;  c[2] = 0.0;
	c[1] = 0.0; c[3] = sy;
	// translation
	c[4] = 0.0;
	c[5] = 0.0;

	return m;
}

void set_rotate (const NR::Coord theta)
{
	NR::Coord s = sin (theta);
	NR::Coord c = cos (theta);
	c[0] = c; c[2] = -s;
	c[1] = s; c[3] = c;
	// translation
	c[4] = 0.0;
	c[5] = 0.0;
	return m;
}

};
*/
