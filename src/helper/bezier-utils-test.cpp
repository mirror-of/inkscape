#include "../utest/utest.h"
#include <glib.h>

/* mental disclaims all responsibility for this evil idea for testing
   static functions.  The main disadvantages are that we retain the
   #define's and `using' directives of the included file. */
#include "bezier-utils.cpp"

using NR::Point;

static bool range_approx_equal(double const a[], double const b[], unsigned len);

/* (Returns false if NaN encountered.) */
template<class T>
static bool range_equal(T const a[], T const b[], unsigned len) {
	for(unsigned i = 0; i < len; ++i) {
		if(a[i] != b[i]) {
			return false;
		}
	}
	return true;
}

static inline double square(double x) {
	return x * x;
}

int main(int argc, char *argv[]) {
	utest_start("bezier-utils.cpp");

	UTEST_TEST("copy_without_nans_or_adjacent_duplicates") {
		NRPoint const src[] = {
			NRPoint(Point(2., 3.)),
			NRPoint(Point(2., 3.)),
			NRPoint(Point(0., 0.)),
			NRPoint(Point(2., 3.)),
			NRPoint(Point(2., 3.)),
			NRPoint(Point(1., 9.)),
			NRPoint(Point(1., 9.))
		};
		Point const exp_dest[] = {
			Point(2., 3.),
			Point(0., 0.),
			Point(2., 3.),
			Point(1., 9.)
		};
		g_assert(sizeof(src)/sizeof(*src) == 7);
		Point dest[7];
		struct tst {
			unsigned src_ix0;
			unsigned src_len;
			unsigned exp_dest_ix0;
			unsigned exp_dest_len;
		} const test_data[] = {
			/* src start ix, src len, exp_dest start ix, exp dest len */
			{0, 0, 0, 0},
			{2, 1, 1, 1},
			{0, 1, 0, 1},
			{0, 2, 0, 1},
			{0, 3, 0, 2},
			{1, 3, 0, 3},
			{0, 5, 0, 3},
			{0, 6, 0, 4},
			{0, 7, 0, 4}
		};
		for(unsigned i = 0 ; i < sizeof(test_data)/sizeof(test_data[0]) ; ++i) {
			tst const &t = test_data[i];
			UTEST_ASSERT( t.exp_dest_len
				      == copy_without_nans_or_adjacent_duplicates(src + t.src_ix0,
										  t.src_len,
										  dest) );
			UTEST_ASSERT(range_equal(dest,
						 exp_dest + t.exp_dest_ix0,
						 t.exp_dest_len));
		}
	}

	UTEST_TEST("bezier_pt(1)") {
		Point const a[] = {Point(2.0, 4.0),
				   Point(1.0, 8.0)};
		UTEST_ASSERT(bezier_pt(1, a, 0.0) == a[0]);
		UTEST_ASSERT(bezier_pt(1, a, 1.0) == a[1]);
		UTEST_ASSERT(bezier_pt(1, a, 0.5) == Point(1.5, 6.0));
		double const t[] = {0.5, 0.25, 0.3, 0.6};
		for(unsigned i = 0; i < sizeof(t)/sizeof(*t); ++i) {
			double const ti = t[i], si = 1.0 - ti;
			UTEST_ASSERT(bezier_pt(1, a, ti) == si * a[0] + ti * a[1]);
		}
	}

	UTEST_TEST("bezier_pt(2)") {
		Point const b[] = {Point(1.0, 2.0),
				   Point(8.0, 4.0),
				   Point(3.0, 1.0)};
		UTEST_ASSERT(bezier_pt(2, b, 0.0) == b[0]);
		UTEST_ASSERT(bezier_pt(2, b, 1.0) == b[2]);
		UTEST_ASSERT(bezier_pt(2, b, 0.5) == Point(5.0, 2.75));
		double const t[] = {0.5, 0.25, 0.3, 0.6};
		for(unsigned i = 0; i < sizeof(t)/sizeof(*t); ++i) {
			double const ti = t[i], si = 1.0 - ti;
			UTEST_ASSERT(bezier_pt(2, b, ti) == si*si * b[0] + 2*si*ti * b[1] + ti*ti * b[2]);
		}
	}

	Point const c[] = {Point(1.0, 2.0),
			   Point(8.0, 4.0),
			   Point(3.0, 1.0),
			   Point(-2.0, -4.0)};
	UTEST_TEST("bezier_pt(3)") {
		UTEST_ASSERT(bezier_pt(3, c, 0.0) == c[0]);
		UTEST_ASSERT(bezier_pt(3, c, 1.0) == c[3]);
		UTEST_ASSERT(bezier_pt(3, c, 0.5) == Point(4.0, 13.0/8.0));
		double const t[] = {0.5, 0.25, 0.3, 0.6};
		for(unsigned i = 0; i < sizeof(t)/sizeof(*t); ++i) {
			double const ti = t[i], si = 1.0 - ti;
			UTEST_ASSERT( LInfty( bezier_pt(3, c, ti)
					      - ( si*si*si * c[0] +
						  3*si*si*ti * c[1] +
						  3*si*ti*ti * c[2] +
						  ti*ti*ti * c[3] ) )
				      < 1e-4 );
		}
	}

	struct Err_tst {
		Point pt;
		double u;
		double err;
	} const err_tst[] = {
		{c[0], 0.0, 0.0},
		{Point(4.0, 13.0/8.0), 0.5, 0.0},
		{Point(4.0, 2.0), 0.5, 9.0/64.0},
		{Point(3.0, 2.0), 0.5, 1.0 + 9.0/64.0},
		{Point(6.0, 2.0), 0.5, 4.0 + 9.0/64.0},
		{c[3], 1.0, 0.0},
	};
	UTEST_TEST("compute_error") {
		for(unsigned i = 0; i < G_N_ELEMENTS(err_tst); ++i) {
			Err_tst const &t = err_tst[i];
			UTEST_ASSERT( compute_error(t.pt, t.u, c) == t.err );
		}
	}

	UTEST_TEST("compute_max_error") {
		Point d[G_N_ELEMENTS(err_tst)];
		double u[G_N_ELEMENTS(err_tst)];
		for(unsigned i = 0; i < G_N_ELEMENTS(err_tst); ++i) {
			Err_tst const &t = err_tst[i];
			d[i] = t.pt;
			u[i] = t.u;
		}
		g_assert( G_N_ELEMENTS(u) == G_N_ELEMENTS(d) );
		unsigned max_ix = ~0u;

		UTEST_ASSERT( err_tst[4].err == compute_max_error(d, u, G_N_ELEMENTS(d), c, &max_ix) );
		UTEST_ASSERT( max_ix == 4 );
	}

	UTEST_TEST("chord_length_parameterize") {
		/* n == 2 */
		{
			Point const d[] = {Point(2.9415, -5.8149),
					   Point(23.021, 4.9814)};
			double u[G_N_ELEMENTS(d)];
			double const exp_u[] = {0.0, 1.0};
			g_assert( G_N_ELEMENTS(u) == G_N_ELEMENTS(exp_u) );
			chord_length_parameterize(d, u, G_N_ELEMENTS(d));
			UTEST_ASSERT(range_equal(u, exp_u, G_N_ELEMENTS(exp_u)));
		}

		/* Straight line. */
		{
			double const exp_u[] = {0.0, 0.1829, 0.2105, 0.2105, 0.619, 0.815, 0.999, 1.0};
			unsigned const n = G_N_ELEMENTS(exp_u);
			Point d[n];
			double u[n];
			Point const a(-23.985, 4.915), b(4.9127, 5.203);
			for(unsigned i = 0; i < n; ++i) {
				double bi = exp_u[i], ai = 1.0 - bi;
				d[i] = ai * a  +  bi * b;
			}
			chord_length_parameterize(d, u, n);
			UTEST_ASSERT(range_approx_equal(u, exp_u, n));
		}
	}

	UTEST_TEST("sp_bezier_fit_cubic") {
		/* Feed it some points that can be fit exactly with a
		   single bezier segment, and see how well it
		   manages. */
		Point const src_b[4] = {Point(5., -3.),
					Point(-3., 8.),
					Point(4., 2.),
					Point(3., -2.)};
		double const t[] = {0.0, .01, .02, .03, .05, .09, .18, .25, .37, .44,
				    .51, .69, .81, .91, .93, .97, .98, .99, 1.0};
		unsigned const n = G_N_ELEMENTS(t);
		Point d[n];
		NRPoint cd[n];
		for(unsigned i = 0; i < n; ++i) {
			d[i] = bezier_pt(3, src_b, t[i]);
			cd[i] = NRPoint(d[i]);
		}

		NRPoint cest_b[4];
		gint const succ = sp_bezier_fit_cubic(cest_b, cd, n, square(1.2));
		UTEST_ASSERT(succ >= 0);
		printf("succ=%d:", succ);
		Point est_b[4];
		for(unsigned i = 0; i < 4; ++i) {
			est_b[i] = Point(cest_b[i]);
			printf(" (%f, %f)", est_b[i][0], est_b[i][1]);
		}
		double sum_errsq = 0.0;
		for(unsigned i = 0; i < n; ++i) {
			/* We're being unfair here in using our t[] rather than
			   best t[] for cest_b. */
			Point const fit_pt = bezier_pt(3, est_b, t[i]);
			Point const diff = fit_pt - d[i];
			sum_errsq += dot(diff, diff);
		}
		printf(" gives RMS error %g\n", sqrt(sum_errsq / n));
	}

	return !utest_end();
}

/* (Returns false if NaN encountered.) */
static bool range_approx_equal(double const a[], double const b[], unsigned len) {
	for(unsigned i = 0; i < len; ++i) {
		if(!( fabs( a[i] - b[i] ) < 1e-4 )) {
			return false;
		}
	}
	return true;
}
