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

};
