#ifndef __NR_SVP_H__
#define __NR_SVP_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

/* Sorted vector paths */

typedef struct _NRSVPSegment NRSVPSegment;
typedef struct _NRSVPFlat NRSVPFlat;
typedef struct _NRSVP NRSVP;

#include <libnr/nr-types.h>
#include <libnr/nr-path.h>

struct _NRSVPSegment {
	NRShort wind;
	NRUShort length;
	NRULong start;
	float x0, x1;
};

struct _NRSVPFlat {
	NRShort wind;
	NRUShort length;
	float y;
	float x0, x1;
};

struct _NRSVP {
	unsigned int length;
	NRPointF *points;
	NRSVPSegment segments[1];
};

#define NR_SVPSEG_LENGTH(s,i) ((s)->segments[i].length)
#define NR_SVPSEG_IS_FLAT(s,i) (!(s)->segments[i].length)

#define NR_SVPSEG_X0(s,i) ((s)->segments[i].x0)
#define NR_SVPSEG_Y0(s,i) ((s)->points[(s)->segments[i].start].y)
#define NR_SVPSEG_X1(s,i) ((s)->segments[i].x1)
#define NR_SVPSEG_Y1(s,i) ((s)->points[(s)->segments[i].start + (s)->segments[i].length - 1].y)

#define NR_SVPFLAT_X0(s,i) (((NRSVPFlat *) (s)->segments + i)->x0)
#define NR_SVPFLAT_X1(s,i) (((NRSVPFlat *) (s)->segments + i)->x1)
#define NR_SVPFLAT_Y(s,i) (((NRSVPFlat *) (s)->segments + i)->y)

void nr_svp_free (NRSVP *svp);

int nr_svp_point_wind (NRSVP *svp, float x, float y);
double nr_svp_point_distance (NRSVP *svp, float x, float y);
void nr_svp_bbox (NRSVP *svp, NRRectF *bbox, unsigned int clear);

/* Sorted vertex lists */

/* fixme: Move/remove this (Lauris) */
typedef double NRCoord;

typedef struct _NRVertex NRVertex;
typedef struct _NRSVL NRSVL;
typedef struct _NRFlat NRFlat;

struct _NRVertex {
	NRVertex *next;
	NRCoord x, y;
};

struct _NRSVL {
	NRSVL *next;
	NRVertex *vertex;
	NRRectF bbox;
	NRShort dir;
	NRShort wind;
};

struct _NRFlat {
	NRFlat *next;
	NRCoord y, x0, x1;
};

NRSVP *nr_svp_from_svl (NRSVL *svl, NRFlat *flat);

int nr_svl_point_wind (NRSVL *svl, float x, float y);

#endif
