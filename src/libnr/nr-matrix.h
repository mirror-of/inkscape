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

#define nr_matrix_set_identity(m) (*(m) = NR_MATRIX_IDENTITY)

#define nr_matrix_test_identity(m,e) (!(m) || NR_MATRIX_DF_TEST_CLOSE (m, &NR_MATRIX_IDENTITY, e))

#define nr_matrix_test_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && NR_MATRIX_DF_TEST_CLOSE (m0, m1, e)))
#define nr_matrix_test_transform_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && NR_MATRIX_DF_TEST_TRANSFORM_CLOSE (m0, m1, e)))
#define nr_matrix_test_translate_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && NR_MATRIX_DF_TEST_TRANSLATE_CLOSE (m0, m1, e)))

NRMatrix *nr_matrix_d_from_f (NRMatrix *d, const NRMatrix *s);
NRMatrix *nr_matrix_f_from_d (NRMatrix *d, const NRMatrix *s);

NRMatrix *nr_matrix_invert (NRMatrix *d, const NRMatrix *m);

/* d,m0,m1 needn't be distinct in any of these multiply routines. */

NRMatrix *nr_matrix_multiply (NRMatrix *d, const NRMatrix *m0, const NRMatrix *m1);

NRMatrix *nr_matrix_set_translate (NRMatrix *m, const NR::Coord x, const NR::Coord y);

NRMatrix *nr_matrix_set_scale (NRMatrix *m, const NR::Coord sx, const NR::Coord sy);

NRMatrix *nr_matrix_set_rotate (NRMatrix *m, const NR::Coord theta);

#define NR_MATRIX_DF_TRANSFORM_X(m,x,y) ((m)->c[0] * (x) + (m)->c[2] * (y) + (m)->c[4])
#define NR_MATRIX_DF_TRANSFORM_Y(m,x,y) ((m)->c[1] * (x) + (m)->c[3] * (y) + (m)->c[5])

#define NR_MATRIX_DF_EXPANSION2(m) (fabs ((m)->c[0] * (m)->c[3] - (m)->c[1] * (m)->c[2]))
#define NR_MATRIX_DF_EXPANSION(m) (sqrt (NR_MATRIX_DF_EXPANSION2 (m)))

namespace NR{
class scale : public Point{
 public:
	Point operator *(const Point v) const {
		return Point(pt[0]*v.pt[0], pt[1]*v.pt[1]);
	}
};
 
class rotate : public Point{
 public:
	rotate(Coord theta);
	rotate(Point p) : Point(p) {}
};

class translate : public Point{};

inline Point operator *(const rotate r, const Point v) {
	return Point(r[0]* v[0] - r[1]*v[1], r[1]*v[0] + r[0]*v[1]);
}

inline Point operator *(const translate t, const Point v) {
	return t + v;
}

class Matrix{
 public:
/*
  c[] = | 0 2 | 4 |
        | 1 3 | 5 |

              x
  Points are  y  from the point of view of a matrix.
              1
*/
	NR::Coord c[6];

	Matrix() {
	}

	Matrix(const Matrix& m) {
		for(int i = 0; i < 6; i++)
			c[i] = m.c[i];
	}

	Matrix(const scale& sm) {
		for(int i = 0; i < 6; i++)
			c[i] = 0;
		c[0] = sm.pt[0];
		c[3] = sm.pt[1];
	}
	Matrix(const rotate& rm) {
		c[0] = rm.pt[0]; c[2] = -rm.pt[1]; c[4] = 0;
		c[1] = rm.pt[1]; c[3] =  rm.pt[0]; c[5] = 0;
	}
	Matrix(const translate& tm) {
		for(int i = 0; i < 4; i++)
			c[i] = 0;
		c[4] = tm[0];
		c[5] = tm[1];
	}
	Matrix(NRMatrix const *nr);
	
	bool test_identity() const;
	Matrix inverse() const;
	
	Point operator*(const Point v) const {
		// perhaps this should be done with a loop?  It's more
		// readable this way though.
		return Point(c[0]*v.pt[0] + c[2]*v.pt[1] + c[4],
			     c[1]*v.pt[0] + c[3]*v.pt[1] + c[5]);
	}

	void set_identity();
	
	// What do these do?  some kind of norm?
	NR::Coord Matrix::det() const;
	NR::Coord descrim2() const;
	NR::Coord descrim() const;
	
	// legacy
	void copyto(NRMatrix* nrm);
	operator NRMatrix*() const;
};

// Matrix factories
 Matrix from_basis(const Point x_basis, const Point y_basis, const Point offset=Point(0,0));

 Matrix identity();
 //Matrix translate(const Point p);
 //Matrix scale(const Point s);
 //Matrix rotate(const NR::Coord angle);

Matrix operator*(const Matrix a, const Matrix b);

bool transform_equalp(const Matrix m0, const Matrix m1, const NR::Coord epsilon);
bool translate_equalp(const Matrix m0, const Matrix m1, const NR::Coord epsilon);

inline Point operator*(const NRMatrix& nrm, const Point p) {
	 return Point(NR_MATRIX_DF_TRANSFORM_X(&nrm, p.pt[0], p.pt[1]),
		      NR_MATRIX_DF_TRANSFORM_Y(&nrm, p.pt[0], p.pt[1]));
 }
};

#endif
