#define __NR_GRADIENT_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-macros.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-pixops.h>
#include <libnr/nr-pixblock-pixel.h>
#include <libnr/nr-blit.h>
#include <libnr/nr-gradient.h>

#define noNR_USE_GENERIC_RENDERER

#ifndef hypot
#define hypot(a,b) sqrt ((a) * (a) + (b) * (b))
#endif

#define NRG_MASK (NR_GRADIENT_VECTOR_LENGTH - 1)
#define NRG_2MASK ((NR_GRADIENT_VECTOR_LENGTH << 1) - 1)

/* Radial */

static void nr_rgradient_render_block_symmetric (NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);
static void nr_rgradient_render_block_optimized (NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);
static void nr_rgradient_render_block_end (NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);
static void nr_rgradient_render_generic_symmetric (NRRGradientRenderer *rgr, NRPixBlock *pb);
static void nr_rgradient_render_generic_optimized (NRRGradientRenderer *rgr, NRPixBlock *pb);

NRRenderer *
nr_rgradient_renderer_setup (NRRGradientRenderer *rgr,
			     const unsigned char *cv,
			     unsigned int spread,
			     const NRMatrixF *gs2px,
			     float cx, float cy,
			     float fx, float fy,
			     float r)
{
	rgr->vector = cv;
	rgr->spread = spread;

	if (r < NR_EPSILON_F) {
		rgr->renderer.render = nr_rgradient_render_block_end;
	} else if (NR_DF_TEST_CLOSE (cx, fx, NR_EPSILON_F) &&
		   NR_DF_TEST_CLOSE (cy, fy, NR_EPSILON_F)) {
		rgr->renderer.render = nr_rgradient_render_block_symmetric;

		nr_matrix_f_invert (&rgr->px2gs, gs2px);
		rgr->px2gs.c[0] *= (NR_GRADIENT_VECTOR_LENGTH / r);
		rgr->px2gs.c[1] *= (NR_GRADIENT_VECTOR_LENGTH / r);
		rgr->px2gs.c[2] *= (NR_GRADIENT_VECTOR_LENGTH / r);
		rgr->px2gs.c[3] *= (NR_GRADIENT_VECTOR_LENGTH / r);
		rgr->px2gs.c[4] -= cx;
		rgr->px2gs.c[5] -= cy;
		rgr->px2gs.c[4] *= (NR_GRADIENT_VECTOR_LENGTH / r);
		rgr->px2gs.c[5] *= (NR_GRADIENT_VECTOR_LENGTH / r);

		rgr->cx = 0.0;
		rgr->cy = 0.0;
		rgr->fx = rgr->cx;
		rgr->fy = rgr->cy;
		rgr->r = 1.0;
	} else {
		NRMatrixF n2gs, n2px;
		double df;

		rgr->renderer.render = nr_rgradient_render_block_optimized;

		df = hypot (fx - cx, fy - cy);
		if (df >= r) {
			fx = cx + (fx - cx ) * r / (float) df;
			fy = cy + (fy - cy ) * r / (float) df;
		}

		n2gs.c[0] = cx - fx;
		n2gs.c[1] = cy - fy;
		n2gs.c[2] = cy - fy;
		n2gs.c[3] = fx - cx;
		n2gs.c[4] = fx;
		n2gs.c[5] = fy;

		nr_matrix_multiply_fff (&n2px, &n2gs, gs2px);
		nr_matrix_f_invert (&rgr->px2gs, &n2px);

		rgr->cx = 1.0;
		rgr->cy = 0.0;
		rgr->fx = 0.0;
		rgr->fy = 0.0;
		rgr->r = r / (float) hypot (fx - cx, fy - cy);
		rgr->C = 1.0F - rgr->r * rgr->r;
		/* INVARIANT: C < 0 */
		rgr->C = MIN (rgr->C, -NR_EPSILON_F);
	}

	return (NRRenderer *) rgr;
}

static void
nr_rgradient_render_block_symmetric (NRRenderer *r, NRPixBlock *pb, NRPixBlock *m)
{
	NRRGradientRenderer *rgr;
	int width, height;

	rgr = (NRRGradientRenderer *) r;

	width = pb->area.x1 - pb->area.x0;
	height = pb->area.y1 - pb->area.y0;

	nr_rgradient_render_generic_symmetric (rgr, pb);
}

static void
nr_rgradient_render_block_optimized (NRRenderer *r, NRPixBlock *pb, NRPixBlock *m)
{
	NRRGradientRenderer *rgr;
	int width, height;

	rgr = (NRRGradientRenderer *) r;

	width = pb->area.x1 - pb->area.x0;
	height = pb->area.y1 - pb->area.y0;

	nr_rgradient_render_generic_optimized (rgr, pb);
}

static void
nr_rgradient_render_block_end (NRRenderer *r, NRPixBlock *pb, NRPixBlock *m)
{
	const unsigned char *c;

	c = ((NRRGradientRenderer *) r)->vector + 4 * (NR_GRADIENT_VECTOR_LENGTH - 1);

	nr_blit_pixblock_mask_rgba32 (pb, m, (c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3]);
}

/*
 * The archetype is following
 *
 * gx gy - pixel coordinates
 * Px Py - coordinates, where Fx Fy - gx gy line intersects with circle
 *
 * (1)  (gx - fx) * (Py - fy) = (gy - fy) * (Px - fx)
 * (2)  (Px - cx) * (Px - cx) + (Py - cy) * (Py - cy) = r * r
 *
 * (3)   Py = (Px - fx) * (gy - fy) / (gx - fx) + fy
 * (4)  (gy - fy) / (gx - fx) = D
 * (5)   Py = D * Px - D * fx + fy
 *
 * (6)   D * fx - fy + cy = N
 * (7)   Px * Px - 2 * Px * cx + cx * cx + (D * Px) * (D * Px) - 2 * (D * Px) * N + N * N = r * r
 * (8)  (D * D + 1) * (Px * Px) - 2 * (cx + D * N) * Px + cx * cx + N * N = r * r
 *
 * (9)   A = D * D + 1
 * (10)  B = -2 * (cx + D * N)
 * (11)  C = cx * cx + N * N - r * r
 *
 * (12)  Px = (-B +- SQRT (B * B - 4 * A * C)) / 2 * A
 */

static void
nr_rgradient_render_generic_symmetric (NRRGradientRenderer *rgr, NRPixBlock *pb)
{
	double dx, dy;
	int x, y;
	unsigned char *d;
	const unsigned char *s;
	int idx;

	dx = rgr->px2gs.c[0];
	dy = rgr->px2gs.c[1];

	if (pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
		for (y = pb->area.y0; y < pb->area.y1; y++) {
			double gx, gy;
			d = NR_PIXBLOCK_PX (pb) + (y - pb->area.y0) * pb->rs;
			gx = rgr->px2gs.c[0] * pb->area.x0 + rgr->px2gs.c[2] * y + rgr->px2gs.c[4];
			gy = rgr->px2gs.c[1] * pb->area.x0 + rgr->px2gs.c[3] * y + rgr->px2gs.c[5];
			for (x = pb->area.x0; x < pb->area.x1; x++) {
				double pos;
				pos = hypot (gx, gy);
				if (rgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
					idx = ((int) pos) & NRG_2MASK;
					if (idx > NRG_MASK) idx = NRG_2MASK - idx;
				} else if (rgr->spread == NR_GRADIENT_SPREAD_REPEAT) {
					idx = ((int) pos) & NRG_MASK;
				} else {
					idx = CLAMP (((int) pos), 0, NRG_MASK);
				}
				s = rgr->vector + 4 * idx;
				d[0] = NR_COMPOSENPP (s[0], s[3], d[0], d[3]);
				d[1] = NR_COMPOSENPP (s[1], s[3], d[1], d[3]);
				d[2] = NR_COMPOSENPP (s[2], s[3], d[2], d[3]);
				d[3] = (65025 - (255 - s[3]) * (255 - d[3]) + 127) / 255;
				d += 4;
				gx += dx;
				gy += dy;
			}
		}
	} else if (pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8N) {
		for (y = pb->area.y0; y < pb->area.y1; y++) {
			double gx, gy;
			d = NR_PIXBLOCK_PX (pb) + (y - pb->area.y0) * pb->rs;
			gx = rgr->px2gs.c[0] * pb->area.x0 + rgr->px2gs.c[2] * y + rgr->px2gs.c[4];
			gy = rgr->px2gs.c[1] * pb->area.x0 + rgr->px2gs.c[3] * y + rgr->px2gs.c[5];
			for (x = pb->area.x0; x < pb->area.x1; x++) {
				double pos;
				pos = hypot (gx, gy);
				if (rgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
					idx = ((int) pos) & NRG_2MASK;
					if (idx > NRG_MASK) idx = NRG_2MASK - idx;
				} else if (rgr->spread == NR_GRADIENT_SPREAD_REPEAT) {
					idx = ((int) pos) & NRG_MASK;
				} else {
					idx = CLAMP (((int) pos), 0, NRG_MASK);
				}
				s = rgr->vector + 4 * idx;
				if (s[3] == 255) {
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
					d[3] = 255;
				} else if (s[3] != 0) {
					unsigned int ca;
					ca = 65025 - (255 - s[3]) * (255 - d[3]);
					d[0] = NR_COMPOSENNN_A7 (s[0], s[3], d[0], d[3], ca);
					d[1] = NR_COMPOSENNN_A7 (s[1], s[3], d[1], d[3], ca);
					d[2] = NR_COMPOSENNN_A7 (s[2], s[3], d[2], d[3], ca);
					d[3] = (ca + 127) / 255;
				}
				d += 4;
				gx += dx;
				gy += dy;
			}
		}
	} else {
		NRPixBlock spb;
		int bpp;

		nr_pixblock_setup_extern (&spb, NR_PIXBLOCK_MODE_R8G8B8A8N, 0, 0, NR_GRADIENT_VECTOR_LENGTH, 1,
					  (unsigned char *) rgr->vector,
					  4 * NR_GRADIENT_VECTOR_LENGTH,
					  0, 0);
		bpp = (pb->mode == NR_PIXBLOCK_MODE_A8) ? 1 : (pb->mode == NR_PIXBLOCK_MODE_R8G8B8) ? 3 : 4;

		for (y = pb->area.y0; y < pb->area.y1; y++) {
			double gx, gy;
			d = NR_PIXBLOCK_PX (pb) + (y - pb->area.y0) * pb->rs;
			gx = rgr->px2gs.c[0] * pb->area.x0 + rgr->px2gs.c[2] * y + rgr->px2gs.c[4];
			gy = rgr->px2gs.c[1] * pb->area.x0 + rgr->px2gs.c[3] * y + rgr->px2gs.c[5];
			for (x = pb->area.x0; x < pb->area.x1; x++) {
				double pos;
				pos = hypot (gx, gy);
				if (rgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
					idx = ((int) pos) & NRG_2MASK;
					if (idx > NRG_MASK) idx = NRG_2MASK - idx;
				} else if (rgr->spread == NR_GRADIENT_SPREAD_REPEAT) {
					idx = ((int) pos) & NRG_MASK;
				} else {
					idx = CLAMP (((int) pos), 0, NRG_MASK);
				}
				s = rgr->vector + 4 * idx;
				nr_compose_pixblock_pixblock_pixel (pb, d, &spb, s);
				d += bpp;
				gx += dx;
				gy += dy;
			}
		}

		nr_pixblock_release (&spb);
	}
}

static void
nr_rgradient_render_generic_optimized (NRRGradientRenderer *rgr, NRPixBlock *pb)
{
	int x, y;
	unsigned char *d;
	const unsigned char *s;
	int idx;
	int bpp;
	NRPixBlock spb;
	int x0, y0, x1, y1, rs;
	double r;

	x0 = pb->area.x0;
	y0 = pb->area.y0;
	x1 = pb->area.x1;
	y1 = pb->area.y1;
	rs = pb->rs;

	nr_pixblock_setup_extern (&spb, NR_PIXBLOCK_MODE_R8G8B8A8N, 0, 0, NR_GRADIENT_VECTOR_LENGTH, 1,
				  (unsigned char *) rgr->vector,
				  4 * NR_GRADIENT_VECTOR_LENGTH,
				  0, 0);
	bpp = (pb->mode == NR_PIXBLOCK_MODE_A8) ? 1 : (pb->mode == NR_PIXBLOCK_MODE_R8G8B8) ? 3 : 4;

	r = MAX (rgr->r, 1e-9);

	for (y = y0; y < y1; y++) {
		double gx, gy, dx, dy;
		d = NR_PIXBLOCK_PX (pb) + (y - y0) * rs;
		gx = rgr->px2gs.c[0] * x0 + rgr->px2gs.c[2] * y + rgr->px2gs.c[4];
		gy = rgr->px2gs.c[1] * x0 + rgr->px2gs.c[3] * y + rgr->px2gs.c[5];
		dx = rgr->px2gs.c[0];
		dy = rgr->px2gs.c[1];
		for (x = x0; x < x1; x++) {
			double pos;
			double gx2, gxy2, qgx2_4;
			double pxgx;

			gx2 = gx * gx;
			gxy2 = gx2 + gy * gy;
			qgx2_4 = gx2 - rgr->C * gxy2;
			/* INVARIANT: qgx2_4 >= 0.0 */
			/* qgx2_4 = MAX (qgx2_4, 0.0); */
			pxgx = gx + sqrt (qgx2_4);
			/* We can safely divide by 0 here */
			/* If we are sure pxgx cannot be -0 */
			pos = gxy2 / pxgx * NR_GRADIENT_VECTOR_LENGTH;
			if (pos < (1U << 31)) {
				if (rgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
					idx = ((int) pos) & (2 * NR_GRADIENT_VECTOR_LENGTH - 1);
					if (idx > NRG_MASK) idx = NRG_2MASK - idx;
				} else if (rgr->spread == NR_GRADIENT_SPREAD_REPEAT) {
					idx = ((int) pos) & (NR_GRADIENT_VECTOR_LENGTH - 1);
				} else {
					idx = CLAMP (((int) pos), 0, (NR_GRADIENT_VECTOR_LENGTH - 1));
				}
			} else {
				idx = NR_GRADIENT_VECTOR_LENGTH - 1;
			}
			s = rgr->vector + 4 * idx;
			nr_compose_pixblock_pixblock_pixel (pb, d, &spb, s);
			d += bpp;

			gx += dx;
			gy += dy;
		}
	}

	nr_pixblock_release (&spb);
}

