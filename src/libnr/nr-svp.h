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

#define NR_SVPSEG_IS_FLAT(s,i) (!(s)->segments[i].length)

void nr_svp_free (NRSVP *svp);

void nr_svp_bbox (NRSVP *svp, NRRect *bbox, unsigned int clear);

/* Sorted vertex lists */

typedef struct _NRVertex NRVertex;

struct _NRVertex {
    NRVertex *next;
    NR::Coord x, y;
};


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
