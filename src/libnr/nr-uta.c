#define __NR_UTA_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "nr-macros.h"

#include "nr-uta.h"

/* These take area as pixels */

void
nr_uta_mark_area (NRUTA *uta, int x0, int y0, int x1, int y1)
{
	int bx0, by0, bx1, by1;
	int bx, by;

	bx0 = x0 >> NR_UTILE_SHIFT;
	by0 = y0 >> NR_UTILE_SHIFT;
	bx1 = (x1 + NR_UTILE_MASK) >> NR_UTILE_SHIFT;
	by1 = (y1 + NR_UTILE_MASK) >> NR_UTILE_SHIFT;
	bx0 = MAX (bx0, uta->x0);
	by0 = MAX (by0, uta->y0);
	bx1 = MIN (bx1, uta->x1);
	by1 = MIN (by1, uta->y1);

	for (by = by0; by < by1; by++) {
		for (bx = bx0; bx < bx1; bx++) {
			int cx0, cx1, cy0, cy1;
			NRULong *u;
			u = uta->utiles + (by - uta->y0) * (uta->x1 - uta->x0) + (bx - uta->x0);
			cx0 = x0 - (bx << NR_UTILE_SHIFT);
			cy0 = y0 - (by << NR_UTILE_SHIFT);
			cx1 = x1 - (bx << NR_UTILE_SHIFT);
			cy1 = y1 - (by << NR_UTILE_SHIFT);
			cx0 = MAX (cx0, 0);
			cy0 = MAX (cy0, 0);
			cx1 = MIN (cx1, NR_UTILE_SIZE);
			cy1 = MIN (cy1, NR_UTILE_SIZE);
			cx0 = MIN (cx0, NR_UTILE_X0 (u));
			cy0 = MIN (cy0, NR_UTILE_Y0 (u));
			cx1 = MAX (cx1, NR_UTILE_X1 (u));
			cy1 = MAX (cy1, NR_UTILE_Y1 (u));
			NR_UTILE_SET (u, cx0, cy0, cx1, cy1);
		}
	}
}

/* This takes area as blocks */
void
nr_uta_move_resize (NRUTA *uta, int x0, int y0, int x1, int y1)
{
	int x, y;
	NRULong *nutiles;
	nutiles = nr_new (NRULong, (y1 - y0) * (x1 - x0));
	for (y = y0; y < y1; y++) {
		for (x = x0; x < x1; x++) {
			if ((x >= uta->x0) && (y >= uta->y0) && (x < uta->x1) && (y < uta->y1)) {
				nutiles[(y - y0) * (x1 - x0) + (x - x0)] = uta->utiles[(y - uta->y0) * (uta->x1 - uta->x0) + (x - uta->x0)];
			} else {
				NR_UTILE_SET (nutiles + (y - y0) * (x1 - x0) + (x - x0), NR_UTILE_SIZE, NR_UTILE_SIZE, 0, 0);
			}
		}
	}
	nr_free (uta->utiles);
	uta->utiles = nutiles;
	uta->x0 = x0;
	uta->y0 = y0;
	uta->x1 = x1;
	uta->y1 = y1;
}

void
nr_uta_clear_block (NRUTA *uta, int x, int y)
{
	if ((x >= uta->x0) && (y >= uta->y0) && (x < uta->x1) && (y < uta->y1)) {
		NRULong *u;
		u = uta->utiles + (y - uta->y0) * (uta->x1 - uta->x0) + (x - uta->x0);
		NR_UTILE_SET (u, NR_UTILE_SIZE, NR_UTILE_SIZE, 0, 0);
	}
}
