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

/* Currently uses single-precision intermediate results (depending on compiler,
   optimization settings etc.). */
NRMatrix *nr_matrix_set_translate (NRMatrix *m, double x, double y);

NRMatrix *nr_matrix_set_scale (NRMatrix *m, double sx, double sy);

NRMatrix *nr_matrix_set_rotate (NRMatrix *m, double theta);

#define NR_MATRIX_DF_TRANSFORM_X(m,x,y) ((m)->c[0] * (x) + (m)->c[2] * (y) + (m)->c[4])
#define NR_MATRIX_DF_TRANSFORM_Y(m,x,y) ((m)->c[1] * (x) + (m)->c[3] * (y) + (m)->c[5])

#define NR_MATRIX_DF_EXPANSION2(m) (fabs ((m)->c[0] * (m)->c[3] - (m)->c[1] * (m)->c[2]))
#define NR_MATRIX_DF_EXPANSION(m) (sqrt (NR_MATRIX_DF_EXPANSION2 (m)))

#endif
