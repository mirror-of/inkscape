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
nr_matrix_set_translate (NRMatrix *m, const NR::Coord x, const NR::Coord y)
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
nr_matrix_set_scale (NRMatrix *m, const NR::Coord sx, const NR::Coord sy)
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
nr_matrix_set_rotate (NRMatrix *m, const NR::Coord theta)
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

Matrix::Matrix(const NRMatrix *nr) {
	if (nr) {
		assign(nr->c);
	} else {
		set_identity();
	}
}

Matrix operator*(Matrix const &m0, Matrix const &m1)
{
	Matrix ret(m0[0] * m1[0] + m0[1] * m1[2],          m0[0] * m1[1] + m0[1] * m1[3],
		   m0[2] * m1[0] + m0[3] * m1[2],          m0[2] * m1[1] + m0[3] * m1[3],
		   m0[4] * m1[0] + m0[5] * m1[2] + m1[4],  m0[4] * m1[1] + m0[5] * m1[3] + m1[5]);
	return ret;
}

Matrix &Matrix::operator*=(Matrix const &o)
{
	*this = *this * o;
	return *this;
}

Matrix Matrix::inverse() const
{
	Matrix d(0,0,0,0,0,0);
	
	NR::Coord det = _c[0] * _c[3] - _c[1] * _c[2];
	if (!NR_DF_TEST_CLOSE (det, 0.0, NR_EPSILON)) {
		Coord t = _c[3] / det;
		d._c[3] = _c[0] / det;
		d._c[0] = t;

		d._c[1] = -_c[1] / det;
		d._c[2] = -_c[2] / det;
		d._c[4] = -_c[4] * d._c[0] - _c[5] * d._c[2];
		d._c[5] = -_c[4] * d._c[1] - _c[5] * d._c[3];
	} else {
		d.set_identity ();
	}

	return d;
}

void Matrix::set_identity ()
{
	_c[0] = 1.0; _c[1] = 0.0;
	_c[2] = 0.0; _c[3] = 1.0;
	// translation
	_c[4] = 0.0; _c[5] = 0.0;
}

Matrix identity ()
{
	Matrix ret(1.0, 0.0,
		   0.0, 1.0,
		   0.0, 0.0);
	return ret;
}

Matrix from_basis(const Point x_basis, const Point y_basis, const Point offset) {
	Matrix const ret(x_basis[X], y_basis[X],
			 x_basis[Y], y_basis[Y],
			 offset[X], offset[Y]);
	return ret;
}

rotate::rotate(NR::Coord const theta)
	: vec(cos(theta), sin(theta))
{
}

NR::Coord Matrix::det() const {
	return _c[0] * _c[3] - _c[1] * _c[2];
}

NR::Coord Matrix::descrim2() const {
	return fabs (det());
}

NR::Coord Matrix::descrim() const{
	return sqrt (descrim2());
}

Matrix &Matrix::assign(const Coord *array) {
	assert(array != NULL);

	for ( int i = 0 ; i < 6 ; i++ ) {
		_c[i] = array[i];
	}

	return *this;
}

NRMatrix *Matrix::copyto(NRMatrix *nrm) const {
	assert(nrm != NULL);
	
	for ( int i = 0 ; i < 6 ; i++ ) {
		nrm->c[i] = _c[i];
	}

	return nrm;
}

NR::Coord *Matrix::copyto(NR::Coord *array) const {
	assert(array != NULL);

	for ( int i = 0 ; i < 6 ; i++ ) {
		array[i] = _c[i];
	}

	return array;
}

double expansion(Matrix const &m) {
        return sqrt(fabs(m.det()));
}
                                                                                
bool Matrix::is_translation(const Coord eps) const {
    return ( fabs(_c[0]-1.0) < eps && 
             fabs(_c[3]-1.0) < eps &&
             fabs(_c[1]) < eps && 
             fabs(_c[2]) < eps );
}

bool Matrix::test_identity() const {
        return NR_MATRIX_DF_TEST_CLOSE (this, &NR_MATRIX_IDENTITY, NR_EPSILON);
}

bool transform_equalp(Matrix const &m0, Matrix const &m1, NR::Coord const epsilon) {
        return NR_MATRIX_DF_TEST_TRANSFORM_CLOSE (&m0, &m1, epsilon);
                                                                                
}

bool translate_equalp(Matrix const &m0, Matrix const &m1, NR::Coord const epsilon) {
        return NR_MATRIX_DF_TEST_TRANSLATE_CLOSE (&m0, &m1, epsilon);
}

bool matrix_equalp(Matrix const &m0, Matrix const &m1, NR::Coord const epsilon)
{
	return ( NR_MATRIX_DF_TEST_TRANSFORM_CLOSE(&m0, &m1, epsilon) &&
		 NR_MATRIX_DF_TEST_TRANSLATE_CLOSE(&m0, &m1, epsilon) );
}

void assert_close(Matrix const &a, Matrix const &b)
{
	if (!matrix_equalp(a, b, 1e-3)) {
		fprintf(stderr,
			"a = | %g %g |,\tb = | %g %g |\n"
			"    | %g %g | \t    | %g %g |\n"
			"    | %g %g | \t    | %g %g |\n",
			a[0], a[1], b[0], b[1],
			a[2], a[3], b[2], b[3],
			a[4], a[5], b[4], b[5]);
		abort();
	}
}

};
