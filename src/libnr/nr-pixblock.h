#ifndef __NR_PIXBLOCK_H__
#define __NR_PIXBLOCK_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-types.h>
#include <libnr/nr-forward.h>

enum {
	NR_PIXBLOCK_SIZE_TINY, /* Fits in (unsigned char *) */
	NR_PIXBLOCK_SIZE_4K, /* Pixelstore */
	NR_PIXBLOCK_SIZE_16K, /* Pixelstore */
	NR_PIXBLOCK_SIZE_64K, /* Pixelstore */
	NR_PIXBLOCK_SIZE_BIG, /* Normally allocated */
	NR_PIXBLOCK_SIZE_STATIC /* Externally managed */
};

enum {
	NR_PIXBLOCK_MODE_A8, /* Grayscale */
	NR_PIXBLOCK_MODE_R8G8B8, /* 8 bit RGB */
	NR_PIXBLOCK_MODE_R8G8B8A8N, /* Normal 8 bit RGBA */
	NR_PIXBLOCK_MODE_R8G8B8A8P /* Premultiplied 8 bit RGBA */
};

struct _NRPixBlock {
	unsigned int size : 3;
	unsigned int mode : 2;
	unsigned int empty : 1;
	unsigned int rs;
	NRRectS area;
	union {
		unsigned char *px;
		unsigned char p[sizeof (unsigned char *)];
	} data;
};

#define NR_PIXBLOCK_BPP(pb) (((pb)->mode == NR_PIXBLOCK_MODE_A8) ? 1 : ((pb)->mode == NR_PIXBLOCK_MODE_R8G8B8) ? 3 : 4)
#define NR_PIXBLOCK_PX(pb) (((pb)->size == NR_PIXBLOCK_SIZE_TINY) ? (pb)->data.p : (pb)->data.px)

void nr_pixblock_setup (NRPixBlock *pb, int mode, int x0, int y0, int x1, int y1, int clear);
void nr_pixblock_setup_fast (NRPixBlock *pb, int mode, int x0, int y0, int x1, int y1, int clear);
void nr_pixblock_setup_extern (NRPixBlock *pb, int mode, int x0, int y0, int x1, int y1, unsigned char *px, int rs, int empty, int clear);
void nr_pixblock_release (NRPixBlock *pb);

NRPixBlock *nr_pixblock_new (int mode, int x0, int y0, int x1, int y1, int clear);
NRPixBlock *nr_pixblock_free (NRPixBlock *pb);

unsigned char *nr_pixelstore_4K_new (int clear, unsigned char val);
void nr_pixelstore_4K_free (unsigned char *px);
unsigned char *nr_pixelstore_16K_new (int clear, unsigned char val);
void nr_pixelstore_16K_free (unsigned char *px);
unsigned char *nr_pixelstore_64K_new (int clear, unsigned char val);
void nr_pixelstore_64K_free (unsigned char *px);

#endif
