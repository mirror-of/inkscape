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

Matrix::Matrix(const NRMatrix* nr) {
	if(nr) {
		for(int i = 0; i < 6; i++)
			cc[i] = nr->c[i];
	} else {
		set_identity();
	}
}
	
Matrix::operator NRMatrix*() const {
	NRMatrix* d = new NRMatrix;
	for(int i = 0; i < 6; i++)
		d->c[i] = cc[i];
	return d;
}

Matrix::Matrix(Point x_basis, Point y_basis, Point offset) {
	for (int i = 0; i < 2; i++) {
		cc[2*i + NR::X] = x_basis.pt[i];
		cc[2*i + NR::Y] = y_basis.pt[i];
		cc[4+i] = offset.pt[i];
	}
}

Matrix operator*(const Matrix m0, const Matrix m1)
{
	Matrix d;
	
	d.cc[0] = m0.cc[0] * m1.cc[0] + m0.cc[1] * m1.cc[2];
	d.cc[1] = m0.cc[0] * m1.cc[1] + m0.cc[1] * m1.cc[3];
	d.cc[2] = m0.cc[2] * m1.cc[0] + m0.cc[3] * m1.cc[2];
	d.cc[3] = m0.cc[2] * m1.cc[1] + m0.cc[3] * m1.cc[3];
	d.cc[4] = m0.cc[4] * m1.cc[0] + m0.cc[5] * m1.cc[2] + m1.cc[4];
	d.cc[5] = m0.cc[4] * m1.cc[1] + m0.cc[5] * m1.cc[3] + m1.cc[5];
	
	return d;
}

Matrix Matrix::inverse() const
{
	Matrix d;
	
	NR::Coord det = cc[0] * cc[3] - cc[1] * cc[2];
	if (!NR_DF_TEST_CLOSE (det, 0.0, NR_EPSILON)) {
		Coord t = cc[3] / det;
		d.cc[3] = cc[0] / det;
		d.cc[0] = t;
		t = -cc[1] / det;
		d.cc[1] = -cc[1] / det;
		d.cc[2] = -cc[2] / det;
		d.cc[4] = -cc[4] * d.cc[0] - cc[5] * d.cc[2];
		d.cc[5] = -cc[4] * d.cc[1] - cc[5] * d.cc[3];
	} else {
		d.set_identity ();
	}

	return d;
}

void Matrix::set_translate (const Point p)
{
	cc[0] = 1.0; cc[2] = 0.0;
	cc[1] = 0.0; cc[3] = 1.0;
	// translation
	for(int i = 0; i < 2; i++)
		cc[4+i] = p.pt[i];
}

void Matrix::set_identity ()
{
	cc[0] = 1.0; cc[2] = 0.0;
	cc[1] = 0.0; cc[3] = 1.0;
	// translation
	cc[4] = 0; cc[5] = 0;
}

void Matrix::set_scale (const Point scale)
{
	cc[0] = scale.pt[NR::X];  cc[2] = 0.0;
	cc[1] = 0.0;              cc[3] = scale.pt[NR::Y];
	// translation
	cc[4] = 0.0;
	cc[5] = 0.0;
}

void Matrix::set_rotate (const NR::Coord theta)
{
	NR::Coord sn = sin (theta);
	NR::Coord cs = cos (theta);
	cc[0] = cs; cc[2] = -sn; // this may be backwards (anglewise)
	cc[1] = sn; cc[3] = cs;  // from standard maths def.
	// translation
	cc[4] = 0.0;
	cc[5] = 0.0;
}

NR::Coord Matrix::det() const {
	return cc[0] * cc[3] - cc[1] * cc[2];
}

NR::Coord Matrix::descrim2() const {
	return fabs (det());
}

NR::Coord Matrix::descrim() const{
	return sqrt (descrim2());
}

void Matrix::copy(NRMatrix* nrm) {
	assert(nrm);
	
	for(int i = 0; i < 6; i++)
		nrm->c[i] = cc[i];
}

};
