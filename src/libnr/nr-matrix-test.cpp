#include "../utest/utest.h"
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>
using NR::Matrix;
using NR::X;
using NR::Y;

#include <libnr/nr-point-fns.cpp> // bad!
#include <libnr/nr-matrix-fns.cpp> // bad!

inline bool point_equalp(NR::Point const &a, NR::Point const &b)
{
	return ( NR_DF_TEST_CLOSE(a[X], b[X], 1e-5) &&
		 NR_DF_TEST_CLOSE(a[Y], b[Y], 1e-5) );
}

int main(int argc, char *argv[]) {
	int rc = EXIT_SUCCESS;

	Matrix const m_id(NR::identity());

	utest_start("NR::scale");

	NR::scale const sa(1.5, 2.0);
	UTEST_TEST("x,y constructor and operator[] const") {
		UTEST_ASSERT(sa[X] == 1.5);
		UTEST_ASSERT(sa[Y] == 2.0);
		UTEST_ASSERT(sa[0u] == 1.5);
		UTEST_ASSERT(sa[1u] == 2.0);
	}

	NR::Point const b(-2.0, 3.0);
	NR::scale const sb(b);

	UTEST_TEST("copy constructor, operator==, operator!=") {
		NR::scale const sa_copy(sa);
		UTEST_ASSERT( sa == sa_copy );
		UTEST_ASSERT(!( sa != sa_copy ));
		UTEST_ASSERT( sa != sb );
	}

	UTEST_TEST("operator=") {
		NR::scale sa_eq(sb);
		sa_eq = sa;
		UTEST_ASSERT( sa == sa_eq );
	}

	UTEST_TEST("point constructor") {
		UTEST_ASSERT(sb[X] == b[X]);
		UTEST_ASSERT(sb[Y] == b[Y]);
	}

	UTEST_TEST("operator*(Point, scale)") {
		NR::Point const ab( b * sa );
		UTEST_ASSERT( ab == NR::Point(-3.0, 6.0) );
	}

	UTEST_TEST("operator*(scale, scale)") {
		NR::scale const sab( sa * sb );
		UTEST_ASSERT( sab == NR::scale(-3.0, 6.0) );
	}

	UTEST_TEST("operator/(scale, scale)") {
		NR::scale const sa_b( sa / sb );
		UTEST_ASSERT( sa_b == NR::scale(-0.75, 2./3.) );
	}

	if (!utest_end()) {
		rc = EXIT_FAILURE;
	}


	utest_start("rotate");

	NR::rotate const r_id(0.0);
	NR::rotate const rot234(.234);
	UTEST_TEST("constructors, comparisons") {
		UTEST_ASSERT( r_id == r_id );
		UTEST_ASSERT( rot234 == rot234 );
		UTEST_ASSERT( rot234 != r_id );
		UTEST_ASSERT( r_id == NR::rotate(NR::Point(1.0, 0.0)) );
		UTEST_ASSERT( Matrix(r_id) == m_id );
		UTEST_ASSERT( Matrix(r_id).test_identity() );

		UTEST_ASSERT( rot234 == NR::rotate(NR::Point(cos(.234), sin(.234))) );
	}

	UTEST_TEST("operator=") {
		NR::rotate rot234_eq(r_id);
		rot234_eq = rot234;
		UTEST_ASSERT( rot234 == rot234_eq );
		UTEST_ASSERT( rot234_eq != r_id );
	}

	UTEST_TEST("inverse") {
		UTEST_ASSERT( r_id.inverse() == r_id );
		UTEST_ASSERT( rot234.inverse() == NR::rotate(-.234) );
	}

	NR::rotate const rot180(NR::Point(-1.0, 0.0));
	UTEST_TEST("operator*(Point, rotate)") {
		UTEST_ASSERT( b * r_id == b );
		UTEST_ASSERT( b * rot180 == -b );
		UTEST_ASSERT( b * rot234 == b * Matrix(rot234) );
	}

	UTEST_TEST("operator*(rotate, rotate)") {
		UTEST_ASSERT( r_id * r_id == r_id );
		UTEST_ASSERT( rot180 * rot180 == r_id );
		UTEST_ASSERT( rot234 * r_id == rot234 );
		UTEST_ASSERT( r_id * rot234 == rot234 );
		UTEST_ASSERT(matrix_equalp(rot234 * rot234.inverse(), r_id, 1e-14));
		UTEST_ASSERT(matrix_equalp(rot234.inverse() * rot234, r_id, 1e-14));
		UTEST_ASSERT(point_equalp(( NR::rotate(0.25) * NR::rotate(.5) ).vec, NR::rotate(.75).vec));
	}

	UTEST_TEST("operator/(rotate, rotate)") {
		UTEST_ASSERT( rot234 / r_id == rot234 );
		UTEST_ASSERT( rot234 / rot180 == rot234 * rot180 );
		UTEST_ASSERT(matrix_equalp(rot234 / rot234, r_id, 1e-14));
		UTEST_ASSERT(matrix_equalp(r_id / rot234, rot234.inverse(), 1e-14));
	}

	if (!utest_end()) {
		rc = EXIT_FAILURE;
	}


	utest_start("translate");

	NR::translate const tb(b);
	NR::translate const tc(-3.0, -2.0);
	UTEST_TEST("constructors, operator[]") {
		UTEST_ASSERT( tc[X] == -3.0 && tc[Y] == -2.0 );
		UTEST_ASSERT( tb[0] == b[X] && tb[1] == b[Y] );
	}

	UTEST_TEST("operator=") {
		NR::translate tb_eq(tc);
		tb_eq = tb;
		UTEST_ASSERT( tb == tb_eq );
		UTEST_ASSERT( tb_eq != tc );
	}

	NR::translate const tbc( tb * tc );
	UTEST_TEST("operator*(translate, translate)") {
		UTEST_ASSERT( tbc.offset == NR::Point(-5.0, 1.0) );
		UTEST_ASSERT( tbc.offset == ( tc * tb ).offset );
		UTEST_ASSERT( Matrix(tbc) == Matrix(tb) * Matrix(tc) );
	}

	UTEST_TEST("operator*(Point, translate)") {
		UTEST_ASSERT( tbc.offset == b * tc );
		UTEST_ASSERT( b * tc == b * Matrix(tc) );
	}

	NR::translate const t_id(0.0, 0.0);
	UTEST_TEST("identity") {
		UTEST_ASSERT( b * t_id == b );
		UTEST_ASSERT( Matrix(t_id) == m_id );
	}

	if (!utest_end()) {
		rc = EXIT_FAILURE;
	}


	utest_start("Matrix");

	Matrix const c16(1.0, 2.0,
			 3.0, 4.0,
			 5.0, 6.0);
	UTEST_TEST("basic constructors, operator=") {
		Matrix const c16_copy(c16);
		Matrix c16_eq(m_id);
		c16_eq = c16;
		for(unsigned i = 0; i < 6; ++i) {
			UTEST_ASSERT( c16[i] == 1.0 + i );
			UTEST_ASSERT( c16[i] == c16_copy[i] );
			UTEST_ASSERT( c16[i] == c16_eq[i] );
			UTEST_ASSERT( m_id[i] == double( i == 0 || i == 3 ) );
		}
	}

	UTEST_TEST("scale constructor") {
		NR::scale const s(2.0, 3.0);
		NR::Matrix const ms(s);
		NR::Point const p(5.0, 7.0);
		UTEST_ASSERT( p * s == NR::Point(10.0, 21.0) );
		UTEST_ASSERT( p * ms == NR::Point(10.0, 21.0) );
	}

	UTEST_TEST("rotate constructor") {
		NR::rotate const r86(NR::Point(.8, .6));
		NR::Matrix const mr86(r86);
		NR::Point const p0(1.0, 0.0);
		NR::Point const p90(0.0, 1.0);
		UTEST_ASSERT( p0 * r86 == NR::Point(.8, .6) );
		UTEST_ASSERT( p0 * mr86 == NR::Point(.8, .6) );
		UTEST_ASSERT( p90 * r86 == NR::Point(-.6, .8) );
		UTEST_ASSERT( p90 * mr86 == NR::Point(-.6, .8) );
		UTEST_ASSERT(matrix_equalp(Matrix( r86 * r86 ),
					   mr86 * mr86,
					   1e-14));
	}

	NR::translate const t23(2.0, 3.0);
	UTEST_TEST("translate constructor") {
		NR::Matrix const mt23(t23);
		UTEST_ASSERT( b * t23 == b * mt23 );
	}

	NR::scale const s_id(1.0, 1.0);
	UTEST_TEST("test_identity") {
		UTEST_ASSERT(m_id.test_identity());
		UTEST_ASSERT(Matrix(t_id).test_identity());
		UTEST_ASSERT(!(Matrix(tb).test_identity()));
		UTEST_ASSERT(Matrix(r_id).test_identity());
		UTEST_ASSERT(!(Matrix(rot180).test_identity()));
		UTEST_ASSERT(Matrix(s_id).test_identity());
		UTEST_ASSERT(!(Matrix(NR::scale(1.0, 0.0)).test_identity()));
		UTEST_ASSERT(!(Matrix(NR::scale(0.0, 1.0)).test_identity()));
		UTEST_ASSERT(!(Matrix(NR::scale(1.0, -1.0)).test_identity()));
		UTEST_ASSERT(!(Matrix(NR::scale(-1.0, -1.0)).test_identity()));
	}

	UTEST_TEST("inverse") {
		UTEST_ASSERT( m_id.inverse() == m_id );
		UTEST_ASSERT( Matrix(t23).inverse() == Matrix(NR::translate(-2.0, -3.0)) );
		NR::scale const s2(-4.0, 2.0);
		NR::scale const sp5(-.25, .5);
		UTEST_ASSERT( Matrix(s2).inverse() == Matrix(sp5) );
	}

	UTEST_TEST("elliptic quadratic form") {
		NR::Matrix const aff(1.0, 1.0,
							 0.0, 1.0,
							 5.0, 6.0);
		NR::Matrix const invaff = aff.inverse();
		UTEST_ASSERT( invaff[1] == -1.0 );
		
		NR::Matrix ef = elliptic_quadratic_form(invaff);
		for(int i = 0; i < 3; i++)
			g_print("%f %f\n", ef[i*2], ef[i*2+1]);
		
		
	}
	if (!utest_end()) {
		rc = EXIT_FAILURE;
	}

	return rc;
}
