#ifndef _NR_UTA_H_
#define _NR_UTA_H_

#include "nr-primitives.h"

typedef struct _NRUTA NRUTA;
/* fixme: We should include glib and use gint32 */
typedef int NRUTile;

struct _NRUTA {
	int utwidth, utheight;
	NRUTile * utiles;
};

/* width and height are in pixels */
NRUTA * nr_uta_new (int width, int height);

void nr_uta_free (NRUTA * uta);

void nr_uta_set_rect (NRUTA * uta, NRIRect * rect);
void nr_uta_set_uta (NRUTA * uta, NRUTA * src, int x0, int y0);

#endif
