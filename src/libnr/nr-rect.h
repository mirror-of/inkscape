#ifndef __NR_RECT_H__
#define __NR_RECT_H__

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

/* NULL rect is infinite */

#define nr_rect_d_set_empty(r) (*(r) = NR_RECT_D_EMPTY)
#define nr_rect_f_set_empty(r) (*(r) = NR_RECT_F_EMPTY)
#define nr_rect_l_set_empty(r) (*(r) = NR_RECT_L_EMPTY)
#define nr_rect_s_set_empty(r) (*(r) = NR_RECT_S_EMPTY)

#define nr_rect_d_test_empty(r) ((r) && NR_RECT_DFLS_TEST_EMPTY (r))
#define nr_rect_f_test_empty(r) ((r) && NR_RECT_DFLS_TEST_EMPTY (r))
#define nr_rect_l_test_empty(r) ((r) && NR_RECT_DFLS_TEST_EMPTY (r))
#define nr_rect_s_test_empty(r) ((r) && NR_RECT_DFLS_TEST_EMPTY (r))

#define nr_rect_d_test_intersect(r0,r1) \
	(!nr_rect_d_test_empty (r0) && !nr_rect_d_test_empty (r1) && \
	 !((r0) && (r1) && !NR_RECT_DFLS_TEST_INTERSECT (r0, r1)))
#define nr_rect_f_test_intersect(r0,r1) \
	(!nr_rect_f_test_empty (r0) && !nr_rect_f_test_empty (r1) && \
	 !((r0) && (r1) && !NR_RECT_DFLS_TEST_INTERSECT (r0, r1)))
#define nr_rect_l_test_intersect(r0,r1) \
	(!nr_rect_l_test_empty (r0) && !nr_rect_l_test_empty (r1) && \
	 !((r0) && (r1) && !NR_RECT_DFLS_TEST_INTERSECT (r0, r1)))
#define nr_rect_s_test_intersect(r0,r1) \
	(!nr_rect_s_test_empty (r0) && !nr_rect_s_test_empty (r1) && \
	 !((r0) && (r1) && !NR_RECT_DFLS_TEST_INTERSECT (r0, r1)))

#define nr_rect_d_point_d_test_inside(r,p) ((p) && (!(r) || (!NR_RECT_DF_TEST_EMPTY (r) && NR_RECT_DF_POINT_DF_TEST_INSIDE (r,p))))
#define nr_rect_f_point_f_test_inside(r,p) ((p) && (!(r) || (!NR_RECT_DF_TEST_EMPTY (r) && NR_RECT_DF_POINT_DF_TEST_INSIDE (r,p))))

/* NULL values are OK for r0 and r1, but not for d */

NRRectD *nr_rect_d_intersect (NRRectD *d, const NRRectD *r0, const NRRectD *r1);
NRRectF *nr_rect_f_intersect (NRRectF *d, const NRRectF *r0, const NRRectF *r1);
NRRectL *nr_rect_l_intersect (NRRectL *d, const NRRectL *r0, const NRRectL *r1);
NRRectS *nr_rect_s_intersect (NRRectS *d, const NRRectS *r0, const NRRectS *r1);

NRRectD *nr_rect_d_union (NRRectD *d, const NRRectD *r0, const NRRectD *r1);
NRRectF *nr_rect_f_union (NRRectF *d, const NRRectF *r0, const NRRectF *r1);
NRRectL *nr_rect_l_union (NRRectL *d, const NRRectL *r0, const NRRectL *r1);
NRRectS *nr_rect_s_union (NRRectS *d, const NRRectS *r0, const NRRectS *r1);

NRRectD *nr_rect_d_union_xy (NRRectD *d, double x, double y);
NRRectF *nr_rect_f_union_xy (NRRectF *d, float x, float y);
NRRectL *nr_rect_l_union_xy (NRRectL *d, NRLong x, NRLong y);
NRRectS *nr_rect_s_union_xy (NRRectS *d, NRShort x, NRShort y);

NRRectD *nr_rect_d_matrix_d_transform (NRRectD *d, NRRectD *s, NRMatrixD *m);
NRRectF *nr_rect_f_matrix_f_transform (NRRectF *d, NRRectF *s, NRMatrixF *m);

#endif
