#ifndef __NR_MATRIX_FNS_H__
#define __NR_MATRIX_FNS_H__

#include "nr-matrix.h"

namespace NR{

Matrix elliptic_quadratic_form(Matrix const &m);
/** Given a matrix m such that unit_circle = m*x, this returns the
 * quadratic form x*A*x = 1. */

class Eigen{
/** Given a matrix (ignoring the translation) this returns the eigen
 * values and vectors. */
 public:
	Point vectors[2];
	Point values;
	Eigen(Matrix const &m);
};

// Matrix factories
Matrix from_basis(const Point x_basis, const Point y_basis, const Point offset=Point(0,0));

Matrix identity();
//Matrix translate(const Point p);
//Matrix scale(const Point s);
//Matrix rotate(const NR::Coord angle);

double expansion(Matrix const &m);

bool transform_equalp(Matrix const &m0, Matrix const &m1, NR::Coord const epsilon);
bool translate_equalp(Matrix const &m0, Matrix const &m1, NR::Coord const epsilon);
bool matrix_equalp(Matrix const &m0, Matrix const &m1, NR::Coord const epsilon);

/** find the smallest rectangle that contains the transformed Rect r. */
//Rect operator*(const Matrix& nrm, const Rect &r);

};

#endif
