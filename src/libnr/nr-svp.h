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

#include <libnr/nr-coord.h>
#include <libnr/nr-forward.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-path.h>

#include <glib.h>

struct _NRSVPSegment {
    gint16 wind;
    guint16 length;
    guint32 start;
    float x0, x1;
};

struct _NRSVPFlat {
    gint16 wind;
    guint16 length;
    float y;
    float x0, x1;
};

struct _NRSVP {
    unsigned int length;
    NRPoint *points;
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
NR::Coord nr_svp_point_distance (NRSVP *svp, float x, float y);
void nr_svp_bbox (NRSVP *svp, NRRect *bbox, unsigned int clear);

/* Sorted vertex lists */

typedef struct _NRVertex NRVertex;
typedef struct _NRSVL NRSVL;
typedef struct _NRFlat NRFlat;

struct _NRVertex {
    NRVertex *next;
    NR::Coord x, y;
};

struct _NRSVL {
    NRSVL *next;
    NRVertex *vertex;
    NRRect bbox;
    gint16 dir;
    gint16 wind;
};

struct _NRFlat {
    NRFlat *next;
    NR::Coord y, x0, x1;
};

NRSVP *nr_svp_from_svl (NRSVL *svl, NRFlat *flat);

int nr_svl_point_wind (NRSVL *svl, float x, float y);

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
