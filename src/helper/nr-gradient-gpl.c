#define __NR_GRADIENT_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-macros.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-pixops.h>
#include <libnr/nr-pixblock-pixel.h>
#include <libnr/nr-blit.h>

#include "nr-gradient-gpl.h"

#define noNR_USE_GENERIC_RENDERER

#ifndef hypot
#define hypot(a,b) sqrt ((a) * (a) + (b) * (b))
#endif

#define NRG_MASK (NR_GRADIENT_VECTOR_LENGTH - 1)
#define NRG_2MASK ((NR_GRADIENT_VECTOR_LENGTH << 1) - 1)

static void nr_lgradient_render_block (NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);
static void nr_lgradient_render_R8G8B8A8N_EMPTY (NRLGradientRenderer *lgr, unsigned char *px, int x0, int y0, int width, int height, int rs);
static void nr_lgradient_render_R8G8B8A8N (NRLGradientRenderer *lgr, unsigned char *px, int x0, int y0, int width, int height, int rs);
static void nr_lgradient_render_R8G8B8 (NRLGradientRenderer *lgr, unsigned char *px, int x0, int y0, int width, int height, int rs);
static void nr_lgradient_render_generic (NRLGradientRenderer *lgr, NRPixBlock *pb);

NRRenderer *
nr_lgradient_renderer_setup (NRLGradientRenderer *lgr,
			     const unsigned char *cv, 
			     unsigned int spread, 
			     const NRMatrixF *gs2px,
			     float x0, float y0,
			     float x1, float y1)
{
	NRMatrixF n2gs, n2px, px2n;

	lgr->renderer.render = nr_lgradient_render_block;

	lgr->vector = cv;
	lgr->spread = spread;

	n2gs.c[0] = x1 - x0;
	n2gs.c[1] = y1 - y0;
	n2gs.c[2] = y1 - y0;
	n2gs.c[3] = x0 - x1;
	n2gs.c[4] = x0;
	n2gs.c[5] = y0;

	nr_matrix_multiply_fff (&n2px, &n2gs, gs2px);
	nr_matrix_f_invert (&px2n, &n2px);

	lgr->x0 = (int) (n2px.c[4] + 0.5);
	lgr->y0 = (int) (n2px.c[5] + 0.5);
	lgr->dx = px2n.c[0] * NR_GRADIENT_VECTOR_LENGTH;
	lgr->dy = px2n.c[2] * NR_GRADIENT_VECTOR_LENGTH;

	return (NRRenderer *) lgr;
}

static void
nr_lgradient_render_block (NRRenderer *r, NRPixBlock *pb, NRPixBlock *m)
{
	NRLGradientRenderer *lgr;
	int width, height;

	lgr = (NRLGradientRenderer *) r;

	width = pb->area.x1 - pb->area.x0;
	height = pb->area.y1 - pb->area.y0;

#ifdef NR_USE_GENERIC_RENDERER
	nr_lgradient_render_generic (lgr, pb);
#else
	if (pb->empty) {
		switch (pb->mode) {
		case NR_PIXBLOCK_MODE_A8:
			nr_lgradient_render_generic (lgr, pb);
			break;
		case NR_PIXBLOCK_MODE_R8G8B8:
			nr_lgradient_render_generic (lgr, pb);
			break;
		case NR_PIXBLOCK_MODE_R8G8B8A8N:
			nr_lgradient_render_R8G8B8A8N_EMPTY (lgr, NR_PIXBLOCK_PX (pb), pb->area.x0, pb->area.y0, width, height, pb->rs);
			break;
		case NR_PIXBLOCK_MODE_R8G8B8A8P:
			nr_lgradient_render_generic (lgr, pb);
			break;
		default:
			break;
		}
	} else {
		switch (pb->mode) {
		case NR_PIXBLOCK_MODE_A8:
			nr_lgradient_render_generic (lgr, pb);
			break;
		case NR_PIXBLOCK_MODE_R8G8B8:
			nr_lgradient_render_R8G8B8 (lgr, NR_PIXBLOCK_PX (pb), pb->area.x0, pb->area.y0, width, height, pb->rs);
			break;
		case NR_PIXBLOCK_MODE_R8G8B8A8N:
			nr_lgradient_render_R8G8B8A8N (lgr, NR_PIXBLOCK_PX (pb), pb->area.x0, pb->area.y0, width, height, pb->rs);
			break;
		case NR_PIXBLOCK_MODE_R8G8B8A8P:
			nr_lgradient_render_generic (lgr, pb);
			break;
		default:
			break;
		}
	}
#endif
}

static void
nr_lgradient_render_R8G8B8A8N_EMPTY (NRLGradientRenderer *lgr, unsigned char *px, int x0, int y0, int width, int height, int rs)
{
	int x, y;
	double pos;

	for (y = 0; y < height; y++) {
		const unsigned char *s;
		unsigned char *d;
		int idx;
		d = px + y * rs;
		pos = (y + y0 - lgr->y0) * lgr->dy + (0 + x0 - lgr->x0) * lgr->dx;
		if (lgr->spread == NR_GRADIENT_SPREAD_PAD) {
			for (x = 0; x < width; x++) {
				idx = (int) pos;
				idx = CLAMP (idx, 0, NRG_MASK);
				s = lgr->vector + 4 * idx;
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
				d += 4;
				pos += lgr->dx;
			}
		} else if (lgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
			for (x = 0; x < width; x++) {
				idx = (int) pos;
				idx = idx & NRG_2MASK;
				if (idx > NRG_MASK) idx = NRG_2MASK - idx;
				s = lgr->vector + 4 * idx;
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
				d += 4;
				pos += lgr->dx;
			}
		} else {
			for (x = 0; x < width; x++) {
				idx = (int) pos;
				idx = idx & NRG_MASK;
				s = lgr->vector + 4 * idx;
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = s[3];
				d += 4;
				pos += lgr->dx;
			}
		}
	}
}

static void
nr_lgradient_render_R8G8B8A8N (NRLGradientRenderer *lgr, unsigned char *px, int x0, int y0, int width, int height, int rs)
{
	int x, y;
	unsigned char *d;
	double pos;

	for (y = 0; y < height; y++) {
		d = px + y * rs;
		pos = (y + y0 - lgr->y0) * lgr->dy + (0 + x0 - lgr->x0) * lgr->dx;
		for (x = 0; x < width; x++) {
			int ip, idx;
			unsigned int ca;
			const unsigned char *s;
			ip = (int) pos;
			switch (lgr->spread) {
			case NR_GRADIENT_SPREAD_PAD:
				idx = CLAMP (ip, 0, NRG_MASK);
				break;
			case NR_GRADIENT_SPREAD_REFLECT:
				idx = ip & NRG_2MASK;
				if (idx > NRG_MASK) idx = NRG_2MASK - idx;
				break;
			case NR_GRADIENT_SPREAD_REPEAT:
				idx = ip & NRG_MASK;
				break;
			default:
				idx = 0;
				break;
			}
			/* Full composition */
			s = lgr->vector + 4 * idx;
			if (s[3] == 255) {
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = 255;
			} else if (s[3] != 0) {
				ca = 65025 - (255 - s[3]) * (255 - d[3]);
				d[0] = NR_COMPOSENNN_A7 (s[0], s[3], d[0], d[3], ca);
				d[1] = NR_COMPOSENNN_A7 (s[1], s[3], d[1], d[3], ca);
				d[2] = NR_COMPOSENNN_A7 (s[2], s[3], d[2], d[3], ca);
				d[3] = (ca + 127) / 255;
			}
			d += 4;
			pos += lgr->dx;
		}
	}
}

static void
nr_lgradient_render_R8G8B8 (NRLGradientRenderer *lgr, unsigned char *px, int x0, int y0, int width, int height, int rs)
{
	int x, y;
	unsigned char *d;
	double pos;

	for (y = 0; y < height; y++) {
		d = px + y * rs;
		pos = (y + y0 - lgr->y0) * lgr->dy + (0 + x0 - lgr->x0) * lgr->dx;
		for (x = 0; x < width; x++) {
			int ip, idx;
			const unsigned char *s;
			ip = (int) pos;
			switch (lgr->spread) {
			case NR_GRADIENT_SPREAD_PAD:
				idx = CLAMP (ip, 0, NRG_MASK);
				break;
			case NR_GRADIENT_SPREAD_REFLECT:
				idx = ip & NRG_2MASK;
				if (idx > NRG_MASK) idx = NRG_2MASK - idx;
				break;
			case NR_GRADIENT_SPREAD_REPEAT:
				idx = ip & NRG_MASK;
				break;
			default:
				idx = 0;
				break;
			}
			/* Full composition */
			s = lgr->vector + 4 * idx;
			d[0] = NR_COMPOSEN11 (s[0], s[3], d[0]);
			d[1] = NR_COMPOSEN11 (s[1], s[3], d[1]);
			d[2] = NR_COMPOSEN11 (s[2], s[3], d[2]);
			d += 3;
			pos += lgr->dx;
		}
	}
}

static void
nr_lgradient_render_generic (NRLGradientRenderer *lgr, NRPixBlock *pb)
{
	int x, y;
	unsigned char *d;
	double pos;
	int bpp;
	NRPixBlock spb;
	int x0, y0, width, height, rs;

	x0 = pb->area.x0;
	y0 = pb->area.y0;
	width = pb->area.x1 - pb->area.x0;
	height = pb->area.y1 - pb->area.y0;
	rs = pb->rs;

	nr_pixblock_setup_extern (&spb, NR_PIXBLOCK_MODE_R8G8B8A8N, 0, 0, NR_GRADIENT_VECTOR_LENGTH, 1,
				  (unsigned char *) lgr->vector,
				  4 * NR_GRADIENT_VECTOR_LENGTH,
				  0, 0);
	bpp = (pb->mode == NR_PIXBLOCK_MODE_A8) ? 1 : (pb->mode == NR_PIXBLOCK_MODE_R8G8B8) ? 3 : 4;

	for (y = 0; y < height; y++) {
		d = NR_PIXBLOCK_PX (pb) + y * rs;
		pos = (y + y0 - lgr->y0) * lgr->dy + (0 + x0 - lgr->x0) * lgr->dx;
		for (x = 0; x < width; x++) {
			int ip, idx;
			const unsigned char *s;
			ip = (int) pos;
			switch (lgr->spread) {
			case NR_GRADIENT_SPREAD_PAD:
				idx = CLAMP (ip, 0, NRG_MASK);
				break;
			case NR_GRADIENT_SPREAD_REFLECT:
				idx = ip & NRG_2MASK;
				if (idx > NRG_MASK) idx = NRG_2MASK - idx;
				break;
			case NR_GRADIENT_SPREAD_REPEAT:
				idx = ip & NRG_MASK;
				break;
			default:
				idx = 0;
				break;
			}
			s = lgr->vector + 4 * idx;
			nr_compose_pixblock_pixblock_pixel (pb, d, &spb, s);
			d += bpp;
			pos += lgr->dx;
		}
	}

	nr_pixblock_release (&spb);
}

