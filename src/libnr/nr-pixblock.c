#define __NR_PIXBLOCK_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <string.h>
#include <stdlib.h>
#include "nr-macros.h"
#include "nr-pixblock.h"

#define NR_TINY_MAX sizeof (unsigned char *)

void
nr_pixblock_setup_fast (NRPixBlock *pb, int mode, int x0, int y0, int x1, int y1, int clear)
{
	int w, h, bpp, size;

	w = x1 - x0;
	h = y1 - y0;
	bpp = (mode == NR_PIXBLOCK_MODE_A8) ? 1 : (mode == NR_PIXBLOCK_MODE_R8G8B8) ? 3 : 4;

	size = bpp * w * h;

	if (size <= NR_TINY_MAX) {
		pb->size = NR_PIXBLOCK_SIZE_TINY;
		if (clear) memset (pb->data.p, 0x0, size);
	} else if (size <= 4096) {
		pb->size = NR_PIXBLOCK_SIZE_4K;
		pb->data.px = nr_pixelstore_4K_new (clear, 0x0);
	} else if (size <= 16384) {
		pb->size = NR_PIXBLOCK_SIZE_16K;
		pb->data.px = nr_pixelstore_16K_new (clear, 0x0);
	} else if (size <= 65536) {
		pb->size = NR_PIXBLOCK_SIZE_64K;
		pb->data.px = nr_pixelstore_64K_new (clear, 0x0);
	} else {
		pb->size = NR_PIXBLOCK_SIZE_BIG;
		pb->data.px = nr_new (unsigned char, size);
		if (clear) memset (pb->data.px, 0x0, size);
	}

	pb->mode = mode;
	pb->empty = 1;
	pb->area.x0 = x0;
	pb->area.y0 = y0;
	pb->area.x1 = x1;
	pb->area.y1 = y1;
	pb->rs = bpp * w;
}

void
nr_pixblock_setup (NRPixBlock *pb, int mode, int x0, int y0, int x1, int y1, int clear)
{
	int w, h, bpp, size;

	w = x1 - x0;
	h = y1 - y0;
	bpp = (mode == NR_PIXBLOCK_MODE_A8) ? 1 : (mode == NR_PIXBLOCK_MODE_R8G8B8) ? 3 : 4;

	size = bpp * w * h;

	if (size <= NR_TINY_MAX) {
		pb->size = NR_PIXBLOCK_SIZE_TINY;
		if (clear) memset (pb->data.p, 0x0, size);
	} else {
		pb->size = NR_PIXBLOCK_SIZE_BIG;
		pb->data.px = nr_new (unsigned char, size);
		if (clear) memset (pb->data.px, 0x0, size);
	}

	pb->mode = mode;
	pb->empty = 1;
	pb->area.x0 = x0;
	pb->area.y0 = y0;
	pb->area.x1 = x1;
	pb->area.y1 = y1;
	pb->rs = bpp * w;
}

void
nr_pixblock_setup_extern (NRPixBlock *pb, int mode, int x0, int y0, int x1, int y1, unsigned char *px, int rs, int empty, int clear)
{
	int w, bpp;

	w = x1 - x0;
	bpp = (mode == NR_PIXBLOCK_MODE_A8) ? 1 : (mode == NR_PIXBLOCK_MODE_R8G8B8) ? 3 : 4;

	pb->size = NR_PIXBLOCK_SIZE_STATIC;
	pb->mode = mode;
	pb->empty = empty;
	pb->area.x0 = x0;
	pb->area.y0 = y0;
	pb->area.x1 = x1;
	pb->area.y1 = y1;
	pb->data.px = px;
	pb->rs = rs;

	if (clear) {
		if (rs == bpp * w) {
			memset (pb->data.px, 0x0, bpp * (y1 - y0) * w);
		} else {
			int y;
			for (y = y0; y < y1; y++) {
				memset (pb->data.px + (y - y0) * rs, 0x0, bpp * w);
			}
		}
	}
}

void
nr_pixblock_release (NRPixBlock *pb)
{
	switch (pb->size) {
	case NR_PIXBLOCK_SIZE_TINY:
		break;
	case NR_PIXBLOCK_SIZE_4K:
		nr_pixelstore_4K_free (pb->data.px);
		break;
	case NR_PIXBLOCK_SIZE_16K:
		nr_pixelstore_16K_free (pb->data.px);
		break;
	case NR_PIXBLOCK_SIZE_64K:
		nr_pixelstore_64K_free (pb->data.px);
		break;
	case NR_PIXBLOCK_SIZE_BIG:
		nr_free (pb->data.px);
		break;
	case NR_PIXBLOCK_SIZE_STATIC:
		break;
	default:
		break;
	}
}

NRPixBlock *
nr_pixblock_new (int mode, int x0, int y0, int x1, int y1, int clear)
{
	NRPixBlock *pb;

	pb = nr_new (NRPixBlock, 1);

	nr_pixblock_setup (pb, mode, x0, y0, x1, y1, clear);

	return pb;
}

NRPixBlock *
nr_pixblock_free (NRPixBlock *pb)
{
	nr_pixblock_release (pb);

	nr_free (pb);

	return NULL;
}

/* PixelStore operations */

#define NR_4K_BLOCK 32
static unsigned char **nr_4K_px = NULL;
static unsigned int nr_4K_len = 0;
static unsigned int nr_4K_size = 0;

unsigned char *
nr_pixelstore_4K_new (int clear, unsigned char val)
{
	unsigned char *px;

	if (nr_4K_len != 0) {
		nr_4K_len -= 1;
		px = nr_4K_px[nr_4K_len];
	} else {
		px = nr_new (unsigned char, 4096);
	}
	
	if (clear) memset (px, val, 4096);

	return px;
}

void
nr_pixelstore_4K_free (unsigned char *px)
{
	if (nr_4K_len == nr_4K_size) {
		nr_4K_size += NR_4K_BLOCK;
		nr_4K_px = nr_renew (nr_4K_px, unsigned char *, nr_4K_size);
	}

	nr_4K_px[nr_4K_len] = px;
	nr_4K_len += 1;
}

#define NR_16K_BLOCK 32
static unsigned char **nr_16K_px = NULL;
static unsigned int nr_16K_len = 0;
static unsigned int nr_16K_size = 0;

unsigned char *
nr_pixelstore_16K_new (int clear, unsigned char val)
{
	unsigned char *px;

	if (nr_16K_len != 0) {
		nr_16K_len -= 1;
		px = nr_16K_px[nr_16K_len];
	} else {
		px = nr_new (unsigned char, 16384);
	}
	
	if (clear) memset (px, val, 16384);

	return px;
}

void
nr_pixelstore_16K_free (unsigned char *px)
{
	if (nr_16K_len == nr_16K_size) {
		nr_16K_size += NR_16K_BLOCK;
		nr_16K_px = nr_renew (nr_16K_px, unsigned char *, nr_16K_size);
	}

	nr_16K_px[nr_16K_len] = px;
	nr_16K_len += 1;
}

#define NR_64K_BLOCK 32
static unsigned char **nr_64K_px = NULL;
static unsigned int nr_64K_len = 0;
static unsigned int nr_64K_size = 0;

unsigned char *
nr_pixelstore_64K_new (int clear, unsigned char val)
{
	unsigned char *px;

	if (nr_64K_len != 0) {
		nr_64K_len -= 1;
		px = nr_64K_px[nr_64K_len];
	} else {
		px = nr_new (unsigned char, 65536);
	}

	if (clear) memset (px, val, 65536);

	return px;
}

void
nr_pixelstore_64K_free (unsigned char *px)
{
	if (nr_64K_len == nr_64K_size) {
		nr_64K_size += NR_64K_BLOCK;
		nr_64K_px = nr_renew (nr_64K_px, unsigned char *, nr_64K_size);
	}

	nr_64K_px[nr_64K_len] = px;
	nr_64K_len += 1;
}

