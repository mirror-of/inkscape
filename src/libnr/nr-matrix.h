#ifndef __NR_MATRIX_H__
#define __NR_MATRIX_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-macros.h>
#include <libnr/nr-values.h>
#include <libnr/nr-types.h>

#define nr_matrix_set_identity(m) (*(m) = NR_MATRIX_IDENTITY)

#define nr_matrix_test_identity(m,e) (!(m) || NR_MATRIX_DF_TEST_CLOSE (m, &NR_MATRIX_IDENTITY, e))

#define nr_matrix_test_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && NR_MATRIX_DF_TEST_CLOSE (m0, m1, e)))
#define nr_matrix_test_transform_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && NR_MATRIX_DF_TEST_TRANSFORM_CLOSE (m0, m1, e)))
#define nr_matrix_test_translate_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && NR_MATRIX_DF_TEST_TRANSLATE_CLOSE (m0, m1, e)))

NRMatrix *nr_matrix_invert (NRMatrix *d, const NRMatrix *m);

/* d,m0,m1 needn't be distinct in any of these multiply routines. */

NRMatrix *nr_matrix_multiply (NRMatrix *d, const NRMatrix *m0, const NRMatrix *m1);

NRMatrix *nr_matrix_set_translate (NRMatrix *m, const NR::Coord x, const NR::Coord y);

NRMatrix *nr_matrix_set_scale (NRMatrix *m, const NR::Coord sx, const NR::Coord sy);

NRMatrix *nr_matrix_set_rotate (NRMatrix *m, const NR::Coord theta);

#define NR_MATRIX_DF_TRANSFORM_X(m,x,y) ((*(m))[0] * (x) + (*(m))[2] * (y) + (*(m))[4])
#define NR_MATRIX_DF_TRANSFORM_Y(m,x,y) ((*(m))[1] * (x) + (*(m))[3] * (y) + (*(m))[5])

#define NR_MATRIX_DF_EXPANSION2(m) (fabs ((*(m))[0] * (*(m))[3] - (*(m))[1] * (*(m))[2]))
#define NR_MATRIX_DF_EXPANSION(m) (sqrt (NR_MATRIX_DF_EXPANSION2 (m)))

namespace NR {

class scale {
private:
	Point _p;

private:
	scale();

public:
	explicit scale(Point const &p) : _p(p) {}
	scale(double const x, double const y) : _p(x, y) {}
	inline Coord operator[](Dim2 const d) const { return _p[d]; }
	inline Coord operator[](unsigned const d) const { return _p[d]; }

	bool operator==(scale const &o) const {
		return _p == o._p;
	}

	bool operator!=(scale const &o) const {
		return _p != o._p;
	}
};

inline Point operator*(Point const &p, scale const &s)
{
	return Point(s[X] * p[X],
		     s[Y] * p[Y]);
}

inline scale operator*(scale const &a, scale const &b)
{
	return scale(a[X] * b[X],
		     a[Y] * b[Y]);
}

inline scale operator/(scale const &numer, scale const &denom)
{
	return scale(numer[X] / denom[X],
		     numer[Y] / denom[Y]);
}

class rotate {
public:
	Point vec;

private:
	rotate();

public:
	explicit rotate(Coord theta);
	explicit rotate(Point const &p) : vec(p) {}

	bool operator==(rotate const &o) const {
		return vec == o.vec;
	}

	bool operator!=(rotate const &o) const {
		return vec != o.vec;
	}

	rotate inverse() const {
		/* TODO: In the usual case that vec is a unit vector (within rounding error),
		   dividing by len_sq is either a noop or numerically harmful.
		   Make a unit_rotate class (or the like) that knows its length is 1. */
		double const len_sq = dot(vec, vec);
		return rotate( Point(vec[X], -vec[Y])
			       / len_sq );
	}
};

inline Point operator*(Point const &v, rotate const &r)
{
	return Point(r.vec[X] * v[X] - r.vec[Y] * v[Y],
		     r.vec[Y] * v[X] + r.vec[X] * v[Y]);
}

inline rotate operator*(rotate const &a, rotate const &b)
{
	return rotate( a.vec * b );
}

inline rotate operator/(rotate const &numer, rotate const &denom)
{
	return numer * denom.inverse();
}


class translate {
public:
	Point offset;
private:
	translate();
public:
	explicit translate(Point const &p) : offset(p) {}
	explicit translate(Coord const x, Coord const y) : offset(x, y) {}
	Coord operator[](Dim2 const dim) const { return offset[dim]; }
	Coord operator[](unsigned const dim) const { return offset[dim]; }
};

inline bool operator==(translate const &a, translate const &b)
{
	return a.offset == b.offset;
}

inline translate operator*(translate const &a, translate const &b)
{
	return translate( a.offset + b.offset );
}

inline Point operator*(Point const &v, translate const &t)
{
	return t.offset + v;
}

/*
 * For purposes of multiplication, points should be thought of as row vectors
 *
 *    p = ( p[X] p[Y]  1  )
 *
 * to be right-multiplied by transformation matrices
 *
 *  c[] = | c[0] c[1]  0  |
 *        | c[2] c[3]  0  |
 *        | c[4] c[5]  1  |
 *
 * (so the columns of the matrix correspond to the columns (elements) of the result,
 * and the rows of the matrix correspond to columns (elements) of the "input").
 */
class Matrix {
public:
	explicit Matrix() { }

	Matrix(Matrix const &m) {
		for ( int i = 0 ; i < 6 ; i++ ) {
			_c[i] = m._c[i];
		}
	}

	Matrix(NRMatrix const &m) {
		for ( int i = 0 ; i < 6 ; i++ ) {
			_c[i] = m.c[i];
		}
	}

	Matrix(double c0, double c1,
	       double c2, double c3,
	       double c4, double c5)
	{
		_c[0] = c0; _c[1] = c1;
		_c[2] = c2; _c[3] = c3;
		_c[4] = c4; _c[5] = c5;
	}

	Matrix &operator=(Matrix const &m) {
		for (unsigned i = 0 ; i < 6 ; ++i) {
			_c[i] = m._c[i];
		}
		return *this;
	}

	Matrix(scale const &sm) {
		_c[0] = sm[X]; _c[1] = 0;
		_c[2] = 0;     _c[3] =  sm[Y];
		_c[4] = 0;     _c[5] = 0;
	}

	Matrix(rotate const &r) {
		_c[0] =  r.vec[X]; _c[1] = r.vec[Y];
		_c[2] = -r.vec[Y]; _c[3] = r.vec[X];
		_c[4] = 0;         _c[5] = 0;
	}

	Matrix(translate const &tm) {
		_c[0] = 1;     _c[1] = 0;
		_c[2] = 0;     _c[3] = 1;
		_c[4] = tm[X]; _c[5] = tm[Y];
	}

	Matrix(NRMatrix const *nr);
	
	bool test_identity() const;
	Matrix inverse() const;

	Matrix &operator*=(Matrix const &o);

	Coord &operator[](int const i) {
		g_assert( unsigned(i) < 6 );
		return _c[i];
	}

	Coord operator[](int const i) const {
		g_assert( unsigned(i) < 6 );
		return _c[i];
	}

	void set_identity();
	
	// What do these do?  some kind of norm?
	Coord det() const;
	Coord descrim2() const;
	Coord descrim() const;
	
	// legacy
	Matrix &assign(const Coord *array);
	NRMatrix *copyto(NRMatrix* nrm) const;
	Coord *copyto(Coord *array) const;

	operator NRMatrix&() {
		g_assert(sizeof(_c) == sizeof(NRMatrix));
		return *reinterpret_cast<NRMatrix *>(_c);
	}
	operator const NRMatrix&() const {
		g_assert(sizeof(_c) == sizeof(NRMatrix));
		return *reinterpret_cast<const NRMatrix *>(_c);
	}
	operator NRMatrix*() {
		g_assert(sizeof(_c) == sizeof(NRMatrix));
		return reinterpret_cast<NRMatrix *>(_c);
	}
	operator const NRMatrix*() const {
		g_assert(sizeof(_c) == sizeof(NRMatrix));
		return reinterpret_cast<const NRMatrix *>(_c);
	}

private:
	NR::Coord _c[6];
};

inline bool operator==(Matrix const &a, Matrix const &b)
{
	for(unsigned i = 0; i < 6; ++i) {
		if (a[i] != b[i]) {
			return false;
		}
	}
	return true;
}

inline bool operator!=(Matrix const &a, Matrix const &b)
{
	return !(a == b);
}

// Matrix factories
Matrix from_basis(const Point x_basis, const Point y_basis, const Point offset=Point(0,0));

Matrix identity();
//Matrix translate(const Point p);
//Matrix scale(const Point s);
//Matrix rotate(const NR::Coord angle);

double expansion(Matrix const &m);

Matrix operator*(Matrix const &a, Matrix const &b);

bool transform_equalp(Matrix const &m0, Matrix const &m1, NR::Coord const epsilon);
bool translate_equalp(Matrix const &m0, Matrix const &m1, NR::Coord const epsilon);
bool matrix_equalp(Matrix const &m0, Matrix const &m1, NR::Coord const epsilon);

void assert_close(Matrix const &a, Matrix const &b);

inline Point operator*(Point const &v, Matrix const &m)
{
	NR::Point const m_xform_col_0(m[0], m[2]);
	NR::Point const m_xform_col_1(m[1], m[3]);
	NR::Point const m_xlate(m[4], m[5]);
	return ( Point(dot(v, m_xform_col_0),
		       dot(v, m_xform_col_1))
		 + m_xlate );
/*
	return Point(v[X] * m[0]  +  v[Y] * m[2]  +  m[4],
		     v[X] * m[1]  +  v[Y] * m[3]  +  m[5]);
*/
}

inline Point &Point::operator*=(Matrix const &m)
{
	*this = *this * m;
	return *this;
}

inline Matrix operator*(Matrix const &m, translate const &t)
{
	Matrix ret(m);
	ret[4] += t[X];
	ret[5] += t[Y];
	assert_close( ret, m * Matrix(t) );
	return ret;
}

inline Matrix operator*(translate const &t, Matrix const &m)
{
	Matrix ret(m);
	ret[4] += m[0] * t[X] + m[2] * t[Y];
	ret[5] += m[1] * t[X] + m[3] * t[Y];
	assert_close( ret, Matrix(t) * m );
	return ret;
}

inline Matrix operator*(scale const &s, Matrix const &m)
{
	Matrix ret(m);
	ret[0] *= s[X];
	ret[1] *= s[X];
	ret[2] *= s[Y];
	ret[3] *= s[Y];
	assert_close( ret, Matrix(s) * m );
	return ret;
}

/** find the smallest rectangle that contains the transformed Rect r. */
//Rect operator*(const Matrix& nrm, const Rect &r);

};

#endif
