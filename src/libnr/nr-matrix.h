#ifndef __NR_MATRIX_H__
#define __NR_MATRIX_H__

/*
 * Definition of NRMatrix and NR::Matrix types.
 *
 * Note: operator functions (e.g. Matrix * Matrix etc.) are mostly in
 * libnr/nr-matrix-ops.h.  See end of file for discussion.
 *
 * Main authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>:
 *     Original NRMatrix definition and related macros.
 *
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>:
 *     NR::Matrix class version of the above.
 *
 * This code is in public domain.
 */

#include <glib.h>

#include <libnr/nr-macros.h>
#include <libnr/nr-point.h>
#include <libnr/nr-point-fns.h>
#include <libnr/nr-rotate.h>
#include <libnr/nr-scale.h>
#include <libnr/nr-translate.h>

struct NRMatrix {
	NR::Coord c[6];

	NR::Coord &operator[](int i) { return c[i]; }
	NR::Coord operator[](int i) const { return c[i]; }
};

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

	explicit Matrix(scale const &sm) {
		_c[0] = sm[X]; _c[1] = 0;
		_c[2] = 0;     _c[3] =  sm[Y];
		_c[4] = 0;     _c[5] = 0;
	}

	explicit Matrix(rotate const &r) {
		_c[0] =  r.vec[X]; _c[1] = r.vec[Y];
		_c[2] = -r.vec[Y]; _c[3] = r.vec[X];
		_c[4] = 0;         _c[5] = 0;
	}

	explicit Matrix(translate const &tm) {
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


extern void assert_close(Matrix const &a, Matrix const &b);

} /* namespace NR */


/*
 * Discussion of splitting up nr-matrix.h into lots of little files:
 *
 *   Advantages:
 *
 *    - Reducing amount of recompilation necessary when anything changes.
 *
 *    - Hopefully also reducing compilation time by reducing the number of inline
 *      function definitions encountered by the compiler for a given .o file.
 *      (No timing comparisons done yet.  On systems without much memory available
 *      for caching, this may be outweighed by additional I/O costs.)
 *
 *   Disadvantages:
 *
 *    - More #include lines necessary per file.  If a compile fails due to
 *      not having all the necessary #include lines, then the developer needs
 *      to spend some time working out what #include to add.
 */

#endif /* !__NR_MATRIX_H__ */


/*
  Local Variables:
  mode:c++
  c-file-style:"bsd"
  c-file-offsets:((innamespace . 0) (inline-open . 0))
  indent-tabs-mode:t
  fill-column:99
  End:
  vim: filetype=c++:noexpandtab :
*/
