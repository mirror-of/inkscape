#define __NR_SVP_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#define noNR_VERBOSE

#define NR_SVP_LENGTH_MAX 128

#include <assert.h>
#include <stdio.h>

#include "nr-values.h"
#include "nr-macros.h"
#include "nr-rect.h"
#include "nr-matrix.h"
#include "nr-svp-uncross.h"
#include "nr-svp-private.h"

/* Sorted vector paths */

NRSVP *
nr_svp_from_svl (NRSVL *svl, NRFlat *flat)
{
	NRSVP *svp;
	NRSVL *si;
	NRFlat *fi;
	NRVertex *vi;
	unsigned int nsegs, npoints, clip;

	nsegs = 0;
	npoints = 0;
	clip = FALSE;
	for (si = svl; si; si = si->next) {
		unsigned int np;
		nsegs += 1;
		np = 0;
		for (vi = si->vertex; vi; vi = vi->next) {
			np += 1;
			if ((np == NR_SVP_LENGTH_MAX) && vi->next) {
				nsegs += 1;
				npoints += np;
				np = 1;
				clip = TRUE;
			}
		}
		npoints += np;
	}
	for (fi = flat; fi; fi = fi->next) nsegs += 1;

	if (clip) {
		NRSVL *sl, *pl, *nl;
		sl = NULL;
		pl = NULL;
		for (si = svl; si; si = si->next) {
			nl = nr_svl_new_full (si->vertex, &si->bbox, si->wind);
			if (pl) {
				pl->next = nl;
			} else {
				sl = nl;
			}
			pl = nl;
		}
		svl = sl;
	}

	svp = malloc (sizeof (NRSVP) + (nsegs - 1) * sizeof (NRSVPSegment));
	svp->length = nsegs;
	if (nsegs > 0) {
		unsigned int sidx, pidx;
		svp->points = nr_new (NRPointF, npoints);
		sidx = 0;
		pidx = 0;
		si = svl;
		fi = flat;
		while (si || fi) {
			while (fi && (!si || (fi->y < si->vertex->y))) {
				NRSVPFlat *flat;
				flat = (NRSVPFlat *) svp->segments + sidx;
				flat->wind = 0;
				flat->length = 0;
				flat->y = (float) fi->y;
				flat->x0 = (float) fi->x0;
				flat->x1 = (float) fi->x1;
				sidx += 1;
				fi = fi->next;
			}
			while (si && (!fi || (si->vertex->y <= fi->y))) {
				NRSVPSegment *seg;
				seg = svp->segments + sidx;
				seg->wind = si->wind;
				seg->length = 0;
				seg->start = pidx;
				seg->x0 = si->bbox.x0;
				seg->x1 = si->bbox.x1;
				sidx += 1;
				for (vi = si->vertex; vi; vi = vi->next) {
					svp->points[pidx].x = (float) vi->x;
					svp->points[pidx].y = (float) vi->y;
					seg->length += 1;
					pidx += 1;
					if ((seg->length == NR_SVP_LENGTH_MAX) && vi->next) {
						NRSVL *nsvl;
						/* Have to add SVL segment */
						nsvl = nr_svl_new_vertex_wind (vi, si->wind);
						si = nr_svl_insert_sorted (si, nsvl);
						break;
					}
				}
				si = si->next;
			}
		}
		assert (sidx == nsegs);
		assert (pidx == npoints);
	} else {
		svp->points = NULL;
	}

	if (clip) {
		while (svl) {
			si = svl->next;
			svl->vertex = NULL;
			nr_svl_free_one (svl);
			svl = si;
		}
	}

	return svp;
}

void
nr_svp_free (NRSVP *svp)
{
	if (svp->points) nr_free (svp->points);
	free (svp);
}

int
nr_svp_point_wind (NRSVP *svp, float x, float y)
{
	unsigned int sidx;
	int wind;

	wind = 0;
	for (sidx = 0; sidx < svp->length; sidx++) {
		NRSVPSegment *seg;
		seg = svp->segments + sidx;
		if (seg->wind && (seg->x0 < x) && (svp->points[seg->start].y <= y) && (svp->points[seg->start + seg->length - 1].y > y)) {
			if (seg->x1 <= x) {
				/* Segment entirely to the left */
				wind += seg->wind;
			} else {
				unsigned int pidx, last;
				last = seg->start + seg->length - 1;
				for (pidx = seg->start; (pidx < last) && (svp->points[pidx].y <= y); pidx++) {
					if (svp->points[pidx + 1].y > y) {
						NRPointF *pt;
						/* Segment crosses with our Y */
						pt = svp->points + pidx;
						if ((pt[0].x <= x) && (pt[1].x <= x)) {
							/* Both endpoints to the left */
							wind += seg->wind;
						} else {
							float cxy;
							/* Have to calculate X at Y */
							cxy = pt[0].x + (pt[1].x - pt[0].x) * (y - pt[0].y) / (pt[1].y - pt[0].y);
							if (cxy < x) wind += seg->wind;
						}
						break;
					}
				}
			}
		}
	}
	return wind;
}

static double
nr_line_point_distance2 (float Ax, float Ay, float Bx, float By, float Px, float Py)
{
	double Dx, Dy, s;
	double dist2;
	Dx = Bx - Ax;
	Dy = By - Ay;
	s = ((Px - Ax) * Dx + (Py - Ay) * Dy) / (Dx * Dx + Dy * Dy);
	if (s <= 0.0) {
		dist2 = (Px - Ax) * (Px - Ax) + (Py - Ay) * (Py - Ay);
	} else if (s >= 1.0) {
		dist2 = (Px - Bx) * (Px - Bx) + (Py - By) * (Py - By);
	} else {
		double Qx, Qy;
		Qx = Ax + s * Dx;
		Qy = Ay + s * Dy;
		dist2 = (Px - Qx) * (Px - Qx) + (Py - Qy) * (Py - Qy);
	}
	return dist2;
}

double
nr_svp_point_distance (NRSVP *svp, float x, float y)
{
	unsigned int sidx;
	double best, best2;

	best = NR_HUGE_F;
	best2 = best * best;
	for (sidx = 0; sidx < svp->length; sidx++) {
		NRSVPSegment *seg;
		seg = svp->segments + sidx;
		if (((seg->x0 - x) < best) &&
		    ((NR_SVPSEG_Y0 (svp, sidx) - y) < best) &&
		    ((x - seg->x1) < best) &&
		    ((y - NR_SVPSEG_Y1 (svp, sidx)) < best)) {
			if (seg->length < 2) {
				NRSVPFlat *flat;
				double dist2;
				flat = (NRSVPFlat *) seg;
				dist2 = nr_line_point_distance2 (flat->x0, flat->y, flat->x1, flat->y, x, y);
				if (dist2 < best2) {
					best2 = dist2;
					best = sqrt (best2);
				}
			} else {
				unsigned int pidx;
				for (pidx = 0; pidx < (unsigned int) seg->length - 1; pidx++) {
					NRPointF *pt;
					double dist2;
					pt = svp->points + seg->start + pidx;
					dist2 = nr_line_point_distance2 (pt[0].x, pt[0].y, pt[1].x, pt[1].y, x, y);
					if (dist2 < best2) {
						best2 = dist2;
						best = sqrt (best2);
					}
				}
			}
		}
	}
	return best;
}

void
nr_svp_bbox (NRSVP *svp, NRRectF *bbox, unsigned int clear)
{
	unsigned int sidx;
	float x0, y0, x1, y1;

	x0 = y0 = NR_HUGE_F;
	x1 = y1 = -NR_HUGE_F;

	for (sidx = 0; sidx < svp->length; sidx++) {
		NRSVPSegment *seg;
		seg = svp->segments + sidx;
		if (seg->length) {
			x0 = MIN (x0, seg->x0);
			y0 = MIN (y0, svp->points[seg->start].y);
			x1 = MAX (x1, seg->x1);
			y1 = MAX (y1, svp->points[seg->start + seg->length - 1].y);
		}
	}

	if ((x1 > x0) && (y1 > y0)) {
		if (clear || (bbox->x1 <= bbox->x0) || (bbox->y1 <= bbox->y0)) {
			bbox->x0 = x0;
			bbox->y0 = y0;
			bbox->x1 = x1;
			bbox->y1 = y1;
		} else {
			bbox->x0 = MIN (bbox->x0, x0);
			bbox->y0 = MIN (bbox->y0, y0);
			bbox->x1 = MAX (bbox->x1, x1);
			bbox->y1 = MAX (bbox->y1, y1);
		}
	}
}

#include <libart_lgpl/art_misc.h>

void
nr_svl_build_finish_segment (NRSVLBuild *svlb)
{
	if (svlb->refvx) {
		NRSVL *new;
		/* We have running segment */
		if (svlb->dir > 0) {
			/* We are upwards, prepended, so reverse */
			svlb->refvx = nr_vertex_reverse_list (svlb->refvx);
		}
		new = nr_svl_new_full (svlb->refvx, &svlb->bbox, (!svlb->reverse) ? svlb->dir : -svlb->dir);
		*svlb->svl = nr_svl_insert_sorted (*svlb->svl, new);
	}
	svlb->refvx = NULL;
}

void
nr_svl_build_moveto (NRSVLBuild *svlb, float x, float y)
{
	nr_svl_build_finish_segment (svlb);
	svlb->sx = NR_COORD_X_FROM_ART (x);
	svlb->sy = NR_COORD_Y_FROM_ART (y);
	svlb->dir = 0;
}

void
nr_svl_build_lineto (NRSVLBuild *svlb, float x, float y)
{
	x = (float) NR_COORD_X_FROM_ART (x);
	y = (float) NR_COORD_Y_FROM_ART (y);
	if (y != svlb->sy) {
		NRVertex *vertex;
		int newdir;
		/* We have valid line */
		newdir = (y > svlb->sy) ? 1 : -1;
		if (newdir != svlb->dir) {
			/* We have either start or turn */
			nr_svl_build_finish_segment (svlb);
			svlb->dir = newdir;
		}
		if (!svlb->refvx) {
			svlb->refvx = nr_vertex_new_xy (svlb->sx, svlb->sy);
			svlb->bbox.x0 = svlb->bbox.x1 = (float) svlb->sx;
			svlb->bbox.y0 = svlb->bbox.y1 = (float) svlb->sy;
		}
		/* Add vertex to list */
		vertex = nr_vertex_new_xy (x, y);
		vertex->next = svlb->refvx;
		svlb->refvx = vertex;
		/* Stretch bbox */
		nr_rect_f_union_xy (&svlb->bbox, x, y);
		svlb->sx = x;
		svlb->sy = y;
	} else if (x != svlb->sx) {
		NRFlat *flat;
		/* Horizontal line ends running segment */
		nr_svl_build_finish_segment (svlb);
		svlb->dir = 0;
		/* Add horizontal lines to flat list */
		flat = nr_flat_new_full (y, MIN (svlb->sx, x), MAX (svlb->sx, x));
		*svlb->flats = nr_flat_insert_sorted (*svlb->flats, flat);
		svlb->sx = x;
		/* sy = y ;-) */
	}
}

void
nr_svl_build_curveto (NRSVLBuild *svlb, double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3, float flatness)
{
	double dx1_0, dy1_0, dx2_0, dy2_0, dx3_0, dy3_0, dx2_3, dy2_3, d3_0_2;
	double s1_q, t1_q, s2_q, t2_q, v2_q;
	double f2, f2_q;
	double x00t, y00t, x0tt, y0tt, xttt, yttt, x1tt, y1tt, x11t, y11t;

	dx1_0 = x1 - x0;
	dy1_0 = y1 - y0;
	dx2_0 = x2 - x0;
	dy2_0 = y2 - y0;
	dx3_0 = x3 - x0;
	dy3_0 = y3 - y0;
	dx2_3 = x3 - x2;
	dy2_3 = y3 - y2;
	f2 = flatness * flatness;
	d3_0_2 = dx3_0 * dx3_0 + dy3_0 * dy3_0;
	if (d3_0_2 < f2) {
		double d1_0_2, d2_0_2;
		d1_0_2 = dx1_0 * dx1_0 + dy1_0 * dy1_0;
		d2_0_2 = dx2_0 * dx2_0 + dy2_0 * dy2_0;
		if ((d1_0_2 < f2) && (d2_0_2 < f2)) {
			goto nosubdivide;
		} else {
			goto subdivide;
		}
	}
	f2_q = flatness * flatness * d3_0_2;
	s1_q = dx1_0 * dx3_0 + dy1_0 * dy3_0;
	t1_q = dy1_0 * dx3_0 - dx1_0 * dy3_0;
	s2_q = dx2_0 * dx3_0 + dy2_0 * dy3_0;
	t2_q = dy2_0 * dx3_0 - dx2_0 * dy3_0;
	v2_q = dx2_3 * dx3_0 + dy2_3 * dy3_0;
	if ((t1_q * t1_q) > f2_q) goto subdivide;
	if ((t2_q * t2_q) > f2_q) goto subdivide;
	if ((s1_q < 0.0) && ((s1_q * s1_q) > f2_q)) goto subdivide;
	if ((v2_q < 0.0) && ((v2_q * v2_q) > f2_q)) goto subdivide;
	if (s1_q >= s2_q) goto subdivide;

 nosubdivide:
	nr_svl_build_lineto (svlb, (float) x3, (float) y3);
	return;

 subdivide:
	x00t = (x0 + x1) * 0.5;
	y00t = (y0 + y1) * 0.5;
	x0tt = (x0 + 2 * x1 + x2) * 0.25;
	y0tt = (y0 + 2 * y1 + y2) * 0.25;
	x1tt = (x1 + 2 * x2 + x3) * 0.25;
	y1tt = (y1 + 2 * y2 + y3) * 0.25;
	x11t = (x2 + x3) * 0.5;
	y11t = (y2 + y3) * 0.5;
	xttt = (x0tt + x1tt) * 0.5;
	yttt = (y0tt + y1tt) * 0.5;

	nr_svl_build_curveto (svlb, x0, y0, x00t, y00t, x0tt, y0tt, xttt, yttt, flatness);
	nr_svl_build_curveto (svlb, xttt, yttt, x1tt, y1tt, x11t, y11t, x3, y3, flatness);
}

NRSVL *
nr_svl_from_art_vpath (ArtVpath *vpath, unsigned int windrule)
{
	NRSVLBuild svlb;
	ArtVpath *s;

	/* Initialize NRSVLBuild */
	svlb.svl = NULL;
	svlb.flats = NULL;
	svlb.refvx = NULL;
	svlb.bbox.x0 = svlb.bbox.y0 = NR_HUGE_F;
	svlb.bbox.x1 = svlb.bbox.y1 = -NR_HUGE_F;
	svlb.dir = 0;
	svlb.sx = svlb.sy = 0.0;

	for (s = vpath; s->code != ART_END; s++) {
		switch (s->code) {
		case ART_MOVETO:
		case ART_MOVETO_OPEN:
			nr_svl_build_moveto (&svlb, (float) s->x, (float) s->y);
			break;
		case ART_LINETO:
			nr_svl_build_lineto (&svlb, (float) s->x, (float) s->y);
			break;
		default:
			/* fixme: free lists */
			return NULL;
			break;
		}
	}
	nr_svl_build_finish_segment (&svlb);
	if (svlb.svl) {
		/* NRSVL *s; */
		*svlb.svl = nr_svl_uncross_full (*svlb.svl, *svlb.flats, windrule);
	} else {
		nr_flat_free_list (*svlb.flats);
	}
	/* This happnes in uncross */
	/* nr_flat_free_list (flats); */

	return *svlb.svl;
}

NRSVL *
nr_svl_from_art_bpath (ArtBpath *bpath, NRMatrixF *transform, unsigned int windrule, unsigned int close, float flatness)
{
	NRSVLBuild svlb;
	ArtBpath *bp;
	double x, y, sx, sy;
	NRSVL *svl;
	NRFlat *flats;

	/* Initialize NRSVLBuild */
	svl = NULL;
	flats = NULL;
	svlb.svl = &svl;
	svlb.flats = &flats;
	svlb.refvx = NULL;
	svlb.bbox.x0 = svlb.bbox.y0 = NR_HUGE_F;
	svlb.bbox.x1 = svlb.bbox.y1 = -NR_HUGE_F;
	svlb.dir = 0;
	svlb.reverse = FALSE;
	svlb.sx = svlb.sy = 0.0;

	x = y = 0.0;
	sx = sy = 0.0;

	for (bp = bpath; bp->code != ART_END; bp++) {
		switch (bp->code) {
		case ART_MOVETO:
		case ART_MOVETO_OPEN:
			if (close && ((x != sx) || (y != sy))) {
				/* Add closepath */
				nr_svl_build_lineto (&svlb, (float) sx, (float) sy);
			}
			if (transform) {
				sx = x = NR_MATRIX_DF_TRANSFORM_X (transform, bp->x3, bp->y3);
				sy = y = NR_MATRIX_DF_TRANSFORM_Y (transform, bp->x3, bp->y3);
			} else {
				sx = x = bp->x3;
				sy = y = bp->y3;
			}
			nr_svl_build_moveto (&svlb, (float) x, (float) y);
			break;
		case ART_LINETO:
			if (transform) {
				x = NR_MATRIX_DF_TRANSFORM_X (transform, bp->x3, bp->y3);
				y = NR_MATRIX_DF_TRANSFORM_Y (transform, bp->x3, bp->y3);
			} else {
				x = bp->x3;
				y = bp->y3;
			}
			nr_svl_build_lineto (&svlb, (float) x, (float) y);
			break;
		case ART_CURVETO:
			if (transform) {
				x = NR_MATRIX_DF_TRANSFORM_X (transform, bp->x3, bp->y3);
				y = NR_MATRIX_DF_TRANSFORM_Y (transform, bp->x3, bp->y3);
				nr_svl_build_curveto (&svlb,
						      svlb.sx, svlb.sy,
						      NR_MATRIX_DF_TRANSFORM_X (transform, bp->x1, bp->y1),
						      NR_MATRIX_DF_TRANSFORM_Y (transform, bp->x1, bp->y1),
						      NR_MATRIX_DF_TRANSFORM_X (transform, bp->x2, bp->y2),
						      NR_MATRIX_DF_TRANSFORM_Y (transform, bp->x2, bp->y2),
						      x, y,
						      flatness);
			} else {
				x = bp->x3;
				y = bp->y3;
				nr_svl_build_curveto (&svlb, svlb.sx, svlb.sy, bp->x1, bp->y1, bp->x2, bp->y2, x, y, flatness);
			}
			break;
		default:
			/* fixme: free lists */
			return NULL;
			break;
		}
	}
	if (close && ((x != sx) || (y != sy))) {
		/* Add closepath */
		nr_svl_build_lineto (&svlb, (float) sx, (float) sy);
	}
	nr_svl_build_finish_segment (&svlb);
	if (svlb.svl) {
		/* NRSVL *s; */
		*svlb.svl = nr_svl_uncross_full (*svlb.svl, *svlb.flats, windrule);
	} else {
		nr_flat_free_list (*svlb.flats);
	}
	/* This happnes in uncross */
	/* nr_flat_free_list (flats); */

	return *svlb.svl;
}

NRSVL *
nr_svl_from_art_svp (ArtSVP *asvp)
{
	NRSVL *svl;
	int i, j;
	svl = NULL;
	for (i = asvp->n_segs - 1; i >= 0; i--) {
		ArtSVPSeg *seg;
		NRSVL *psvl;
		seg = &asvp->segs[i];
		psvl = nr_svl_new ();
		psvl->next = svl;
		svl = psvl;
		svl->vertex = NULL;
		for (j = seg->n_points - 1; j >= 0; j--) {
			NRVertex *vx;
			vx = nr_vertex_new_xy (seg->points[j].x, seg->points[j].y);
			vx->next = svl->vertex;
			svl->vertex = vx;
		}
		svl->dir = seg->dir ? 1 : -1;
		svl->wind = svl->dir;
		svl->bbox.x0 = (float) seg->bbox.x0;
		svl->bbox.y0 = (float) seg->bbox.y0;
		svl->bbox.x1 = (float) seg->bbox.x1;
		svl->bbox.y1 = (float) seg->bbox.y1;
	}
	return svl;
}

ArtSVP *
nr_art_svp_from_svl (NRSVL * svl)
{
	ArtSVP * asvp;
	NRSVL * s;
	int n_segs, sn;

	if (!svl) {
		asvp = art_alloc (sizeof (ArtSVP));
		asvp->n_segs = 0;
		return asvp;
	}

	n_segs = 0;
	for (s = svl; s != NULL; s = s->next) n_segs++;

	asvp = art_alloc (sizeof (ArtSVP) + (n_segs - 1) * sizeof (ArtSVPSeg));
	asvp->n_segs = n_segs;

	sn = 0;
	for (s = svl; s != NULL; s = s->next) {
		ArtSVPSeg * aseg;
		NRVertex * v;
		int n_points, pn;

		aseg = &asvp->segs[sn];

		n_points = 0;
		for (v = s->vertex; v != NULL; v = v->next) n_points++;
		aseg->n_points = n_points;

		aseg->dir = (s->wind == -1);

		aseg->points = art_new (ArtPoint, n_points);

		pn = 0;
		for (v = s->vertex; v != NULL; v = v->next) {
			aseg->points[pn].x = NR_COORD_TO_ART (v->x);
			aseg->points[pn].y = NR_COORD_TO_ART (v->y);
			pn++;
		}

		aseg->bbox.x0 = NR_COORD_TO_ART (s->bbox.x0);
		aseg->bbox.y0 = NR_COORD_TO_ART (s->bbox.y0);
		aseg->bbox.x1 = NR_COORD_TO_ART (s->bbox.x1);
		aseg->bbox.y1 = NR_COORD_TO_ART (s->bbox.y1);

		sn++;
	}

	return asvp;
}

int
nr_svl_point_wind (NRSVL *svl, float x, float y)
{
	NRSVL *s;
	int wind;

	wind = 0;
	for (s = svl; s != NULL; s = s->next) {
		if ((s->bbox.x0 < x) && (s->bbox.y0 <= y) && (s->bbox.y1 > y)) {
			if (s->bbox.x1 <= x) {
				wind += s->wind;
			} else {
				NRVertex *vx;
				for (vx = s->vertex; vx && vx->next && (vx->y <= y); vx = vx->next) {
					if (vx->next->y > y) {
						float cxy;
						cxy = (float) (vx->x + (vx->next->x - vx->x) * (y - vx->y) / (vx->next->y - vx->y));
						if (cxy < x) wind += s->wind;
						break;
					}
				}
			}
		}
	}
	return wind;
}

/* NRVertex */

#define NR_VERTEX_ALLOC_SIZE 4096
static NRVertex *ffvertex = NULL;

NRVertex *
nr_vertex_new (void)
{
	NRVertex * v;
#ifndef NR_VERTEX_ALLOC

	v = ffvertex;

	if (v == NULL) {
		int i;
		v = nr_new (NRVertex, NR_VERTEX_ALLOC_SIZE);
		for (i = 1; i < (NR_VERTEX_ALLOC_SIZE - 1); i++) v[i].next = &v[i + 1];
		v[NR_VERTEX_ALLOC_SIZE - 1].next = NULL;
		ffvertex = v + 1;
	} else {
		ffvertex = v->next;
	}
#else
	v = nr_new (NRVertex, 1);
#endif

	v->next = NULL;

	return v;
}

NRVertex *
nr_vertex_new_xy (NRCoord x, NRCoord y)
{
	NRVertex * v;

	assert (fabs (x) < 1e17);
	assert (fabs (y) < 1e17);

	v = nr_vertex_new ();

	v->x = x;
	v->y = y;

	return v;
}

void
nr_vertex_free_one (NRVertex * v)
{
#ifndef NR_VERTEX_ALLOC
	v->next = ffvertex;
	ffvertex = v;
#else
	nr_free (v);
#endif
}

void
nr_vertex_free_list (NRVertex * v)
{
#ifndef NR_VERTEX_ALLOC
	NRVertex * l;
	for (l = v; l->next != NULL; l = l->next);
	l->next = ffvertex;
	ffvertex = v;
#else
	NRVertex *l, *n;
	l = v;
	while (l) {
		n = l->next;
		nr_free (l);
		l = n;
	}
#endif
}

NRVertex *
nr_vertex_reverse_list (NRVertex * v)
{
	NRVertex * p;

	p = NULL;

	while (v) {
		NRVertex * n;
		n = v->next;
		v->next = p;
		p = v;
		v = n;
	}

	return p;
}

/* NRSVL */

#define NR_SVL_ALLOC_SIZE 256
static NRSVL *ffsvl = NULL;

NRSVL *
nr_svl_new (void)
{
	NRSVL *svl;

	svl = ffsvl;

	if (svl == NULL) {
		int i;
		svl = nr_new (NRSVL, NR_SVL_ALLOC_SIZE);
		for (i = 1; i < (NR_SVL_ALLOC_SIZE - 1); i++) svl[i].next = &svl[i + 1];
		svl[NR_SVL_ALLOC_SIZE - 1].next = NULL;
		ffsvl = svl + 1;
	} else {
		ffsvl = svl->next;
	}

	svl->next = NULL;

	return svl;
}

NRSVL *
nr_svl_new_full (NRVertex *vertex, NRRectF *bbox, int dir)
{
	NRSVL *svl;

	svl = nr_svl_new ();

	svl->vertex = vertex;
	svl->bbox = *bbox;
	svl->dir = dir;
	svl->wind = svl->dir;

	return svl;
}

NRSVL *
nr_svl_new_vertex_wind (NRVertex *vertex, int dir)
{
	NRSVL * svl;

	svl = nr_svl_new ();

	svl->vertex = vertex;
	svl->dir = dir;
	svl->wind = svl->dir;
	nr_svl_calculate_bbox (svl);

	return svl;
}

void
nr_svl_free_one (NRSVL *svl)
{
	if (svl->vertex) nr_vertex_free_list (svl->vertex);
	svl->next = ffsvl;
	ffsvl = svl;
}

void
nr_svl_free_list (NRSVL *svl)
{
	NRSVL *l;

	if (svl) {
		for (l = svl; l->next != NULL; l = l->next) {
			if (l->vertex) nr_vertex_free_list (l->vertex);
		}
		if (l->vertex) nr_vertex_free_list (l->vertex);
		l->next = ffsvl;
		ffsvl = svl;
	}
}


NRSVL *
nr_svl_remove (NRSVL *start, NRSVL *svl)
{
	NRSVL * s, * l;

	s = NULL;
	l = start;
	while (l != svl) {
		s = l;
		l = l->next;
	}

	if (s) {
		s->next = l->next;
		return start;
	}

	return svl->next;
}

NRSVL *
nr_svl_insert_sorted (NRSVL *start, NRSVL *svl)
{
	NRSVL * s, * l;

	if (!start) return svl;
	if (!svl) return start;

	if (nr_svl_compare (svl, start) <= 0) {
		svl->next = start;
		return svl;
	}

	s = start;
	for (l = start->next; l != NULL; l = l->next) {
		if (nr_svl_compare (svl, l) <= 0) {
			svl->next = l;
			s->next = svl;
			return start;
		}
		s = l;
	}

	svl->next = NULL;
	s->next = svl;

	return start;
}

NRSVL *
nr_svl_move_sorted (NRSVL *start, NRSVL *svl)
{
	NRSVL *s, *l;

	s = 0;
	l = start;
	while (l != svl) {
		s = l;
		l = l->next;
	}

	if (s) {
		s->next = nr_svl_insert_sorted (svl->next, svl);
		return start;
	}

	return nr_svl_insert_sorted (start, svl);
}

int
nr_svl_compare (NRSVL *l, NRSVL *r)
{
	float xx0, yy0, x1x0, y1y0;
	float d;

	if (l->vertex->y < r->vertex->y) return -1;
	if (l->vertex->y > r->vertex->y) return 1;
	if (l->vertex->x < r->vertex->x) return -1;
	if (l->vertex->x > r->vertex->x) return 1;

	/* Y is same, X is same, checking for line orders */

	xx0 = (float) (l->vertex->next->x - r->vertex->x);
	yy0 = (float) (l->vertex->next->y - r->vertex->y);
	x1x0 = (float) (r->vertex->next->x - r->vertex->x);
	y1y0 = (float) (r->vertex->next->y - r->vertex->y);

	d = xx0 * y1y0 - yy0 * x1x0;

	/* fixme: test almost zero cases */
	if (d < 0.0) return -1;
	if (d > 0.0) return 1;

	return 0;
}

void
nr_svl_calculate_bbox (NRSVL *svl)
{
	NRVertex * v;

	svl->bbox.x0 = svl->bbox.y0 = 1e18F;
	svl->bbox.x1 = svl->bbox.y1 = -1e18F;

	for (v = svl->vertex; v != NULL; v = v->next) {
		svl->bbox.x0 = (float) MIN (svl->bbox.x0, v->x);
		svl->bbox.y0 = (float) MIN (svl->bbox.y0, v->y);
		svl->bbox.x1 = (float) MAX (svl->bbox.x1, v->x);
		svl->bbox.y1 = (float) MAX (svl->bbox.y1, v->y);
	}
}

/* NRFlat */

#define NR_FLAT_ALLOC_SIZE 128
static NRFlat *ffflat = NULL;

NRFlat *
nr_flat_new_full (NRCoord y, NRCoord x0, NRCoord x1)
{
	NRFlat *flat;

	flat = ffflat;

	if (!flat) {
		int i;
		flat = nr_new (NRFlat, NR_FLAT_ALLOC_SIZE);
		for (i = 1; i < (NR_FLAT_ALLOC_SIZE - 1); i++) flat[i].next = &flat[i + 1];
		flat[NR_FLAT_ALLOC_SIZE - 1].next = NULL;
		ffflat = flat + 1;
	} else {
		ffflat = flat->next;
	}

	flat->next = NULL;
	flat->y = y;
	flat->x0 = x0;
	flat->x1 = x1;

	return flat;
}

void
nr_flat_free_one (NRFlat *flat)
{
	flat->next = ffflat;
	ffflat = flat;
}

void
nr_flat_free_list (NRFlat *flat)
{
	NRFlat *l;

	if (flat) {
		for (l = flat; l->next != NULL; l = l->next);
		l->next = ffflat;
		ffflat = flat;
	}
}

NRFlat *
nr_flat_insert_sorted (NRFlat *start, NRFlat *flat)
{
	NRFlat *s, *l;

	if (!start) return flat;
	if (!flat) return start;

	if (flat->y < start->y) {
		flat->next = start;
		return flat;
	}

	if ((flat->y == start->y) && (flat->x0 <= start->x0)) {
		if (flat->x1 > start->x0) {
			start->x0 = flat->x0;
			start->x1 = MAX (flat->x1, start->x1);
			nr_flat_free_one (flat);
			return start;
		}
		flat->next = start;
		return flat;
	}

	s = start;
	for (l = start->next; l != NULL; l = l->next) {
		if (flat->y < l->y) {
			flat->next = l;
			s->next = flat;
			return start;
		} else if (flat->y == l->y) {
			if ((s->y == flat->y) && (s->x1 > flat->x0)) {
				s->x1 = MAX (flat->x1, s->x1);
				nr_flat_free_one (flat);
				return start;
			} else if (flat->x0 <= l->x0) {
				if (flat->x1 > l->x0) {
					l->x0 = flat->x0;
					l->x1 = MAX (flat->x1, l->x1);
					nr_flat_free_one (flat);
					return start;
				}
				flat->next = l;
				s->next = flat;
				return start;
			}
		}
		s = l;
	}

	flat->next = NULL;
	s->next = flat;

	return start;
}

