#define _NR_UTA_C_

#include <glib.h>
#include "nr-uta.h"

#define NR_UTILE_X0(u) ((u) >> 24)
#define NR_UTILE_Y0(u) (((u) >> 16) & 0xff)
#define NR_UTILE_X1(u) (((u) >> 8) & 0xff)
#define NR_UTILE_Y1(u) ((u) & 0xff)

#define NR_UTILE_VALUE(x0,y0,x1,y1) (((x0) << 24) | ((y0) << 16) | ((x1) << 8) | (y1))

#define NR_UTA_SHIFT 5
#define NR_UTA_SIZE (1 << NR_UTA_SHIFT)
#define NR_UTA_MASK (NR_UTA_SIZE - 1)

NRUTA *
nr_uta_new (gint width, gint height)
{
	NRUTA * uta;
	NRUTile * utiles;

	g_return_val_if_fail ((width > 0) && (height > 0), NULL);

	uta = g_new (NRUTA, 1);

	uta->utwidth = ((width + NR_UTA_MASK) >> NR_UTA_SHIFT);
	uta->utheight = ((height + NR_UTA_MASK) >> NR_UTA_SHIFT);

	utiles = g_new0 (NRUTile, uta->utwidth * uta->utheight);

	uta->utiles = utiles;

	return uta;
}

void
nr_uta_free (NRUTA * uta)
{
	g_return_if_fail (uta != NULL);

	g_free (uta->utiles);
	g_free (uta);
}

/* This is terribly suboptimal */

void
nr_uta_set_rect (NRUTA * uta, NRIRect * rect)
{
	NRIRect urect, clip;
	NRUTile * us, * u;
	gint x0, y0, x1, y1, x, y, xe, ye;

	g_return_if_fail (uta != NULL);
	g_return_if_fail (rect != NULL);
	g_return_if_fail (!nr_irect_is_empty (rect));

	urect.x0 = urect.y0 = 0;
	urect.x1 = uta->utwidth << NR_UTA_SHIFT;
	urect.y1 = uta->utheight << NR_UTA_SHIFT;

	nr_irect_intersection (&clip, &urect, rect);
	if (nr_irect_is_empty (&clip)) return;

	x0 = clip.x0 >> NR_UTA_SHIFT;
	y0 = clip.y0 >> NR_UTA_SHIFT;
	x1 = (clip.x1 + NR_UTA_MASK) >> NR_UTA_SHIFT;
	y1 = (clip.y1 + NR_UTA_MASK) >> NR_UTA_SHIFT;

	us = uta->utiles + y0 * uta->utwidth + x0;

	for (y = y0; y < y1; y += NR_UTA_SIZE) {
		ye = MIN (y + NR_UTA_MASK, y1);
		u = us;
		for (x = x0; x < x1; x += NR_UTA_SIZE) {
			gint ux0, uy0, ux1, uy1;
			xe = MIN (x + NR_UTA_MASK, x1);
			ux0 = MIN (NR_UTILE_X0 (*u), x);
			uy0 = MIN (NR_UTILE_Y0 (*u), y);
			ux1 = MAX (NR_UTILE_X1 (*u), xe);
			uy1 = MAX (NR_UTILE_Y1 (*u), ye);
			*u = NR_UTILE_VALUE (ux0, uy0, ux1, uy1);
			u++;
		}

		us += uta->utwidth;
	}
}

/* This is even more suboptimal */

void
nr_uta_set_uta (NRUTA * uta, NRUTA * src, gint x0, gint y0)
{
	gint x, y;

	g_return_if_fail (uta != NULL);
	g_return_if_fail (src != NULL);

	for (y = 0; y < src->utheight; y++) {
		for (x = 0; x < src->utwidth; x++) {
			NRUTile * u;
			NRIRect c;
			u = src->utiles + y * src->utwidth + x;
			c.x0 = x0 + x * NR_UTA_SIZE + NR_UTILE_X0 (*u);
			c.y0 = y0 + y * NR_UTA_SIZE + NR_UTILE_Y0 (*u);
			c.x1 = x0 + x * NR_UTA_SIZE + NR_UTILE_X1 (*u);
			c.y1 = y0 + y * NR_UTA_SIZE + NR_UTILE_Y1 (*u);
			nr_uta_set_rect (uta, &c);
		}
	}
}

