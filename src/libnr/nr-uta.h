#ifndef __NR_UTA_H__
#define __NR_UTA_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

typedef struct _NRUTA NRUTA;

#define NR_UTILE_SIZE 32
#define NR_UTILE_MASK 0x1f
#define NR_UTILE_SHIFT 5

#define NR_UTILE_X0(u) ((*(u) >> 15) & 0x1f)
#define NR_UTILE_Y0(u) ((*(u) >> 10) & 0x1f)
#define NR_UTILE_X1(u) ((*(u) >> 5) & 0x1f)
#define NR_UTILE_Y1(u) ((*(u)) & 0x1f)

#define NR_UTILE_SET(u,x0,y0,x1,y1) (*(u)) = ((*(u) & 0xfff00000) | ((x0) << 15) | ((y0) << 10) | ((x1) << 5) | (y1))

struct _NRUTA {
	/* All distances are in blocks */
	int x0, y0, x1, y1;
	NRULong *utiles;
};

/* This takes area as pixels */
void nr_uta_mark_area (NRUTA *uta, int x0, int y0, int x1, int y1);

/* This takes area as blocks */
void nr_uta_move_resize (NRUTA *src, int x0, int y0, int x1, int y1);

void nr_uta_clear_block (NRUTA *uta, int x, int y);

#endif
