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

#define nr_matrix_d_set_identity(m) (*(m) = NR_MATRIX_D_IDENTITY)
#define nr_matrix_f_set_identity(m) (*(m) = NR_MATRIX_F_IDENTITY)

#define nr_matrix_d_test_identity(m,e) (!(m) || NR_MATRIX_DF_TEST_CLOSE (m, &NR_MATRIX_D_IDENTITY, e))
#define nr_matrix_f_test_identity(m,e) (!(m) || NR_MATRIX_DF_TEST_CLOSE (m, &NR_MATRIX_F_IDENTITY, e))

#define nr_matrix_d_test_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && NR_MATRIX_DF_TEST_CLOSE (m0, m1, e)))
#define nr_matrix_f_test_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && NR_MATRIX_DF_TEST_CLOSE (m0, m1, e)))
#define nr_matrix_d_test_transform_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && NR_MATRIX_DF_TEST_TRANSFORM_CLOSE (m0, m1, e)))
#define nr_matrix_f_test_transform_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && NR_MATRIX_DF_TEST_TRANSFORM_CLOSE (m0, m1, e)))
#define nr_matrix_d_test_translate_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && NR_MATRIX_DF_TEST_TRANSLATE_CLOSE (m0, m1, e)))
#define nr_matrix_f_test_translate_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && NR_MATRIX_DF_TEST_TRANSLATE_CLOSE (m0, m1, e)))

NRMatrixD *nr_matrix_d_from_f (NRMatrixD *d, const NRMatrixF *s);
NRMatrixF *nr_matrix_f_from_d (NRMatrixF *d, const NRMatrixD *s);

NRMatrixD *nr_matrix_d_invert (NRMatrixD *d, const NRMatrixD *m);
NRMatrixF *nr_matrix_f_invert (NRMatrixF *d, const NRMatrixF *m);

NRMatrixD *nr_matrix_multiply_ddd (NRMatrixD *d, const NRMatrixD *m0, const NRMatrixD *m1);
NRMatrixF *nr_matrix_multiply_fff (NRMatrixF *d, const NRMatrixF *m0, const NRMatrixF *m1);

NRMatrixF *nr_matrix_multiply_fdd (NRMatrixF *d, const NRMatrixD *m0, const NRMatrixD *m1);
NRMatrixF *nr_matrix_multiply_fdf (NRMatrixF *d, const NRMatrixD *m0, const NRMatrixF *m1);
NRMatrixF *nr_matrix_multiply_ffd (NRMatrixF *d, const NRMatrixF *m0, const NRMatrixD *m1);

NRMatrixD *nr_matrix_multiply_ddf (NRMatrixD *d, const NRMatrixD *m0, const NRMatrixF *m1);
NRMatrixD *nr_matrix_multiply_dfd (NRMatrixD *d, const NRMatrixF *m0, const NRMatrixD *m1);

NRMatrixD *nr_matrix_d_set_translate (NRMatrixD *m, double x, double y);
NRMatrixF *nr_matrix_f_set_translate (NRMatrixF *m, float x, float y);

NRMatrixD *nr_matrix_d_set_scale (NRMatrixD *m, double sx, double sy);
NRMatrixF *nr_matrix_f_set_scale (NRMatrixF *m, float sx, float sy);

NRMatrixD *nr_matrix_d_set_rotate (NRMatrixD *m, double theta);
NRMatrixF *nr_matrix_f_set_rotate (NRMatrixF *m, float theta);

#define NR_MATRIX_DF_TRANSFORM_X(m,x,y) ((m)->c[0] * (x) + (m)->c[2] * (y) + (m)->c[4])
#define NR_MATRIX_DF_TRANSFORM_Y(m,x,y) ((m)->c[1] * (x) + (m)->c[3] * (y) + (m)->c[5])

#define NR_MATRIX_DF_EXPANSION2(m) (fabs ((m)->c[0] * (m)->c[3] - (m)->c[1] * (m)->c[2]))
#define NR_MATRIX_DF_EXPANSION(m) (sqrt (NR_MATRIX_DF_EXPANSION2 (m)))

#endif
