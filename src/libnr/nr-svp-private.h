#ifndef __NR_SVP_PRIVATE_H__
#define __NR_SVP_PRIVATE_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-svp.h>

#define NR_QUANT_X 16.0F
#define NR_QUANT_Y 16.0F
#define NR_COORD_X_FROM_ART(v) (floor (NR_QUANT_X * (v) + 0.5F) / NR_QUANT_X)
#define NR_COORD_Y_FROM_ART(v) (floor (NR_QUANT_Y * (v) + 0.5F) / NR_QUANT_Y)
#define NR_COORD_TO_ART(v) (v)

typedef struct _NRSVLBuild NRSVLBuild;

struct _NRSVLBuild {
	NRSVL **svl;
	NRFlat **flats;
	NRVertex *refvx;
	int dir;
	int reverse;
	NRCoord sx, sy;
	NRRectF bbox;
};

void nr_svl_build_finish_segment (NRSVLBuild *svlb);
void nr_svl_build_moveto (NRSVLBuild *svlb, float x, float y);
void nr_svl_build_lineto (NRSVLBuild *svlb, float x, float y);
void nr_svl_build_curveto (NRSVLBuild *svlb,
			   double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3,
			   float flatness);

/* fixme: Remove these if ready (Lauris) */
#include <libart_lgpl/art_svp.h>

NRSVL *nr_svl_from_art_vpath (ArtVpath *vpath, unsigned int windrule);
NRSVL *nr_svl_from_art_bpath (ArtBpath *bpath, NRMatrixF *transform, unsigned int windrule, unsigned int close, float flatness);
NRSVL *nr_svl_from_art_svp (ArtSVP *asvp);
ArtSVP *nr_art_svp_from_svl (NRSVL *svl);

/* NRVertex */

NRVertex *nr_vertex_new (void);
NRVertex *nr_vertex_new_xy (NRCoord x, NRCoord y);
void nr_vertex_free_one (NRVertex *v);
void nr_vertex_free_list (NRVertex *v);

NRVertex *nr_vertex_reverse_list (NRVertex *v);

/* NRSVL */

NRSVL *nr_svl_new (void);
NRSVL *nr_svl_new_full (NRVertex *vertex, NRRectF *bbox, int wind);
NRSVL *nr_svl_new_vertex_wind (NRVertex *vertex, int wind);
void nr_svl_free_one (NRSVL *svl);
void nr_svl_free_list (NRSVL *svl);

NRSVL *nr_svl_remove (NRSVL *start, NRSVL *svp);
NRSVL *nr_svl_insert_sorted (NRSVL *start, NRSVL *svp);
NRSVL *nr_svl_move_sorted (NRSVL *start, NRSVL *svp);
int nr_svl_compare (NRSVL *l, NRSVL *r);

void nr_svl_calculate_bbox (NRSVL *svl);

/* NRFlat */

NRFlat *nr_flat_new_full (NRCoord y, NRCoord x0, NRCoord x1);
void nr_flat_free_one (NRFlat *flat);
void nr_flat_free_list (NRFlat *flat);

NRFlat *nr_flat_insert_sorted (NRFlat *start, NRFlat *flat);

#endif
