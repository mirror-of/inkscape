#define __NR_ARENA_IMAGE_C__

/*
 * RGBA display list system for sodipodi
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-compose-transform.h>
#include "nr-arena-image.h"

int nr_arena_image_x_sample = 1;
int nr_arena_image_y_sample = 1;

/*
 * NRArenaCanvasImage
 *
 */

static void nr_arena_image_class_init (NRArenaImageClass *klass);
static void nr_arena_image_init (NRArenaImage *image);
static void nr_arena_image_finalize (NRObject *object);

static unsigned int nr_arena_image_update (NRArenaItem *item, NRRectL *area, NRGC *gc, unsigned int state, unsigned int reset);
static unsigned int nr_arena_image_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags);
static NRArenaItem *nr_arena_image_pick (NRArenaItem *item, double x, double y, double delta, unsigned int sticky);

static NRArenaItemClass *parent_class;

NRType
nr_arena_image_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_ARENA_ITEM,
						"NRArenaImage",
						sizeof (NRArenaImageClass),
						sizeof (NRArenaImage),
						(void (*) (NRObjectClass *)) nr_arena_image_class_init,
						(void (*) (NRObject *)) nr_arena_image_init);
	}
	return type;
}

static void
nr_arena_image_class_init (NRArenaImageClass *klass)
{
	NRObjectClass *object_class;
	NRArenaItemClass *item_class;

	object_class = (NRObjectClass *) klass;
	item_class = (NRArenaItemClass *) klass;

	parent_class = (NRArenaItemClass *) ((NRObjectClass *) klass)->parent;

	object_class->finalize = nr_arena_image_finalize;

	item_class->update = nr_arena_image_update;
	item_class->render = nr_arena_image_render;
	item_class->pick = nr_arena_image_pick;
}

static void
nr_arena_image_init (NRArenaImage *image)
{
	image->width = 256.0;
	image->height = 256.0;

	nr_matrix_f_set_identity (&image->grid2px);
}

static void
nr_arena_image_finalize (NRObject *object)
{
	NRArenaImage *image;

	image = NR_ARENA_IMAGE (object);

	image->px = NULL;

	((NRObjectClass *) parent_class)->finalize (object);
}

static unsigned int
nr_arena_image_update (NRArenaItem *item, NRRectL *area, NRGC *gc, unsigned int state, unsigned int reset)
{
	NRArenaImage *image;
	double hscale, vscale;
	NRMatrixD grid2px;

	image = NR_ARENA_IMAGE (item);

	/* Request render old */
	nr_arena_item_request_render (item);

	/* Copy affine */
	nr_matrix_d_invert (&grid2px, &gc->transform);
	if (image->px) {
		hscale = image->pxw / image->width;
		vscale = image->pxh / image->height;
	} else {
		hscale = 1.0;
		vscale = 1.0;
	}

	image->grid2px.c[0] = grid2px.c[0] * hscale;
	image->grid2px.c[2] = grid2px.c[2] * hscale;
	image->grid2px.c[4] = grid2px.c[4] * hscale;
	image->grid2px.c[1] = grid2px.c[1] * vscale;
	image->grid2px.c[3] = grid2px.c[3] * vscale;
	image->grid2px.c[5] = grid2px.c[5] * vscale;

	image->grid2px.c[4] -= image->x * hscale;
	image->grid2px.c[5] -= image->y * vscale;

	/* Calculate bbox */
	if (image->px) {
		NRRectD bbox;

		bbox.x0 = image->x;
		bbox.y0 = image->y;
		bbox.x1 = image->x + image->width;
		bbox.y1 = image->y + image->height;
		nr_rect_d_matrix_d_transform (&bbox, &bbox, &gc->transform);

		item->bbox.x0 = (int) floor (bbox.x0);
		item->bbox.y0 = (int) floor (bbox.y0);
		item->bbox.x1 = (int) ceil (bbox.x1);
		item->bbox.y1 = (int) ceil (bbox.y1);
	} else {
		item->bbox.x0 = (int) gc->transform.c[4];
		item->bbox.y0 = (int) gc->transform.c[5];
		item->bbox.x1 = item->bbox.x0 - 1;
		item->bbox.y1 = item->bbox.y0 - 1;
	}

	nr_arena_item_request_render (item);

	return NR_ARENA_ITEM_STATE_ALL;
}

#define FBITS 12
#define XSAMPLE nr_arena_image_x_sample
#define YSAMPLE nr_arena_image_y_sample
#define b2i (image->grid2px)

static unsigned int
nr_arena_image_render (NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags)
{
	NRArenaImage *image;
	NRULong Falpha;
	unsigned char *spx, *dpx;
	int dw, dh, drs, sw, sh, srs;
	NRMatrixF d2s;

	image = NR_ARENA_IMAGE (item);

	if (!image->px) return item->state;

	Falpha = item->opacity;
	if (Falpha < 1) return item->state;

	dpx = NR_PIXBLOCK_PX (pb);
	drs = pb->rs;
	dw = pb->area.x1 - pb->area.x0;
	dh = pb->area.y1 - pb->area.y0;

	spx = image->px;
	srs = image->pxrs;
	sw = image->pxw;
	sh = image->pxh;

	d2s.c[0] = b2i.c[0];
	d2s.c[1] = b2i.c[1];
	d2s.c[2] = b2i.c[2];
	d2s.c[3] = b2i.c[3];
	d2s.c[4] = b2i.c[0] * pb->area.x0 + b2i.c[2] * pb->area.y0 + b2i.c[4];
	d2s.c[5] = b2i.c[1] * pb->area.x0 + b2i.c[3] * pb->area.y0 + b2i.c[5];

	if (pb->mode == NR_PIXBLOCK_MODE_R8G8B8) {
		/* fixme: This is not implemented yet (Lauris) */
		/* nr_R8G8B8_R8G8B8_R8G8B8A8_N_TRANSFORM (dpx, dw, dh, drs, spx, sw, sh, srs, &d2s, Falpha, XSAMPLE, YSAMPLE); */
	} else if (pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
		nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM (dpx, dw, dh, drs, spx, sw, sh, srs, &d2s, Falpha, XSAMPLE, YSAMPLE);
	} else if (pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8N) {
		nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_TRANSFORM (dpx, dw, dh, drs, spx, sw, sh, srs, &d2s, Falpha, XSAMPLE, YSAMPLE);
	}

	pb->empty = FALSE;

	return item->state;
}

static NRArenaItem *
nr_arena_image_pick (NRArenaItem *item, double x, double y, double delta, unsigned int sticky)
{
	NRArenaImage *image;
	unsigned char *p;
	int ix, iy;
	unsigned char *pixels;
	int width, height, rowstride;

	image = NR_ARENA_IMAGE (item);

	if (!image->px) return NULL;

	pixels = image->px;
	width = image->pxw;
	height = image->pxh;
	rowstride = image->pxrs;
	ix = x * image->grid2px.c[0] + y * image->grid2px.c[2] + image->grid2px.c[4];
	iy = x * image->grid2px.c[1] + y * image->grid2px.c[3] + image->grid2px.c[5];

	if ((ix < 0) || (iy < 0) || (ix >= width) || (iy >= height)) return NULL;

	p = pixels + iy * rowstride + ix * 4;

	return (p[3] > 0) ? item : NULL;
}

/* Utility */

void
nr_arena_image_set_pixels (NRArenaImage *image, const unsigned char *px, unsigned int pxw, unsigned int pxh, unsigned int pxrs)
{
	nr_return_if_fail (image != NULL);
	nr_return_if_fail (NR_IS_ARENA_IMAGE (image));

	image->px = (unsigned char *) px;
	image->pxw = pxw;
	image->pxh = pxh;
	image->pxrs = pxrs;

	nr_arena_item_request_update (NR_ARENA_ITEM (image), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_image_set_geometry (NRArenaImage *image, double x, double y, double width, double height)
{
	nr_return_if_fail (image != NULL);
	nr_return_if_fail (NR_IS_ARENA_IMAGE (image));

	image->x = x;
	image->y = y;
	image->width = width;
	image->height = height;

	nr_arena_item_request_update (NR_ARENA_ITEM (image), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

